# Large World Coordinates (LWC) in Federation Game

How to enable and use UE5 Large World Coordinates for galaxy-scale positioning, how precision behaves at scale, and how LWC fits with our one World Partition level and floating origin.

## 1. What LWC Is and Why We Use It

In UE4, world positions used **32-bit floats**. At large distances (e.g. millions of units), precision drops: small movements or object spacing can be lost, and jitter appears. **Large World Coordinates (LWC)** in UE5 adds **64-bit double-precision** for world positions and related math, so we can place and move actors accurately across galaxy-scale distances.

Federation Game targets a full galaxy (100,000+ stars, vast distances). LWC is required so that:

- Star/system positions stay accurate when stored and rendered.
- Movement and physics don’t break down at large coordinates.
- We avoid visible jitter or “quantization” of positions at scale.

## 2. How to Enable LWC

### Project setting (logic / gameplay)

1. Open the project in Unreal Editor.
2. **Edit → Project Settings → Engine → General Settings**.
3. Enable **Enable Large Worlds**.
4. Restart the editor when prompted.

This turns on double-precision for `FVector`, `FTransform`, and related types in C++ and Blueprint.

### Renderer setting (already set in this project)

Rendering also needs to use LWC. In **Config/DefaultEngine.ini** we already have:

```ini
[/Script/Engine.RendererSettings]
; Large World Coordinates for galaxy-scale positioning
r.LargeWorldRenderPosition=True
```

So the **renderer** is configured for LWC. Ensure **Enable Large Worlds** is checked in Project Settings so logic and rendering stay in sync.

### Verifying

- After enabling and restarting, `FVector` in C++ is double-precision when LWC is on.
- In Blueprint, position/transform pins use the same underlying precision.
- **Automation test:** Run `FederationGame.Core.LargeWorldCoordinates.PreservePositionAtLargeScale` (Window → Developer Tools → Session Frontend → Automation, filter “LargeWorldCoordinates”). It spawns an actor at 100M units and asserts the position is preserved; if it fails, LWC may be off or the project may need an editor restart after enabling.

## 3. How to Use LWC in Code

- **Use `FVector` and `FTransform` as usual.** With LWC enabled, they are double-precision; no API change needed for typical placement and movement.
- **Avoid narrowing to `float` for world-space positions.** If you pass coordinates to a system that only accepts `float`, you lose precision at large distances. Prefer keeping positions in `FVector` (or the engine’s internal LWC types) until the last moment (e.g. shader or legacy API).
- **Custom data and serialization:** If you persist world positions (e.g. save games, network), use types that preserve 64-bit values (e.g. `double` or engine LWC types), not `float`.
- **Placement JSON:** Our **Place Actors From Data** flow reads `Location` as numbers; with LWC, those can be large coordinates and remain precise when converted to `FVector`.

See also: `.cursor/rules/unreal-engine.mdc` (use `FVector`, be careful converting to/from float).

## 4. Precision at Galaxy Scale

### Rough scale (for intuition)

- **32-bit float:** Meaningful precision is on the order of **~1 unit** at a distance of **~1,000,000 units** (and degrades as you go farther). So at “galaxy” distances (e.g. 10^6–10^9+ units), float world space causes visible precision loss and jitter.
- **64-bit double:** Sub-millimetre precision is still meaningful at **millions of kilometres** and beyond. So for a galaxy where we use large but finite world units (e.g. 1 unit = 1 km), double-precision keeps positions stable across the whole playable range.

### What to test in-project

- Place or spawn actors at **very large coordinates** (e.g. 1e6, 1e7, 1e8 in X/Y/Z) and move the camera or a pawn slowly; there should be **no visible jitter**.
- **AGalaxyStarField** and other procedural actors that use `FVector` for positions benefit automatically once LWC is enabled; no code change needed for basic placement, but any place that casts to `float` for world position should be reviewed.

We don’t need to “test a number” so much as: **enable LWC, use large coordinates, and confirm no jitter or snapping**. If you want a repeatable check, add a simple test level or Automation test that spawns an actor at (1e7, 0, 0), reads back its position, and asserts that the error is below a small threshold (e.g. &lt; 0.01 units).

