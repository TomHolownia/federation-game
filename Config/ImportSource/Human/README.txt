Import Source: Human Characters
================================

Place FBX, OBJ, or GLTF files here for import into Content/Characters/Human.

To import:
  1. Place your exported model file(s) in this directory
  2. Open the UE5 editor
  3. Go to Tools -> Federation -> Import Assets
  4. The tool will scan this folder and import all supported files

Supported formats: .fbx, .obj, .gltf

Blender FBX Export Settings (for UE5):
  - Scale: 1.0
  - Apply Unit: checked
  - Forward: -Y Forward
  - Up: Z Up
  - Apply Modifiers: checked
  - Mesh: check "Tangent Space" if available
  - If rigged: include Armature + Mesh, Deformation Bones Only

The imported assets will appear at /Game/Characters/Human/ in the Content Browser.
