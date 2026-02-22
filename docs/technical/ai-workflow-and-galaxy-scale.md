# AI Workflow & Galaxy-Scale Architecture

How to scale to a full galaxy without hand-placing content, and how AI can "place" actors via code and data.

## 1. How One Level Scales to a Galaxy

Don't hand-place. Use:

- **Procedural generation** – Code generates positions and properties from algorithms or data.
- **World Partition** – Only load cells near the player; the rest stays as data.
- **Large World Coordinates (LWC)** – Precision for galaxy-scale positions.
- **Data-driven content** – Tables/catalogs drive what gets spawned where.

**What you have:** `AGalaxyStarField` (one actor → thousands of stars in C++). LWC and World Partition are recommended in setup.

| Scale       | Hand-placed? | How |
|------------|---------------|-----|
| Galaxy     | No           | One or few generator actors; positions from algorithm or data. |
| Star systems | Optional   | Procedural when player approaches, or from DataTable/JSON. |
| Planets    | Rare         | Procedural or templates; hand-craft only hero locations. |
| Local space | Sometimes  | Small levels; streaming loads when needed. |

**Next steps:** Enable World Partition on the galaxy level. Keep generation in code. Use DataTables/Data Assets for system definitions; C++ or Blueprint spawns from them.

---

## 2. World Partition and scale architecture

The galaxy uses a **hybrid architecture**: one main World Partition level (`DeepSpace`) for space-scale content, and **separate streaming levels per planet surface**. Planet surfaces are streamed in on approach and unloaded on departure, keeping the main level lightweight while allowing dense per-planet content.

This replaces the earlier "one WP level for everything" approach. A single WP level cannot scale to content-rich planet surfaces because WP uses a flat XY grid that doesn't map to spheres, and planet-scale content density is too large for a single level.

### Level structure

| Topic | Approach |
|-------|----------|
| **Space level** | **`DeepSpace`** with **World Partition** enabled. Contains galaxy-scale content: star fields, skybox, planet sphere visuals, and `UPlanetSurfaceStreamer` components on each planet. |
| **Planet surfaces** | Each planet has a **separate level** (e.g. `Content/Planets/PlanetSurface_Test.umap`) with its own WP. Loaded dynamically via `ULevelStreamingDynamic` when the player approaches, unloaded when they leave. |
| **Streaming** | World Partition streams space-level cells by distance. Planet surfaces are streamed as whole sublevels (via `UPlanetSurfaceStreamer`) triggered by proximity to planet spheres. |
| **Floating origin** | Use a **floating origin**: periodically shift the world so the player stays near (0,0,0) in engine space. This avoids float precision issues across vast distances. Distances in data can be millions of units; the engine sees relative positions. |
| **Travel** | **Warp / jump** (or very high speed) for crossing vast distances so the player does not spend real time flying through empty space. One coordinate space for space; planet transitions handled by streaming. |

### Level design workflow (do not place at huge coordinates)

You do **not** move the editor camera millions of units and place actors there. Instead:

- **Author each location at origin (local space):** Build each planet/system as a prefab, blueprint, or block with everything around (0,0,0). You never work at galaxy-scale coordinates in the editor.
- **Data holds world positions:** A data asset or table stores each location's **world position** in the galaxy (e.g. Earth at (0,0,0), New Mars at (1e9, 2e8, 0)). Coordinates can be vast; they are just numbers in data.
- **Runtime or tool places content:** At runtime (or in a build/editor step), spawn or position each block at its world coordinates. Floating origin is applied so the engine always operates near the origin. Placement-from-data (e.g. Place Actors From Data) can write world positions from this table; the level itself is authored in local chunks.

### Planet surface streaming

Each planet sphere in the space level has a `UPlanetSurfaceStreamer` component (`Source/federation/Planet/PlanetSurfaceStreamer.h`). It manages the full planet approach lifecycle:

1. **Idle** — Player is in space. The streamer checks distance to the player every tick.
2. **Loading** — Player enters `StreamingRadius`. The streamer calls `ULevelStreamingDynamic::LoadLevelInstance()` with the planet's surface level path.
3. **OnSurface** — Level is loaded. The player is teleported to `SurfaceSpawnOffset`, the `UPlanetGravityComponent` is disabled (standard downward gravity on flat terrain), and the `OnSurfaceLoaded` delegate fires.
4. **Unloading** — Player moves beyond `ExitAltitude` from the surface origin. The player is teleported back to their saved space position, gravity is restored, and the surface level is unloaded.

**To add a new planet surface:**

1. Create a level in `Content/Planets/` (e.g. `PlanetSurface_Mars.umap`).
2. Enable World Partition on it; add terrain, foliage, structures.
3. In the space level, add `UPlanetSurfaceStreamer` to the planet sphere actor.
4. Set `SurfaceLevelPath` to the level's long package name (e.g. `/Game/Planets/PlanetSurface_Mars`).

### Planets: gravity and surface

