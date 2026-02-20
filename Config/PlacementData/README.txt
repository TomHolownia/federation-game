Placement presets for Tools -> Federation -> Place Actors From Data.

Each .json file lists an "Actors" array. Open a level, then pick a preset to spawn
those actors in the current level. One .json = one submenu entry.

Pattern so placed assets are always visible (see docs/technical/asset-creation-workflow.md §7):
  - Put asset paths in "Properties" with the same key names as the actor/component UPROPERTY
    (e.g. SkeletalMesh, StarMesh, StarMaterial). The placement code applies them via reflection
    to the actor and to the component (e.g. SkeletalMeshComponent), then fills defaults/fallbacks
    so the asset is always set. When adding new asset types, follow that pattern.

SkeletalMeshActor (Human.json, Mannequin.json):
  Set "Properties.SkeletalMesh" to the asset path. If the path is wrong (e.g. pack installed
  elsewhere), the code tries fallback paths and an Asset Registry search for "Mannequin"/"Manny".
  To fix manually: Content Browser -> right-click the mesh -> Copy Reference, paste into the JSON.

SmallPlanet.json (FED-029):
  Planet sphere only (no lights). Set "Defaults.PlanetRadius" (world units; default 5000).
  The actor is labeled "Planet" and centered at (0,0,0). Place Player Start at (0, 0, PlanetRadius + 50) to land on the "north pole."
  For a flat floor instead, use an actor with "FloorRadius" (Cube mesh, scale XY = R/50, Z = 0.2).
