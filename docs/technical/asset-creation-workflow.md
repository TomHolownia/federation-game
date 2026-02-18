# Asset Creation Workflow

**Task:** FED-026

We automate placement via FederationEditor and levels, but we need many assets for a galaxy. This doc covers: where to get them, how to keep a unified look, how to credit creators, how to create/vary assets (including with Blender + AI), and how to bulk import. All tooling stays in **C++ and Unreal Editor**—no Python. (Unreal uses C++ for code, not C#.)

---

## 1. Where to Get Assets

Using multiple sources is fine. Common options:

- **Fab / Epic free content** – [fab.com](https://fab.com), [unrealengine.com/fabfreecontent](https://unrealengine.com/en-US/fabfreecontent). Rotating and permanent free packs; check per-asset license.
- **UE Marketplace (free)** – [unrealengine.com/marketplace/free](https://www.unrealengine.com/marketplace/en-US/free).
- **Megascans (Quixel Bridge)** – Free for UE; good for surfaces, nature. Built into UE.
- **Paid packs** – e.g. POLYGON Sci-Fi Space, Space Frontier; use when free options don’t cover a need.

---

## 2. Unified Look

Mixing sources can look inconsistent. To unify:

- **Art direction** – Pick a reference (e.g. “dirty industrial sci‑fi”) and a small palette (base colors, metal/plastic feel). Stick to it when choosing and approving assets.
- **Post-processing** – One color grading / LUT and shared post-process settings in UE so everything is tinted and contrasted the same way.
- **Shared materials** – Where possible, re-use a small set of master materials (e.g. metals, plastics, emissives) and instance them; swap only parameters. Run new assets through these instead of keeping every asset’s original materials.
- **Lighting** – Same lighting style and intensity range across levels so assets don’t “pop” differently.
- **Curate** – Reject or rework assets that clash; don’t use everything you download.

---

## 3. Crediting Creators and Legal Safety

To avoid legal issues and respect creators:

- **Licenses** – Before use, note each asset’s license (CC0, CC BY, custom, etc.). Only use assets whose license allows game use and redistribution (if you ever ship).
- **Attribution** – Where required (e.g. CC BY), add an entry to the existing **`CREDITS.md`** at the repo root. It already lists third-party assets with work, author, source, and license (see the Milky Way skybox example). Use the same format for each new asset or pack: name, author/source, URL, license.
- **Source tracking** – In `CREDITS.md`, you can add a short note of which Content folder or asset name the credit refers to if helpful. Keep one section or table per source (Fab, Marketplace, Megascans, etc.) so it’s easy to maintain.
- **Marketplace/Fab** – Read the license on the store page; some require “Epic Games” or “Fab” in credits. Add those lines to `CREDITS.md`.
- **Megascans** – Follow Quixel’s attribution rules and add the required line to `CREDITS.md`.

Using the existing `CREDITS.md` for all third-party assets keeps everything in one place and keeps you legally clear.

---

## 4. Custom Assets and Blender (Including AI)

**Blender** is a good choice: free, FBX/GLTF export to UE, and lots of tutorials.

**Using AI with Blender (without being an artist):**

- **AI-generated concepts** – Use image AI (e.g. DALL·E, Midjourney, Stable Diffusion) for concept art and reference. Model in Blender to match the concept; you don’t need to draw by hand.
- **AI-assisted modeling** – Tools and add-ons are emerging (e.g. mesh from image, procedural helpers). You can use them for blockouts or simple shapes; then clean up and export to UE.
- **Texturing** – AI can generate or upscale textures; you apply them in Blender or in UE. Keep them consistent with your unified look (same material set / color grade).
- **Learning** – Use AI (e.g. ChatGPT, Claude) to explain Blender steps, shortcuts, and “how do I make X” so you can follow along without prior art training.

Workflow: concept (AI or ref) → Blender (model, optional AI help) → export FBX/GLTF → import into UE and assign from your shared materials. Action items for “set up Blender pipeline” or “try AI add-on X” can live on the task board, not in this doc.

---

## 5. Variety (So Two Placements Don’t Look Identical)

- **Instanced Static Meshes (ISM)** – Same mesh, many instances; use for repeated props. We already use this at scale (e.g. GalaxyStarField).
- **Per-instance variation** – Vary scale, rotation, and (where the material supports it) color or a couple of parameters via per-instance data or material parameters. That gives variety without new art.
- **A few variants** – Where it matters, use 2–3 mesh variants (e.g. “crate A”, “crate B”) and place them via the same placement system. No need to document every variation technique here; task board can hold “add variation to placement” type items.

---

## 6. Bulk Import (No Python)

We don’t want to manually import thousands of assets. The approach:

- **Extend FederationEditor** – Add a **Bulk Import** flow next to “Place Actors From Data”: same menu (e.g. Tools → Federation), implemented in **C++**.
- **Behaviour** – User picks a source folder and a Content destination. Tool scans for supported types (e.g. FBX, OBJ, GLTF, PNG, TGA), imports in batches using **IAssetTools** and **UAssetImportTask** (with async where possible), keeps folder structure, and writes a small import log. Progress bar and “cancel” for large runs.
- **No Python** – All logic in C++ (and Slate for the dialog). Unreal’s native APIs are used; no Python or other scripting for this pipeline.

Concrete implementation tasks (e.g. “add FBulkImportAssetsCommand”, “add progress dialog”) can be created as action items on the task board when you’re ready.

---

## MVP: Importing one human

1. **Obtain one human character FBX** (e.g. from [Mixamo](https://mixamo.com) or Epic Marketplace free "Animation Starter Pack" / a free character pack). Place it in **`Config/ImportSource/Human/`** (rename to `Human.fbx` if you want the default placement preset to work without editing).
2. In Unreal Editor: **Tools → Federation → Import Assets**. Assets are imported to **Content/Characters/Human**.
3. Open a level, then **Tools → Federation → Place Actors From Data → Human**. One human appears in the level (at 0, 0, 100 by default).
4. If your FBX has a different name, the imported asset path will differ. Edit **`Config/PlacementData/Human.json`** and set `Properties.SkeletalMesh` to the actual asset path (e.g. `/Game/Characters/Human/YourFileName.YourFileName`). Run Place Actors From Data → Human again.

**Credits:** Add any third-party human (or other) asset to the repo's **`CREDITS.md`** (same format as the existing Milky Way skybox entry: work, author, source URL, license). Do this when you add the asset so you stay legally clear.

---

## 7. References

- [IAssetTools::ImportAssetsAutomated](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Developer/AssetTools/IAssetTools/ImportAssetsAutomated)
- [UAssetImportTask](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Editor/UnrealEd/UAssetImportTask)
- [Epic Fab free content](https://unrealengine.com/en-US/fabfreecontent)
- [Blender to Unreal](https://dev.epicgames.com/community/learning/tutorials/Ed4W/unreal-engine-blender-to-unreal-essentials)
