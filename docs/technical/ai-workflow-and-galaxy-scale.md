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

---

## Summary for AI

- **Galaxy scale:** Procedural generation + World Partition + LWC. Don't hand-place.
- **Placing actors:** Use data + "Setup Galaxy Level" (or Editor Utility / Python). AI edits data and spawner code; you run the utility.
- **Your role:** Run the editor utility; placement is repeatable and AI-friendly.
