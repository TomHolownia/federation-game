"""
Federation Game - Blender Mesh Processor

Automates the Blender step of the asset pipeline:
  - Import FBX/GLB
  - Separate into individual objects (by loose parts)
  - Optionally decimate to target face count
  - Export each object as a separate FBX

Usage (command line, headless):
  blender --background --python Tools/Blender/process_mesh.py -- \
    --input "Config/ImportSource/Props/SciFiProps.fbx" \
    --output "Config/ImportSource/Props/Separated" \
    --decimate 10000

Usage (with names):
  blender --background --python Tools/Blender/process_mesh.py -- \
    --input "Config/ImportSource/Props/SciFiProps.fbx" \
    --output "Config/ImportSource/Props/Separated" \
    --names "SupplyCrate,FuelCanister,Terminal,AmmoBox,Generator"

Arguments:
  --input    Path to FBX or GLB file
  --output   Directory to export separated FBX files into (created if missing)
  --decimate Target face count per object (optional, skip if omitted)
  --names    Comma-separated names for objects, assigned by size descending
             (optional, defaults to input filename + index)
"""

import bpy
import sys
import os
import argparse


def parse_args():
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []

    parser = argparse.ArgumentParser(description="Federation Blender Mesh Processor")
    parser.add_argument("--input", required=True, help="Path to FBX or GLB file")
    parser.add_argument("--output", required=True, help="Output directory for separated FBX files")
    parser.add_argument("--decimate", type=int, default=0, help="Target face count per object (0 = skip)")
    parser.add_argument("--names", default="", help="Comma-separated names for objects (by size descending)")
    return parser.parse_args(argv)


def clear_scene():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for collection in bpy.data.collections:
        bpy.data.collections.remove(collection)


def import_mesh(filepath):
    ext = os.path.splitext(filepath)[1].lower()
    if ext == ".fbx":
        bpy.ops.import_scene.fbx(filepath=filepath)
    elif ext in (".glb", ".gltf"):
        bpy.ops.import_scene.gltf(filepath=filepath)
    elif ext == ".obj":
        bpy.ops.wm.obj_import(filepath=filepath)
    else:
        raise ValueError(f"Unsupported format: {ext}")

    print(f"[FED] Imported: {filepath}")


def get_mesh_objects():
    return [obj for obj in bpy.data.objects if obj.type == 'MESH']


def separate_by_loose_parts():
    mesh_objects = get_mesh_objects()
    if len(mesh_objects) == 0:
        print("[FED] No mesh objects found after import")
        return []

    if len(mesh_objects) == 1:
        obj = mesh_objects[0]
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.mesh.separate(type='LOOSE')
        bpy.ops.object.mode_set(mode='OBJECT')
        bpy.ops.object.select_all(action='DESELECT')
        print(f"[FED] Separated into {len(get_mesh_objects())} objects")
    else:
        print(f"[FED] Already {len(mesh_objects)} separate objects, skipping separation")

    return get_mesh_objects()


def decimate_object(obj, target_faces):
    face_count = len(obj.data.polygons)
    if face_count <= target_faces:
        print(f"[FED]   {obj.name}: {face_count} faces (already below target {target_faces})")
        return

    ratio = target_faces / face_count
    modifier = obj.modifiers.new(name="Decimate", type='DECIMATE')
    modifier.ratio = max(ratio, 0.01)

    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier="Decimate")

    new_count = len(obj.data.polygons)
    print(f"[FED]   {obj.name}: {face_count} -> {new_count} faces")


def set_origin_to_geometry(obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')


def export_object_as_fbx(obj, output_dir, name):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj

    filepath = os.path.join(output_dir, f"{name}.fbx")
    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        apply_scale_options='FBX_SCALE_ALL',
        axis_forward='-Y',
        axis_up='Z',
        apply_unit_scale=True,
        mesh_smooth_type='FACE',
        use_mesh_modifiers=True,
    )
    print(f"[FED] Exported: {filepath}")


def main():
    args = parse_args()

    input_path = os.path.abspath(args.input)
    output_dir = os.path.abspath(args.output)
    os.makedirs(output_dir, exist_ok=True)

    base_name = os.path.splitext(os.path.basename(input_path))[0]
    custom_names = [n.strip() for n in args.names.split(",") if n.strip()] if args.names else []

    print(f"[FED] === Federation Mesh Processor ===")
    print(f"[FED] Input:    {input_path}")
    print(f"[FED] Output:   {output_dir}")
    print(f"[FED] Decimate: {args.decimate if args.decimate > 0 else 'skip'}")

    clear_scene()
    import_mesh(input_path)
    objects = separate_by_loose_parts()

    if not objects:
        print("[FED] No objects to process, exiting")
        return

    objects.sort(key=lambda o: o.dimensions.x * o.dimensions.y * o.dimensions.z, reverse=True)

    for i, obj in enumerate(objects):
        if i < len(custom_names):
            name = custom_names[i]
        else:
            name = f"{base_name}_{i:03d}" if len(objects) > 1 else base_name
        obj.name = name

        set_origin_to_geometry(obj)

        if args.decimate > 0:
            decimate_object(obj, args.decimate)

        export_object_as_fbx(obj, output_dir, name)

    print(f"[FED] === Done: {len(objects)} objects exported to {output_dir} ===")


if __name__ == "__main__":
    main()
