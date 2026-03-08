# Modular Building Architecture

How buildings work in Federation cities: modular kit construction, World Partition streaming, and seamless enterable interiors at city scale.

---

## 1. Design goals

- Every building in a city is **enterable** -- the player can walk through the front door and explore inside.
- Cities can be **large** (hundreds of buildings) and must stream in performantly on planet surfaces.
- Buildings should be **varied** without requiring unique art for each one.
- The approach must fit the existing asset pipeline (Blender --> FBX --> UE5 bulk import) and planet surface streaming architecture (`UPlanetSurfaceStreamer`, World Partition per planet surface level).

---

## 2. Modular kit system

Buildings are **not** modelled as monolithic meshes (one mesh per room or per floor). Instead, they are assembled from a shared library of snap-together **kit pieces**.

### Why kits over monolithic models

| Approach | Problem |
|----------|---------|
| One mesh per room | Adjacent rooms share walls/floors/ceilings -- modelling them separately duplicates geometry, wastes memory, and creates seams. |
| One mesh per floor | Inflexible -- can't reuse across building types. Poor LOD/streaming granularity. Hard to make destructible. |
| Modular kit | Maximum reuse. Same wall panel builds a club, apartment, and government building. Instanced rendering draws thousands of identical pieces cheaply. |

### Kit piece categories

All pieces snap to a consistent grid. Recommended: **400 UU (4m) per module** as the base unit.

| Category | Examples | Target polycount |
|----------|----------|-----------------|
| **Structural** | Wall (solid), wall (window cutout), wall (door cutout), floor tile, ceiling tile, corner piece, T-junction | 500--2K faces |
| **Vertical** | Staircase module, elevator shaft, ramp | 1K--5K faces |
| **Exterior** | Facade panel, roof section, balcony, awning, signage mount | 500--3K faces |
| **Interior props** | Counter, shelf, table, chair, terminal, bed, medical equipment | 500--5K faces |
| **Decoration** | Trim, tech panel, vent, light fixture, holographic display frame | 200--1K faces |

### Art direction

Per the asset creation workflow: clean utopian sci-fi. White/silver/blue palette, polished surfaces, subtle glow accents. Kit pieces should be designed to this style. Battle-worn variants (scorch marks, dents) are separate pieces or material parameter variations, not the default.

### Blender pipeline

Kit pieces follow the same workflow as other assets (FED-047):

```
AI concept art (building style reference)
  --> Blender or Meshy (model individual kit pieces)
  --> process_mesh.py (clean up, decimate, validate grid alignment)
  --> FBX export (Scale 1.0, Forward -Y, Up Z, Apply Modifiers)
  --> Config/ImportSource/Buildings/
  --> Tools > Federation > Import Assets
  --> Content/Buildings/Kit/
```

### Variety without unique art

- **Material parameters** -- One master material with params (DirtAmount, TintColor, EmissiveIntensity). Each building instance gets a different Material Instance. Same geometry, different look.
- **Decals** -- Projected signage, blast marks, graffiti, faction logos. Applied per-building at placement time.
- **A few variants** -- 2--3 mesh variants per piece where it matters (e.g. "wall_plain", "wall_pipes", "wall_screen"). Randomly selected at placement.
- **Procedural prop placement** -- Interior props placed from a pool with randomised positions within a room grid. No two shops look identical.

---

## 3. Building assembly

### Blueprint building templates

A Blueprint class (e.g. `ABuildingTemplate`) defines a building as a list of floor definitions. Each floor definition specifies which kit pieces go where on the grid. The Blueprint spawns all pieces as child components or child actors.

Example structure:

```
ABuildingTemplate
  ├── ExteriorShell (merged/simplified exterior mesh for HLOD)
  ├── Floor_0 (SceneComponent group)
  │     ├── Wall_0_0 (StaticMeshComponent - wall_door)
  │     ├── Wall_0_1 (StaticMeshComponent - wall_window)
  │     ├── Floor_0_Tile (StaticMeshComponent - floor_tile)
  │     ├── Ceiling_0 (StaticMeshComponent - ceiling_tile)
  │     └── Props_0 (child actors: counter, terminal, etc.)
  ├── Floor_1
  │     ├── Stairs_0_1 (StaticMeshComponent - staircase)
  │     ├── Wall_1_0 ...
  │     └── Props_1 ...
  └── Roof
```

### Procedural city generator

For large cities, a generator actor (similar to `AGalaxyStarField`) reads city layout data and assembles buildings from templates:

- **Input:** Data defining building positions, types, and sizes (JSON, DataTable, or procedural algorithm with seed).
- **Output:** Spawned building actors on the planet surface level.
- **Variation:** The generator randomises template selection, material params, prop placement, and rotation within constraints.

### Hand-crafted hero buildings

Unique locations from the game design (Memphis club, Zaphros, libraries, government buildings, the Magical Disappearing Tourist Shop) get bespoke interior layouts. They still use kit pieces for construction but with hand-placed arrangements and unique props. These are authored in-editor as placed actors, not generated.

---

## 4. Streaming and performance

### World Partition handles everything

All building actors live in the **planet surface World Partition level** (e.g. `Content/Planets/PlanetSurface_Earth.umap`). There are no sublevels per building or per floor. WP's distance-based cell streaming loads and unloads actors automatically.

**Why not sublevel streaming for interiors?** Streaming a sublevel per floor (via `ULevelStreamingDynamic`) causes hitches on floor transitions -- running up a staircase triggers a level load mid-stride. This isn't seamless enough. WP cell streaming is continuous and handles the transitions invisibly.

### Data Layers for tiered loading distances

UE5 Data Layers (or per-actor loading distance overrides) let different parts of a building load at different distances:

| Layer | Contents | Loading distance | Rationale |
|-------|----------|-----------------|-----------|
| **City silhouette** | HLOD impostors of city blocks | Very far (km+) | Skyline visible from distance |
| **Exterior** | Building exterior shell meshes | Far (hundreds of metres) | Buildings visible as you approach |
| **Interior structure** | Interior walls, floors, ceilings, stairs | Medium (tens of metres) | Loaded before player can enter, but not visible from outside (occluded by exterior walls) |
| **Interior props** | Furniture, terminals, decorations, NPCs | Short (building proximity) | Only needed when player is at/inside the building |

The player experience is seamless: everything is pre-loaded by the time they reach the door. They never see geometry pop in because interior structure loads while it's still hidden behind exterior walls.

### Occlusion culling

Even with interiors loaded in memory, UE5 hardware occlusion culling ensures they are **not rendered** unless the camera can see them. Walking past a row of 20 buildings with loaded interiors costs almost nothing in draw calls -- the GPU skips everything behind opaque walls.

### Instanced Static Meshes

Kit pieces are ideal for instancing. Hundreds of "wall_solid" actors across a city reference the same `UStaticMesh`. The mesh data exists once in memory; each instance is just a transform. With ISM or automatic instancing, this translates to minimal draw calls.

### Memory budget

- **Structural kit pieces:** Very cheap. A wall panel at 1K faces is ~20KB. Loaded once, instanced everywhere.
- **Props:** More expensive (unique textures). Controlled by short loading distance -- only nearby buildings have props loaded.
- **NPCs:** Managed separately by the gameplay dormancy system (see below).

### Building dormancy

A lightweight trigger volume per building controls **gameplay activation**, not streaming. When the player is outside:

- Interior NPCs are dormant (no AI ticking, no animation).
- Interactive objects (terminals, doors) don't process input.
- Physics on interior props is disabled.

When the player enters the trigger volume, gameplay wakes up. This keeps CPU cost proportional to what the player is interacting with, while the geometry remains loaded and ready.

---

## 5. HLOD for city skylines

Hierarchical Level of Detail is critical for distant cities. UE5 can auto-generate HLOD:

- **Per-block HLOD:** A city block (e.g. 8--16 buildings) is merged into one simplified mesh at distance. This might be 2K--5K faces representing an entire block that would be 500K+ faces at full detail.
- **Building exterior HLOD:** Individual buildings get simplified exterior-only meshes at medium distance.
- **Nanite:** If kit pieces use Nanite-enabled meshes, UE5 handles geometric LOD automatically at the triangle level. This pairs well with HLOD for the coarsest distance tier.

---

## 6. City layout data

Cities are defined in data, not hand-placed building by building. A city definition might look like:

```json
{
  "CityName": "New London",
  "Planet": "Earth",
  "Seed": 42,
  "Blocks": [
    {
      "Type": "commercial",
      "Position": [0, 0, 0],
      "BuildingCount": 12,
      "Templates": ["shop_small", "shop_medium", "restaurant", "bank"]
    },
    {
      "Type": "residential",
      "Position": [4000, 0, 0],
      "BuildingCount": 20,
      "Templates": ["apartment_3floor", "apartment_5floor"]
    },
    {
      "Type": "government",
      "Position": [2000, 2000, 0],
      "BuildingCount": 3,
      "Templates": ["government_hero", "police_station", "library"]
    }
  ],
  "HeroBuildings": [
    {
      "Template": "club_memphis",
      "Position": [800, 400, 0],
      "Rotation": [0, 45, 0]
    }
  ]
}
```

The city generator reads this and spawns building actors. The seed ensures the same city generates identically every time. Hero buildings are placed at fixed positions; generic buildings are procedurally arranged within their block bounds.

---

## 7. Implementation phases

| Phase | Work | Depends on |
|-------|------|-----------|
| **1 -- Kit prototype** | Design 10--15 core kit pieces in Blender. Import to UE5. Assemble one test building manually in-editor. Validate grid snapping, material assignment, and occlusion. | Asset pipeline (FED-047) |
| **2 -- Building templates** | Create `ABuildingTemplate` Blueprint class. Define 3--5 building types as data (shop, apartment, club). Verify WP streaming and Data Layer loading distances on the test planet surface. | Phase 1 |
| **3 -- City generator** | Build a procedural city generator actor. Reads city layout JSON, spawns buildings from templates. Test with a 50-building city on the stress test planet. Profile with `stat unit`, `stat scenerendering`. | Phase 2 |
| **4 -- Hero interiors** | Hand-craft interiors for key locations (clubs, library, government). Add gameplay dormancy trigger volumes. | Phase 2 |
| **5 -- Scale test** | Generate a 200--500 building city. Profile memory, draw calls, streaming. Tune Data Layer distances and HLOD settings. | Phase 3 |

---

## 8. References

- [World Partition](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine)
- [Data Layers](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-data-layers-in-unreal-engine)
- [HLOD](https://dev.epicgames.com/documentation/en-us/unreal-engine/hierarchical-level-of-detail-in-unreal-engine)
- [Nanite](https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine)
- [Instanced Static Meshes](https://dev.epicgames.com/documentation/en-us/unreal-engine/instanced-static-meshes-in-unreal-engine)
- [Blender to Unreal](https://dev.epicgames.com/community/learning/tutorials/Ed4W/unreal-engine-blender-to-unreal-essentials)
- Asset creation workflow: `docs/technical/asset-creation-workflow.md`
- Planet surface streaming: `docs/technical/ai-workflow-and-galaxy-scale.md` (section 2)
- Building types and locations: `docs/game-design/old-docs/buildings.md`