- **In space:** `UPlanetGravityComponent` provides radial gravity toward the nearest planet sphere. The character walks on the sphere surface with full capsule alignment and gravity-relative camera.
- **On planet surface:** When streamed into a surface level, the gravity component is disabled and standard downward gravity applies. This avoids the radial gravity system fighting with flat terrain. On exit, radial gravity re-engages.
- **Flat vs curved:** Surface levels use flat terrain (UE5 Landscape with standard gravity). The planet sphere in space provides the "curved world" feel. Future work may support curved terrain in surface levels.
- **Cities and detail at distance:** Use **impostors**, **low-poly silhouettes**, or **HLOD** for distant cities; stream in full content when the player is within range. LOD as they get closer.

### Stress testing planet surfaces

`APlanetSurfaceStressTest` (`Source/federation/Planet/PlanetSurfaceStressTest.h`) scatters configurable numbers of instanced meshes across a radius. Place it in a surface level to benchmark WP streaming and rendering:

- **InstanceCount** — Number of mesh instances (default 10,000).
- **ScatterRadius** — Radius to scatter within (default 400,000 UU = 4 km).
- **Console command:** `Fed.StressTest.SetCount <N>` changes density at runtime.

Test procedure: start at 1K instances, increase to 10K/50K/100K, measure FPS via `stat unit` and `stat scenerendering`. Try different landscape sizes (8×8 km, 16×16 km, 32×32 km) to find WP streaming limits.

### MVP path

- **Phase 1 (done):** Small spherical planet (one mesh), local gravity (force toward center), character that walks on it. SmallPlanet placement preset.
- **Phase 2 (done):** Mannequin character with first-person view, walking and collision on the planet sphere.
- **Phase 3 (current — FED-046):** Planet surface streaming MVP. Separate surface level streamed on approach. Stress test actor for performance benchmarking. Tests the hybrid space/surface architecture.
- **Phase 4 (next):** Add Landscape terrain to the surface level, stress-test with foliage and structures. Evaluate whether origin rebasing (FED-033) is needed at surface scale.

---

## 3. How AI Can "Place" Actors

AI can't click in the editor. It can write **code and data** that spawn actors. Workflow: **code-first placement**.

**Canonical pattern: data-defined galaxy**

The whole galaxy is defined in data. An Editor Utility reads it and spawns actors. You never place by hand.

| Piece | Purpose |
|-------|--------|
| **Data** | One row per generator (e.g. Galaxy StarField): class, transform, parameters (StarCount, GalaxyRadius, …). DataTable or JSON. |
| **Editor Utility "Setup Galaxy Level"** | Runs in editor. Clears procedural actors (optional), reads data, spawns each actor and sets properties. Run via menu/button to refresh level. |
| **Spawner code** | Maps each row to actor class and UPROPERTYs. Extend when adding actor types. |

**Workflow:** Edit data → run "Setup Galaxy Level" → level matches data. AI (or you) only touch data and code.

**Other options:** Editor Utilities (C++/Blueprint) or Python editor scripts can spawn actors directly; data-driven is preferred so one source of truth drives the level.

### Implemented: Place Actors From Data

