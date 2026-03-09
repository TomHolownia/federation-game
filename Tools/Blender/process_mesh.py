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
  --input        Path to FBX or GLB file
  --output       Directory to export separated FBX files into (created if missing)
  --names        Comma-separated names for objects, assigned by size descending
  --scale        Uniform scale factor (default 100 for meter-to-cm)
  --no-separate  Skip separation, export as one FBX (also sets character decimate target)
  --decimate N   Override auto-decimate target per object
  --no-decimate  Disable decimation entirely

Auto-decimation defaults (applied unless --no-decimate):
  Props:      5,000 faces per object
  Characters: 20,000 faces (when --no-separate is used)
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
    parser.add_argument("--decimate", type=int, default=0,
                        help="Override auto-decimate target per object (0 = use auto defaults)")
    parser.add_argument("--no-decimate", action="store_true", dest="no_decimate",
                        help="Disable auto-decimation entirely")
    parser.add_argument("--names", default="", help="Comma-separated names for objects (by size descending)")
    parser.add_argument("--no-separate", action="store_true", dest="no_separate",
                        help="Skip separation, export the whole mesh as one FBX")
    parser.add_argument("--scale", type=float, default=100.0,
                        help="Uniform scale factor (default 100 for m->cm)")
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


def fix_texture_paths(input_filepath):
    """Find textures in .fbm folder next to the input file and reconnect them."""
    input_dir = os.path.dirname(input_filepath)
    base_name = os.path.splitext(os.path.basename(input_filepath))[0]
    fbm_dir = os.path.join(input_dir, f"{base_name}.fbm")

    texture_files = {}
    search_dirs = [fbm_dir, input_dir]
    for search_dir in search_dirs:
        if not os.path.isdir(search_dir):
            continue
        for f in os.listdir(search_dir):
            fl = f.lower()
            if fl.endswith(('.png', '.jpg', '.jpeg', '.tga', '.tiff', '.exr')):
                full_path = os.path.join(search_dir, f)
                texture_files[fl] = full_path

    if not texture_files:
        print("[FED] No texture files found nearby, skipping texture fix")
        return

    print(f"[FED] Found {len(texture_files)} texture file(s)")

    fixed = 0
    for img in bpy.data.images:
        if img.filepath and not os.path.isfile(bpy.path.abspath(img.filepath)):
            img_name = os.path.basename(img.filepath).lower()
            if img_name in texture_files:
                img.filepath = texture_files[img_name]
                img.reload()
                fixed += 1
                print(f"[FED]   Relinked: {img.name} -> {texture_files[img_name]}")

    channel_map = {
        'Base Color':  ['basecolor', 'diffuse', 'albedo', 'color'],
        'Metallic':    ['metallic', 'metal'],
        'Roughness':   ['roughness', 'rough'],
        'Normal':      ['normal', 'norm', 'nrm'],
    }

    for mat in bpy.data.materials:
        if not mat.use_nodes:
            continue
        for node in mat.node_tree.nodes:
            if node.type != 'BSDF_PRINCIPLED':
                continue
            for input_name, keywords in channel_map.items():
                inp = node.inputs.get(input_name)
                if not inp or inp.is_linked:
                    continue
                for name_hint, path in texture_files.items():
                    if any(kw in name_hint for kw in keywords):
                        tex_node = mat.node_tree.nodes.new('ShaderNodeTexImage')
                        tex_node.image = bpy.data.images.load(path)
                        if input_name == 'Normal':
                            normal_map = mat.node_tree.nodes.new('ShaderNodeNormalMap')
                            mat.node_tree.links.new(tex_node.outputs['Color'], normal_map.inputs['Color'])
                            mat.node_tree.links.new(normal_map.outputs['Normal'], inp)
                            tex_node.image.colorspace_settings.name = 'Non-Color'
                        elif input_name in ('Metallic', 'Roughness'):
                            mat.node_tree.links.new(tex_node.outputs['Color'], inp)
                            tex_node.image.colorspace_settings.name = 'Non-Color'
                        else:
                            mat.node_tree.links.new(tex_node.outputs['Color'], inp)
                        fixed += 1
                        print(f"[FED]   Connected {input_name}: {path}")
                        break

    print(f"[FED] Fixed {fixed} texture reference(s)")


def apply_scale(scale_factor):
    """Scale all objects uniformly (e.g. 100 to convert meters to centimeters)."""
    if scale_factor == 1.0:
        return
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            obj.scale *= scale_factor
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bpy.ops.object.select_all(action='DESELECT')
    print(f"[FED] Applied scale factor: {scale_factor}x")


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


AUTO_DECIMATE_PROP = 5000
AUTO_DECIMATE_CHARACTER = 20000

def get_auto_decimate_target(obj, is_character):
    """Determine decimate target based on face count and asset type."""
    face_count = len(obj.data.polygons)
    target = AUTO_DECIMATE_CHARACTER if is_character else AUTO_DECIMATE_PROP
    if face_count <= target:
        return 0
    return target


def decimate_object(obj, target_faces):
    face_count = len(obj.data.polygons)
    if target_faces <= 0 or face_count <= target_faces:
        print(f"[FED]   {obj.name}: {face_count} faces (no decimation needed)")
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
        path_mode='COPY',
        embed_textures=True,
    )
    print(f"[FED] Exported (textures embedded): {filepath}")


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
    is_character = args.no_separate
    if args.no_decimate:
        decimate_mode = "disabled"
    elif args.decimate > 0:
        decimate_mode = f"override: {args.decimate}"
    else:
        decimate_mode = f"auto (character={AUTO_DECIMATE_CHARACTER}, prop={AUTO_DECIMATE_PROP})"
    print(f"[FED] Decimate: {decimate_mode}")
    print(f"[FED] Scale:    {args.scale}x")

    clear_scene()
    import_mesh(input_path)
    fix_texture_paths(input_path)
    apply_scale(args.scale)

    if args.no_separate:
        objects = get_mesh_objects()
        print(f"[FED] Skipping separation, {len(objects)} object(s)")
    else:
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

        if not args.no_decimate:
            if args.decimate > 0:
                target = args.decimate
            else:
                target = get_auto_decimate_target(obj, is_character)
            if target > 0:
                decimate_object(obj, target)

        export_object_as_fbx(obj, output_dir, name)

    print(f"[FED] === Done: {len(objects)} objects exported to {output_dir} ===")


if __name__ == "__main__":
    main()