## 5. Do We Still Need LWC? How Does It Work With World Partition and Floating Origin?

### Do we still need LWC?

**Yes.** Our design is galaxy-scale with one World Partition level for “space” and a **floating origin** so the player stays near (0,0,0) in engine space. Data and logic still use vast world coordinates (e.g. 10^6+ units between locations). LWC gives **precision** for those coordinates when we compute or render them. The floating origin then keeps the engine operating near the origin. LWC and floating origin work together: precise positions in data, stable rendering and physics in the engine. (We do not use separate streamed levels per solar system or planet.)

### How LWC works with our approach

- **World Partition:** One persistent level. The world is split into **cells**; only cells near the streaming source (e.g. player) are loaded. All cells share the **same world coordinate space**. LWC ensures that every cell’s content, no matter how far from origin, has accurate positions. So:
  - **LWC** = correct positions in one huge world.
  - **World Partition** = only load cells that are needed, in that same world.

- **Streamed levels (solar system, planet):** We stream in **separate levels** per solar system and per planet (see **Level streaming and scale architecture** in `ai-workflow-and-galaxy-scale.md`). Those levels can:
  - Use a **local origin** (e.g. 0,0,0 = star or planet centre) so that inside the level we don’t need huge coordinates; **or**
  - Be positioned in the **same LWC world** (e.g. the solar system level’s origin is placed at the system’s galaxy position). Either way, the **galaxy** level and any logic that talks about “where in the galaxy” something is should use LWC so that positions are stable when we’re at 10^6+ units.

So:

- **LWC** = use double-precision so that galaxy-scale positions and movement are correct.
- **Streaming** = load/unload regions (World Partition cells) or whole levels (solar system, planet) so we don’t load the whole galaxy at once.

They address different problems and work together: precise coordinates everywhere, and only load what’s needed.

## 6. How to Test (in this project)

1. **Enable LWC** (if not already): The worktree has `bEnableLargeWorldSupport=True` in `Config/DefaultEngine.ini` and `r.LargeWorldRenderPosition=True` in the same file. Open the project in the editor; if the project was created before LWC was added, use **Edit → Project Settings → Engine → General Settings** and ensure **Enable Large Worlds** is checked, then restart the editor.
2. **Run the automation test:** In the editor, **Window → Developer Tools → Session Frontend**. Open the **Automation** tab, filter by `LargeWorldCoordinates` or `FederationGame.Core`. Run **PreservePositionAtLargeScale**. It spawns an actor at (100M, 200M, -50M), reads the location back, and passes if the position is preserved within 0.01 units.
3. **Manual check:** Place or spawn something at a huge location (e.g. via **Place Actors From Data** with a large `Location` in JSON, or drag an actor and type 1e8 in the Details panel). Move the camera slowly; there should be no jitter. Without LWC, you’d see position snapping or wobble at that scale.

## 7. Summary

| Topic | Conclusion |
|-------|------------|
| **Enable LWC** | Project Settings → Engine → General Settings → **Enable Large Worlds**; keep **r.LargeWorldRenderPosition=True** in DefaultEngine.ini. |
| **Use** | Use `FVector`/`FTransform` as usual; avoid casting world positions to `float`. |
| **Precision** | Double-precision keeps positions stable at galaxy scale; verify by testing at large coordinates (e.g. 1e7) and checking for jitter. |
| **Still need LWC?** | Yes, for correct galaxy-scale positions. |
| **LWC + World Partition + floating origin** | LWC = precision for large coordinates; World Partition = stream cells in one world; floating origin = shift world so engine stays near origin. One level for the whole galaxy. |
| **How to test** | Run automation test `FederationGame.Core.LargeWorldCoordinates.PreservePositionAtLargeScale`; or manually place an actor at 1e8 and check for jitter. |

---

*References: Epic’s “Large World Coordinates in Unreal Engine 5” and “Large World Coordinates Rendering”; project docs `ue5-project-setup.md`, `ai-workflow-and-galaxy-scale.md`.*
