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

## 2. How AI Can "Place" Actors

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

The project provides a **Place Actors From Data** command (Level Editor toolbar) that reads `Config/PlacementData.json` and spawns actors in the current level. This is the canonical way for an AI agent to "place" actors: edit the JSON (or generate it), then run the command.

**PlacementData.json format:**

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

**Steps for AI/agents:**

1. Add or edit entries in `Config/PlacementData.json`.
2. User (or automated run) opens the level in the editor and clicks **Place Actors From Data** on the Level Editor toolbar.
3. Actors are spawned; level is marked dirty. Save the level to persist.

To support more actor types or properties, extend the spawner in `Source/federationEditor/PlaceActorsFromDataCommand.cpp` (e.g. read optional keys and set UPROPERTYs after spawn).

### Scalable universe placement strategy

| Source of content | How to manage |
|-------------------|----------------|
| **Handcrafted** | Few key locations: place via PlacementData.json or one-off sublevels; stream in with World Partition. |
| **Random** | Use a seed in data or generator params; same data = same layout. Store seeds in JSON or DataTable. |
| **Procedural** | Generator actors (e.g. `AGalaxyStarField`) read parameters from data; PlacementData.json defines which generators exist and where, plus params (StarCount, GalaxyRadius, etc.). Extend the JSON schema and spawner to set these when spawning. |

One scalable approach: keep a single **placement data** source (e.g. PlacementData.json or a DataTable) that lists all "root" actors (galaxy generators, system markers, hand-placed heroes). Each entry can include type-specific JSON (e.g. `StarCount`, `GalaxyRadius` for a star field). The same editor command (or a variant) reads this and spawns/updates actors so the level always matches the data.

---

## Summary for AI

- **Galaxy scale:** Procedural generation + World Partition + LWC. Don't hand-place.
- **Placing actors:** Edit `Config/PlacementData.json`, then run **Place Actors From Data** (Level Editor toolbar). AI edits the JSON and spawner code; a human (or automation) runs the command. See format above.
- **Your role:** Run the editor command after data changes; placement is repeatable and AI-friendly.