The project provides a **Place Actors From Data** command (Level Editor toolbar and **Tools → Federation** menu). It lists all `*.json` files in **Config/PlacementData/**; choose a preset (e.g. **GalaxyMapTest**, **MilkyWaySkybox**) to spawn those actors in the current level. This is the canonical way for an AI agent to "place" actors: add or edit a JSON in that directory, then run the matching preset.

**Presets included:** `GalaxyMapTest.json` (directional light + galaxy star fields), `MilkyWaySkybox.json` (sky sphere for space background), `SmallPlanet.json` (flat floor + basic lighting for a small planet playable space; see Config/PlacementData/README.txt), `PlanetSurfaceTest.json` (flat ground + stress test actor for surface streaming benchmark). Add more JSON files for custom placement (e.g. specific objects, levels, or procedural setups).

**Placement JSON format** (each file in Config/PlacementData/):

```json
{
  "Actors": [
    {
      "Class": "/Script/federation.GalaxyStarField",
      "Location": [0, 0, 0],
      "Rotation": [0, 0, 0],
      "Scale": [1, 1, 1]
    }
  ]
}
```

- **Class** – C++ class path (e.g. `/Script/ModuleName.ClassName`). Blueprint classes can use their asset path.
- **Location** – [X, Y, Z] in world units.
- **Rotation** – [Pitch, Yaw, Roll] in degrees (optional; default 0).
- **Scale** – [X, Y, Z] or [S] for uniform scale (optional; default 1).
- **Properties** – (optional) Object of property names to values, applied to the spawned actor via reflection. Works for **any** actor type. Supported: numbers, bools, strings, and asset path strings (for UObject* properties). Example: `"Properties": { "StarMesh": "/Engine/BasicShapes/Shape_Sphere.Shape_Sphere", "StarCount": 5000 }`.
- **StarMesh** – (optional) Shorthand for `AGalaxyStarField`; same as putting in Properties.
- **StarMaterial** – (optional) Shorthand for `AGalaxyStarField`; same as putting in Properties.
- **Defaults** – (optional) Root-level object. `Defaults.Properties` is applied to every actor first; each actor's Properties override. If present, `Defaults.StarMesh` and `Defaults.StarMaterial` apply to all `AGalaxyStarField` entries that don't set their own. `Defaults.FloorRadius` (number) sets the floor scale for `StaticMeshActor` entries (e.g. Small Planet preset): radius = half-extent in X/Y; scale becomes [Radius/50, Radius/50, 0.2].

**Steps for AI/agents:**

1. Add or edit a JSON file in `Config/PlacementData/` (e.g. copy `GalaxyMapTest.json` and modify, or create `MyPreset.json`).
2. User (or automated run) opens the level in the editor and runs **Place Actors From Data** → choose the preset (e.g. **GalaxyMapTest**, **MilkyWaySkybox**).
3. Actors are spawned; level is marked dirty. Save the level to persist.

**GalaxyStarField visibility:** Placed `AGalaxyStarField` actors get a default star mesh and material with **zero manual steps**. If `/Game/Federation/Materials/M_GalaxyStar` does not exist, the spawner creates and saves a simple emissive material there on first use, so the mesh always appears. For the **full galaxy look** (star color variation from temperature), use a material that reads **Per Instance Custom Data** (4 floats: R, G, B, Intensity) and save it as `Content/Federation/Materials/M_GalaxyStar`; the spawner will then use that instead. Or set `Defaults.StarMaterial` in JSON to any asset path.

The **Properties** block works generically for any actor: set any UPROPERTY(EditAnywhere) by name (numbers, bools, strings, asset paths). To support new property types (e.g. FVector from arrays), extend `ApplyPropertiesFromJson` in `Source/federationEditor/PlaceActorsFromDataCommand.cpp`.

**Skybox (Milky Way):** `ASkySphere` is a large sphere actor used as a skybox. Use **Place Actors From Data → MilkyWaySkybox** to add it (scale 5000 at origin). By default it uses a fallback material (`M_GalaxyStar` or engine default). For a Milky Way look: (1) Download a free equirectangular panorama (e.g. **NASA SVS GLIMPSE 360** – spitzer.caltech.edu/explore/glimpse360 – or **NOIRLab Milky Way Over Maunakea** – noirlab.edu, CC0); (2) Import the image as a texture in Content; (3) Create a material (Unlit, **Two Sided**), sample the texture and connect to Emissive Color; (4) In `Config/PlacementData/MilkyWaySkybox.json` add `"Properties": { "SkyMaterial": "/Game/Federation/Materials/M_YourSkybox.M_YourSkybox" }` to the SkySphere entry, or set the material on the placed actor in the editor.

### Scalable universe placement strategy

| Source of content | How to manage |
|-------------------|----------------|
| **Handcrafted** | Few key locations: place via a preset in Config/PlacementData/ or one-off sublevels; stream in with World Partition. |
| **Random** | Use a seed in data or generator params; same data = same layout. Store seeds in JSON or DataTable. |
| **Procedural** | Generator actors (e.g. `AGalaxyStarField`) read parameters from data; each placement JSON defines which generators exist and where, plus params (StarCount, GalaxyRadius, etc.). Extend the JSON schema and spawner to set these when spawning. |

One scalable approach: keep **placement presets** in Config/PlacementData/ (e.g. GalaxyMapTest.json, MilkyWaySkybox.json, or custom presets). Each JSON lists "root" actors (galaxy generators, system markers, hand-placed heroes). The editor command lists all presets; choose one to spawn that set of actors so the level matches the data.

---

## Summary for AI

- **Galaxy scale:** Procedural generation + World Partition + LWC. Don't hand-place.
- **Hybrid architecture:** Main WP level (`DeepSpace`) for space. Separate streaming levels per planet surface, loaded/unloaded via `UPlanetSurfaceStreamer`. This replaced the earlier "one WP for everything" approach.
- **Planet surfaces:** Each planet sphere has a `UPlanetSurfaceStreamer` component. Set `SurfaceLevelPath` to the surface level's package name. Gravity switches from radial (space) to standard downward (surface) on transition.
- **Asset safety rule:** Never modify `/Engine/...` assets directly. Always duplicate/create project assets under `/Game/...` (for example `Content/Federation/Materials/`) first, then version control those project assets before wiring gameplay/runtime logic to them.
- **Placing actors:** Add or edit JSON in `Config/PlacementData/`, then run **Place Actors From Data** → choose the preset (Level Editor toolbar or **Tools → Federation**). AI edits the JSON files and spawner code; a human (or automation) runs the chosen preset. See format above.
- **Stress testing:** `APlanetSurfaceStressTest` scatters instanced meshes for benchmarking. Console: `Fed.StressTest.SetCount <N>`.
- **Your role:** Run the editor command after data changes; placement is repeatable and AI-friendly.
