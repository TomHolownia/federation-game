# Asset Creation Workflow

**Task:** FED-026, updated by FED-047

We automate placement via FederationEditor and levels, but we need many assets for a galaxy. This doc covers: where to get them, how to keep a unified look, how to credit creators, how to create/vary assets (including with Blender + AI), and how to bulk import. All tooling stays in **C++ and Unreal Editor** -- no Python. (Unreal uses C++ for code, not C#.)

**Strategy:** Hybrid approach -- use **AI + Blender for hero/signature content**; use **free assets** for the rest. Stick to **free stuff for now** (public domain / permissive where possible). Store assets in **this repo with Git LFS**; no separate assets repo. Track binary types (e.g. `*.fbx`, `*.uasset`) with `git lfs track` so the repo stays manageable.

**Art Direction:** The Federation is a utopia -- clean, bright, well-engineered. White/silver/blue palette, sleek lines, polished surfaces, subtle glow accents. Battle wear and grit exist but only on frontline/veteran gear, not as the default. Approved reference sheets live in `Content/Concepts/`.

---

## 1. Where to Get Assets

Using multiple sources is fine. For now we stick to free sources:

- **UE Marketplace (free)** -- [unrealengine.com/marketplace/free](https://www.unrealengine.com/marketplace/en-US/free). Best starting point for an **actor with animations**: **[Animation Starter Pack](https://www.unrealengine.com/marketplace/en-US/product/animation-starter-pack)** (Epic, free). Adds the classic Mannequin plus 62 animations; add via Marketplace "Add to Project" and the content appears in your project. Standard License; add Epic to `CREDITS.md` if required.
- **Fab / Epic free content** -- [fab.com](https://fab.com), [unrealengine.com/fabfreecontent](https://unrealengine.com/en-US/fabfreecontent). Rotating and permanent free packs; check per-asset license.
- **Megascans (Quixel Bridge)** -- Free for UE; good for surfaces, nature. Built into UE.
- **Mixamo** -- Free humanoid characters and animations; download FBX and use our **Import Assets** flow (Bulk Import). Also used for **auto-rigging** custom humanoid meshes (upload mesh, get skeleton + weight painting back).
- **Meshy** -- [meshy.ai](https://www.meshy.ai). AI image-to-3D and text-to-3D generation. Produces textured meshes from concept art. Use Meshy 6 for best quality. CC BY 4.0 license on free tier; Pro tier for unlimited downloads. Credit in `CREDITS.md`.
- **Paid packs** -- Optional later (e.g. POLYGON Sci-Fi Space); use when free options don't cover a need.

---

## 2. Unified Look

Mixing sources can look inconsistent. To unify:

- **Art direction** -- Clean utopian sci-fi. White/silver/blue palette, polished surfaces, subtle glow accents. Battle wear only on veteran/frontline gear. Approved references live in `Content/Concepts/`.
- **Post-processing** -- One color grading / LUT and shared post-process settings in UE so everything is tinted and contrasted the same way.
- **Shared materials** -- Where possible, re-use a small set of master materials (e.g. metals, plastics, emissives) and instance them; swap only parameters. Run new assets through these instead of keeping every asset's original materials.
- **Lighting** -- Same lighting style and intensity range across levels so assets don't "pop" differently.
- **Curate** -- Reject or rework assets that clash; don't use everything you download.

---

## 3. Crediting Creators and Legal Safety

To avoid legal issues and respect creators:

- **Licenses** -- Before use, note each asset's license (CC0, CC BY, custom, etc.). Only use assets whose license allows game use and redistribution (if you ever ship).
- **Attribution** -- Where required (e.g. CC BY), add an entry to the existing **`CREDITS.md`** at the repo root. It already lists third-party assets with work, author, source, and license (see the Milky Way skybox example). Use the same format for each new asset or pack: name, author/source, URL, license.
- **Source tracking** -- In `CREDITS.md`, you can add a short note of which Content folder or asset name the credit refers to if helpful. Keep one section or table per source (Fab, Marketplace, Megascans, etc.) so it's easy to maintain.
- **Marketplace/Fab** -- Read the license on the store page; some require "Epic Games" or "Fab" in credits. Add those lines to `CREDITS.md`.
- **Megascans** -- Follow Quixel's attribution rules and add the required line to `CREDITS.md`.

Using the existing `CREDITS.md` for all third-party assets keeps everything in one place and keeps you legally clear.

---

## 4. Custom Asset Pipeline (AI + Blender + UE5)

### Three-tier asset strategy

| Tier | What | How | Volume |
|------|------|-----|--------|
| Hero | Player character, key NPCs, bosses, signature ships | Full pipeline (below) | ~20-50 assets |
| Kit-bashed | Generic soldiers, variants, common NPCs | Modular kit: 1 base + swappable parts (helmets, armor, attachments via sockets) | Hundreds of combos from ~30-40 pieces |
| Background | Crowd NPCs, distant characters, ambient | Marketplace packs + procedural variation (material params, decals, scale randomization) | Unlimited |

### Hero asset pipeline

```
AI concept art --> Meshy (image-to-3D) --> Blender cleanup --> Mixamo auto-rig (humanoids) --> FBX export --> UE5 import
```

1. **Concept art** -- Generate with AI image tools. Save approved sheets to `Content/Concepts/`. One clear subject per image, or a batch sheet of clearly separated props.
   - **No branding** -- Do not include real-world logos, brand names, or trademarked designs in concept art. Invented in-universe markings and insignias are fine.
   - **Lore-informed design** -- Before generating concept art, read the relevant game design docs in Confluence (races, factions, planets, items, timeline). Assets should reflect the Federation universe -- its aesthetic, technology level, and faction identity. Use the Confluence MCP to pull context.
2. **Meshy generation** -- Upload concept art to Meshy (image-to-3D). Use Meshy 6 for quality. Download as FBX (or GLB).
   - **Batch sheet technique (tested, works):** Put multiple clearly separated items in one concept image (e.g. 5 props on a white background with generous spacing). Meshy generates them all as separate meshes in a single generation. 5 items = 5x cost efficiency. Separate in Blender: Tab (Edit Mode) > P > "By Loose Parts". Each disconnected piece becomes its own object.
   - **Tips for batch sheets:** Use a clean white background. Keep items well-spaced with no overlap. One clear three-quarter view per item. Avoid text annotations (Meshy may interpret them as geometry).
   - **Single hero models:** Use one subject per image for best quality. Front view works; multi-angle concept sheets may produce duplicate figures.
3. **Blender cleanup** -- Import FBX/GLB. Decimate high-poly meshes to game-ready polycount (10K-50K faces for characters, 1K-10K for props). Fix any geometry issues. Optionally separate into modular pieces.
4. **Rigging (humanoids)** -- Upload cleaned mesh to [Mixamo](https://www.mixamo.com) for auto-rigging. Download rigged FBX with skeleton.
5. **FBX export** -- From Blender: Scale 1.0, Forward -Y, Up Z, Apply Modifiers, include Armature + Mesh if rigged.
6. **UE5 import** -- Place FBX in `Config/ImportSource/<Type>/`, run Tools > Federation > Import Assets. Or drag into Content Browser.
7. **Animation** -- Use UE5 Animation Retargeting to apply Animation Starter Pack (or any humanoid anims) to custom skeletons.

### Variation without new art

- **Material parameters** -- Master material with DirtAmount, DamageLevel, ArmorTint params. Each instance looks different.
- **Decals** -- Library of blast marks, scratches, insignias projected onto any mesh at runtime.
- **Modular attachments** -- Separate meshes attached via skeleton sockets (helmet, pouches, backpack). Swap at runtime.
- **Per-instance randomization** -- Code assigns random material params and attachments at spawn.

### Concept art

All concept art and reference images go in `Content/Concepts/`. PNG format, tracked by Git LFS. Only commit approved references.

---

## 5. Variety (So Two Placements Don't Look Identical)

- **Instanced Static Meshes (ISM)** -- Same mesh, many instances; use for repeated props. We already use this at scale (e.g. GalaxyStarField).
- **Per-instance variation** -- Vary scale, rotation, and (where the material supports it) color or a couple of parameters via per-instance data or material parameters. That gives variety without new art.
- **A few variants** -- Where it matters, use 2-3 mesh variants (e.g. "crate A", "crate B") and place them via the same placement system.

---

## 6. Bulk Import (No Python)

We don't want to manually import thousands of assets. The approach:

- **Extend FederationEditor** -- Add a **Bulk Import** flow next to "Place Actors From Data": same menu (e.g. Tools > Federation), implemented in **C++**.
- **Behaviour** -- User picks a source folder and a Content destination. Tool scans for supported types (e.g. FBX, OBJ, GLTF, PNG, TGA), imports in batches using **IAssetTools** and **UAssetImportTask** (with async where possible), keeps folder structure, and writes a small import log. Progress bar and "cancel" for large runs.
- **No Python** -- All logic in C++ (and Slate for the dialog). Unreal's native APIs are used; no Python or other scripting for this pipeline.

Concrete implementation tasks (e.g. "add FBulkImportAssetsCommand", "add progress dialog") can be created as action items on the task board when you're ready.

---

## 7. Place Actors From Data -- Making Placed Assets Visible

So that **Tools > Federation > Place Actors From Data** always shows assets (no invisible or placeholder actors), use this pattern for every new asset type:

1. **Reflection first** -- Put asset references (mesh, material, etc.) in the placement JSON under **Properties** with the same key names as the **UPROPERTY** on the actor or its components. The placement code applies these via **ApplyPropertiesFromJson** to the actor and, where relevant, to the component (e.g. SkeletalMeshComponent). That way the correct asset is assigned by path without custom code per type.
2. **Apply to components** -- If the asset lives on a **component** (e.g. SkeletalMesh on USkeletalMeshComponent), the code also runs **ApplyPropertiesFromJson** on that component so reflection can set the property. See `PlaceActorsFromDataCommand.cpp`: after applying to the actor, it applies the same MergedProps to any SkeletalMeshComponent.
3. **Defaults and fallbacks** -- After reflection, the placement code runs type-specific blocks that: fill **defaults** if a required property is still null (e.g. StarMesh/StarMaterial for GalaxyStarField), then **apply to the component** (e.g. RegenerateStars(), SetSkeletalMesh). For engine actors whose component doesn't expose the property to reflection, the code resolves the asset from the JSON path and uses **fallbacks** (e.g. known paths or Asset Registry search) so the mesh is set anyway.
4. **New asset types** -- When adding a new placeable type: (a) Prefer **UPROPERTY** on the actor for the main asset so reflection can set it from JSON. (b) In the placement command, after **ApplyPropertiesFromJson(Spawned, MergedProps)**, apply MergedProps to the relevant component if the asset is on the component. (c) Add a type-specific block that sets defaults if null and then updates the component (e.g. set mesh, call RegenerateStars). This keeps placed actors visible by default.

Config presets live in **Config/PlacementData/** (one `.json` per preset). See **Config/PlacementData/README.txt** for JSON format and how to fix asset paths.

---

## 8. References

- [IAssetTools::ImportAssetsAutomated](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Developer/AssetTools/IAssetTools/ImportAssetsAutomated)
- [UAssetImportTask](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Editor/UnrealEd/UAssetImportTask)
- [Epic Fab free content](https://unrealengine.com/en-US/fabfreecontent)
- [Blender to Unreal](https://dev.epicgames.com/community/learning/tutorials/Ed4W/unreal-engine-blender-to-unreal-essentials)
- [Meshy AI](https://www.meshy.ai) -- image-to-3D and text-to-3D
- [Mixamo](https://www.mixamo.com) -- free auto-rigging and humanoid animations