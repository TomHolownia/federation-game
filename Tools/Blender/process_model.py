"""
Federation Game - Blender Model Processor
==========================================
Headless Blender script to process AI-generated models (Meshy, Tripo, etc.)

Usage (command line):
    blender --background --python Tools/Blender/process_model.py -- \
        --input "path/to/model.fbx" \
        --output "path/to/output/" \
        [--decimate 20000] \
        [--no-separate]

Usage (from Blender scripting tab):
    Set INPUT_FILE and OUTPUT_DIR below, then run.

What it does:
    1. Imports FBX/GLB/OBJ
    2. Separates mesh into individual objects by loose parts
    3. Decimates each object to target face count (if over threshold)
    4. Exports each object as individual FBX
    5. Writes manifest.json describing every exported piece
"""

import bpy
import bmesh
import json
import os
import sys
import math
from pathlib import Path
from mathutils import Vector

# ---------------------------------------------------------------------------
# Config -- override via command line args or edit here for scripting tab use
# ---------------------------------------------------------------------------
INPUT_FILE = ""
OUTPUT_DIR = ""
DECIMATE_TARGET = 20000  # max faces per object; 0 = no decimation
DO_SEPARATE = True

SUPPORTED_FORMATS = {".fbx", ".glb", ".gltf", ".obj"}


def parse_args():
    """Parse args after '--' in: blender --background --python script.py -- --input ..."""
    global INPUT_FILE, OUTPUT_DIR, DECIMATE_TARGET, DO_SEPARATE

    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        return

    i = 0
    while i < len(argv):
        if argv[i] == "--input" and i + 1 < len(argv):
            INPUT_FILE = argv[i + 1]
            i += 2
        elif argv[i] == "--output" and i + 1 < len(argv):
            OUTPUT_DIR = argv[i + 1]
            i += 2
        elif argv[i] == "--decimate" and i + 1 < len(argv):
            DECIMATE_TARGET = int(argv[i + 1])
            i += 2
        elif argv[i] == "--no-separate":
            DO_SEPARATE = False
            i += 1
        else:
            i += 1


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()
    for coll in bpy.data.collections:
        bpy.data.collections.remove(coll)


def import_model(filepath):
    ext = Path(filepath).suffix.lower()
    if ext == ".fbx":
        bpy.ops.import_scene.fbx(filepath=filepath)
    elif ext in (".glb", ".gltf"):
        bpy.ops.import_scene.gltf(filepath=filepath)
    elif ext == ".obj":
        bpy.ops.wm.obj_import(filepath=filepath)
    else:
        raise ValueError(f"Unsupported format: {ext}")


def get_mesh_objects():
    return [obj for obj in bpy.data.objects if obj.type == "MESH"]


def separate_by_loose_parts(obj):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.mesh.separate(type="LOOSE")
    bpy.ops.object.mode_set(mode="OBJECT")


def get_object_info(obj):
    mesh = obj.data
    bbox = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    xs = [v.x for v in bbox]
    ys = [v.y for v in bbox]
    zs = [v.z for v in bbox]

    return {
        "name": obj.name,
        "vertices": len(mesh.vertices),
        "faces": len(mesh.polygons),
        "edges": len(mesh.edges),
        "dimensions": {
            "x": round(obj.dimensions.x, 4),
            "y": round(obj.dimensions.y, 4),
            "z": round(obj.dimensions.z, 4),
        },
        "center": {
            "x": round((min(xs) + max(xs)) / 2, 4),
            "y": round((min(ys) + max(ys)) / 2, 4),
            "z": round((min(zs) + max(zs)) / 2, 4),
        },
    }


def decimate_object(obj, target_faces):
    if len(obj.data.polygons) <= target_faces:
        return len(obj.data.polygons)

    ratio = target_faces / len(obj.data.polygons)
    mod = obj.modifiers.new(name="Decimate", type="DECIMATE")
    mod.ratio = ratio
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier=mod.name)
    return len(obj.data.polygons)


def set_origin_to_geometry(obj):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.origin_set(type="ORIGIN_GEOMETRY", center="BOUNDS")


def export_object_fbx(obj, filepath):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        apply_scale_options="FBX_SCALE_ALL",
        axis_forward="-Y",
        axis_up="Z",
        apply_unit_scale=True,
        mesh_smooth_type="FACE",
        use_mesh_modifiers=True,
    )


def process():
    parse_args()

    if not INPUT_FILE:
        print("ERROR: No input file specified. Use --input <path>")
        return

    if not OUTPUT_DIR:
        print("ERROR: No output directory specified. Use --output <path>")
        return

    input_path = Path(INPUT_FILE).resolve()
    output_path = Path(OUTPUT_DIR).resolve()
    output_path.mkdir(parents=True, exist_ok=True)

    if not input_path.exists():
        print(f"ERROR: Input file not found: {input_path}")
        return

    if input_path.suffix.lower() not in SUPPORTED_FORMATS:
        print(f"ERROR: Unsupported format: {input_path.suffix}")
        return

    print(f"Processing: {input_path}")
    print(f"Output to:  {output_path}")

    clear_scene()
    import_model(str(input_path))

    mesh_objects = get_mesh_objects()
    if not mesh_objects:
        print("ERROR: No mesh objects found after import")
        return

    print(f"Found {len(mesh_objects)} mesh object(s) after import")

    if DO_SEPARATE and len(mesh_objects) == 1:
        print("Separating by loose parts...")
        separate_by_loose_parts(mesh_objects[0])
        mesh_objects = get_mesh_objects()
        print(f"  -> {len(mesh_objects)} objects after separation")

    manifest = {
        "source": str(input_path),
        "objects": [],
    }

    for i, obj in enumerate(mesh_objects):
        info_before = get_object_info(obj)
        original_faces = info_before["faces"]

        if DECIMATE_TARGET > 0:
            final_faces = decimate_object(obj, DECIMATE_TARGET)
        else:
            final_faces = original_faces

        set_origin_to_geometry(obj)

        safe_name = f"object_{i:03d}"
        export_file = output_path / f"{safe_name}.fbx"
        export_object_fbx(obj, str(export_file))

        info_after = get_object_info(obj)
        info_after["export_file"] = str(export_file)
        info_after["original_faces"] = original_faces
        info_after["decimated"] = final_faces < original_faces
        manifest["objects"].append(info_after)

        print(f"  Exported: {safe_name}.fbx ({original_faces} -> {final_faces} faces, "
              f"dims: {info_after['dimensions']['x']:.2f} x {info_after['dimensions']['y']:.2f} x {info_after['dimensions']['z']:.2f})")

    manifest_path = output_path / "manifest.json"
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=2)

    print(f"\nDone! {len(manifest['objects'])} objects exported to {output_path}")
    print(f"Manifest: {manifest_path}")


if __name__ == "__main__":
    process()
