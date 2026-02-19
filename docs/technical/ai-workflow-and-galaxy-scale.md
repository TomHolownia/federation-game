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

We use **one World Partition level** for the whole playable galaxy. No level streaming between systems: travel between all named locations is seamless. Scope is on the order of **~36 named locations** (planets/systems on the galaxy map), which is manageable in a single world.

### Level structure

| Topic | Approach |
|-------|----------|
| **World** | **One level** with **World Partition** enabled. This is the only level for galaxy, solar systems, and planets. Keep other levels only for separate flows (e.g. main menu, cinematics) if needed. |
| **Streaming** | World Partition streams **cells** by distance from the player. Only cells near the player are loaded; the rest is unloaded. Empty space between locations has no content, so vast distances do not make the level huge in memory. |
| **Floating origin** | Use a **floating origin**: periodically shift the world so the player stays near (0,0,0) in engine space. This avoids float precision issues across vast distances. Distances in data can be millions of units; the engine sees relative positions. |
| **Travel** | **Warp / jump** (or very high speed) for crossing vast distances so the player does not spend real time flying through empty space. One coordinate space, one level — no level load when moving between locations. |

### Level design workflow (do not place at huge coordinates)

You do **not** move the editor camera millions of units and place actors there. Instead:

- **Author each location at origin (local space):** Build each planet/system as a prefab, blueprint, or block with everything around (0,0,0). You never work at galaxy-scale coordinates in the editor.
- **Data holds world positions:** A data asset or table stores each location's **world position** in the galaxy (e.g. Earth at (0,0,0), New Mars at (1e9, 2e8, 0)). Coordinates can be vast; they are just numbers in data.
- **Runtime or tool places content:** At runtime (or in a build/editor step), spawn or position each block at its world coordinates. Floating origin is applied so the engine always operates near the origin. Placement-from-data (e.g. Place Actors From Data) can write world positions from this table; the level itself is authored in local chunks.

### Planets: gravity and surface

- **Gravity:** Use **local gravity** per planet: direction = toward planet center (or the dominant body). Implement via **gravity volumes** or a **gravity manager** that applies force toward the nearest/dominant planet. World gravity can stay zero in space; gravity volumes or the manager handle surface gravity.
- **Flat vs curved:** Both are supported. **Flat world** = one global up; placement is simple (position + yaw). **Curved world** (spherical planet) = each building/actor must be oriented with the **local surface normal** (so up = outward from planet at that point). Placement data or logic must then provide position-on-sphere and rotation derived from the surface normal. Prefer flat for simplicity and scaling; use curved when a "standing on a planet" feel is required (e.g. horizon, seamless surface to orbit).
- **Cities and detail at distance:** Use **impostors**, **low-poly silhouettes**, or **HLOD** for distant cities; stream in full content when the player is within range. LOD as they get closer.

### Optional MVP path

- **Phase 1:** Small spherical planet (one mesh), local gravity (force toward center), character that walks on it. No level streaming.
- **Phase 2:** Place a few buildings on the planet with **curvature-aware** placement: position on sphere + rotation from surface normal so buildings stand straight on the ground.

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

**Presets included:** `GalaxyMapTest.json` (directional light + galaxy star fields), `MilkyWaySkybox.json` (sky sphere for space background). Add more JSON files for custom placement (e.g. specific objects, levels, or procedural setups).

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
- **Defaults** – (optional) Root-level object. `Defaults.Properties` is applied to every actor first; each actor's Properties override. If present, `Defaults.StarMesh` and `Defaults.StarMaterial` apply to all `AGalaxyStarField` entries that don't set their own. Lets one JSON define "all star fields use this mesh/material" without repeating per actor.

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
- **World Partition and scale:** One World Partition level for the whole playable galaxy (~36 named locations). Floating origin for vast distances; warp/jump for fast travel. No level streaming between systems — seamless travel. Author each location at local origin; data holds world positions; runtime or tool places content. See [World Partition and scale architecture](#2-world-partition-and-scale-architecture) above.
- **Placing actors:** Add or edit JSON in `Config/PlacementData/`, then run **Place Actors From Data** → choose the preset (Level Editor toolbar or **Tools → Federation**). AI edits the JSON files and spawner code; a human (or automation) runs the chosen preset. See format above.
- **Your role:** Run the editor command after data changes; placement is repeatable and AI-friendly.
