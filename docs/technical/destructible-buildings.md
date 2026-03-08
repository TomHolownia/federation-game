# Destructible Buildings

How building destruction works in Federation: Chaos Destruction on modular kit pieces, CPU budgeting, and phased implementation.

---

## 1. Goal

Buildings assembled from modular kit pieces (see `docs/technical/modular-building-architecture.md`) should be destructible during combat. Weapons fire, explosions, and vehicle impacts can damage and destroy individual building pieces, and sufficient structural damage can cause floor collapse.

**CPU constraint:** Real-time physics destruction is expensive. The system must enforce strict CPU limits so that destruction enhances gameplay without tanking frame rate, even during large battles in dense cities.

---

## 2. Approach: Chaos Destruction on modular pieces

UE5's **Chaos Destruction** system (Geometry Collections) is the foundation. Each modular kit piece can be fractured into debris chunks that simulate physics when destroyed.

### Why modular destruction works well

The modular kit approach (wall panels, floor tiles, ceiling tiles as separate actors) maps naturally to destruction:

- Each piece is an **independent destructible unit**. Destroying a wall panel doesn't require recalculating the entire building -- just that one piece shatters.
- Kit pieces are **small and simple** (500--2K faces), so their Geometry Collections are lightweight.
- The same fractured Geometry Collection is **reused across every instance** of that kit piece. Fracture a wall panel once; every wall panel in the city shares the same fracture pattern data.
- Destruction is **localised**. A rocket hits one wall panel, that panel shatters, and the player sees through the hole into the room. Adjacent panels are unaffected unless they take damage too.

### Pre-fractured Geometry Collections

Each kit piece gets a corresponding **Geometry Collection** asset created in the UE5 editor:

1. Import the kit piece Static Mesh (e.g. `SM_Wall_Solid`).
2. Create a Geometry Collection from it using UE5's Fracture tools.
3. Configure fracture: Voronoi or radial, 8--20 chunks per piece (enough to look good, few enough to be cheap).
4. Set damage thresholds: how much damage before the piece breaks.
5. Save as `GC_Wall_Solid` alongside the original mesh.

At runtime, when a kit piece takes enough damage, the Static Mesh actor is replaced with its Geometry Collection. The chunks simulate physics, scatter, and settle.

---

## 3. CPU budget and limits

### The problem

Chaos destruction simulation cost scales with the number of **active rigid bodies**. A single wall panel shattering into 15 chunks is fine. Twenty wall panels shattering simultaneously (300 bodies) in a firefight is a problem.

### Destruction budget system

A global **destruction budget manager** enforces limits:

| Parameter | Recommended default | Purpose |
|-----------|-------------------|---------|
| **MaxActiveDestructions** | 3--5 | Maximum number of pieces actively simulating destruction at once. Additional destruction events queue until a slot opens. |
| **MaxActiveDebrisCount** | 200 | Total rigid body count across all active destructions. Once reached, oldest/furthest debris is force-settled or removed. |
| **DestructionRadius** | 100m (10,000 UU) | Only pieces within this radius of the player can trigger full Chaos destruction. Beyond this radius, destruction uses the cheap damage-state swap (see below). |
| **DebrisLifetime** | 5--10 seconds | After settling (sleeping), debris fades out and is removed. Keeps the scene clean. |
| **DebrisSleepThreshold** | 0.5 seconds of near-zero velocity | How quickly debris goes to sleep after settling, stopping physics simulation. |

### Tiered destruction by distance

| Distance from player | Destruction method | CPU cost |
|---------------------|-------------------|----------|
| **Near** (< DestructionRadius) | Full Chaos Destruction: Geometry Collection, physics simulation, debris scattering | High (budgeted) |
| **Medium** (DestructionRadius to 2x) | Damage-state mesh swap: intact mesh replaced with a pre-made "damaged" variant (holes, scorch marks). No physics. Optional particle effect. | Very low |
| **Far** (> 2x DestructionRadius) | HLOD damage: the HLOD mesh for the block swaps to a "damaged block" variant, or is simply removed if the building is fully destroyed | Negligible |

This means a battle in one part of a city runs full destruction locally, while distant buildings show damage results without simulating physics.

### Falling back under pressure

If the frame budget is tight (e.g. large battle with many combatants), the budget manager can:

- Reduce `MaxActiveDestructions` temporarily.
- Force all pending destructions to use damage-state swaps instead of Chaos.
- Increase `DebrisSleepThreshold` so debris settles faster.
- Reduce `DebrisLifetime` so old debris is cleaned up sooner.

The player sees buildings taking damage either way; the fidelity of the destruction effect scales with available CPU.

---

## 4. Damage-state system (cheap alternative)

Every kit piece has up to three mesh variants:

| State | Mesh | When |
|-------|------|------|
| **Intact** | `SM_Wall_Solid` | Default, no damage |
| **Damaged** | `SM_Wall_Solid_Damaged` | Moderate damage received but not destroyed. Mesh has cracks, holes, scorch marks baked in. |
| **Destroyed** | (removed or rubble mesh) | Piece is gone. Optionally replaced with a small rubble static mesh on the ground. |

State transitions are instant mesh swaps -- no physics cost. This is used:

- As the **only** destruction method beyond `DestructionRadius`.
- As a **fallback** when the Chaos destruction budget is full.
- For **background destruction** (e.g. an artillery bombardment hitting a distant city block).

### Material-driven damage

For lighter damage that doesn't warrant a mesh swap, the kit piece's material can show damage via parameters:

- **DamageLevel** (0--1): drives crack overlay, colour darkening, emissive flickering.
- **ScorchMask**: a runtime-projected decal showing blast patterns.

This gives a spectrum: pristine --> scuffed (material) --> cracked (damaged mesh) --> shattered (Chaos) --> rubble/gone.

---

## 5. Structural integrity and floor collapse

### The concept

If enough structural pieces on a floor are destroyed, the floor above should collapse. This creates dramatic destruction moments during large battles.

### How it works

1. **Structural pieces are tagged.** Kit pieces that are load-bearing (walls, columns, floor tiles) have a `bIsStructural` flag. Decorative pieces (trim, vents, signs) do not.
2. **Per-floor integrity tracking.** Each floor in a building tracks how many of its structural pieces are intact. This is a simple counter, not a physics simulation.
3. **Collapse threshold.** When structural integrity drops below a threshold (e.g. 30% of structural pieces remaining), the floor above is triggered to collapse.
4. **Collapse sequence:**
   - All pieces on the collapsing floor convert to their Geometry Collections simultaneously (budgeted -- may be staggered over a few frames).
   - A downward impulse is applied to simulate gravity collapse.
   - The floor below receives damage from the falling debris.
   - **Cascading collapse** is possible: if the debris destroys enough structural pieces on the floor below, that floor collapses too.
5. **Budget interaction.** Floor collapse is a big event. It temporarily raises `MaxActiveDestructions` for a short burst (0.5--1 second), then aggressively cleans up debris. Alternatively, the collapse can be partially faked: a pre-made "collapsing floor" animation plays while Chaos handles only the pieces nearest the player.

### Simplification for performance

Full structural simulation (finite element analysis, beam loading, etc.) is far too expensive. The integrity system is deliberately simple:

- It counts destroyed pieces, not load paths.
- All structural pieces are treated equally (a wall contributes the same as a column).
- Collapse is binary: above threshold = stable, below = collapse.

This is a gameplay system, not an engineering simulation. It's predictable, designable (adjust the threshold per building type), and cheap.

---

## 6. Networking considerations (future)

For multiplayer, destruction state must replicate:

- **Damage-state swaps** replicate as a single enum per piece (Intact/Damaged/Destroyed). Cheap.
- **Chaos destruction** does not replicate physics frame-by-frame. Instead: the server decides when a piece is destroyed, replicates the event, and each client runs its own local Chaos simulation. Results differ slightly per client but are visually similar enough.
- **Structural integrity** is server-authoritative. The server tracks integrity counters and triggers collapse events that replicate to all clients.

This is future work and not needed for the initial implementation.

---

## 7. Implementation phases

| Phase | Work | Depends on |
|-------|------|-----------|
| **1 -- Damage states** | Add Intact/Damaged/Destroyed mesh variants for 3--5 core kit pieces. Implement mesh swap on damage. Test with a weapon hitting a wall. | Modular building kit (Phase 1 from building architecture doc) |
| **2 -- Chaos destruction** | Create Geometry Collections for core kit pieces. Implement the destruction budget manager. Test Chaos shattering within budget limits on a single building. Profile CPU. | Phase 1 |
| **3 -- Distance tiering** | Implement DestructionRadius-based tiering: full Chaos near, damage-state swap far. Test in a 50-building city with an explosion. | Phase 2 |
| **4 -- Structural integrity** | Add per-floor structural tracking. Implement collapse trigger and sequence. Test: destroy enough walls on a floor, watch the floor above collapse. | Phase 2 |
| **5 -- Scale and budget tuning** | Test in a 200+ building city battle scenario. Tune all budget parameters. Implement dynamic fallback under frame pressure. Profile extensively. | Phases 3, 4 |
| **6 -- Polish** | Particle effects (dust, sparks, fire), sound design, screen shake. Debris material variation (concrete chunks vs metal shards based on kit piece type). | Phase 5 |

---

## 8. References

- [Chaos Destruction Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-destruction-overview-in-unreal-engine)
- [Geometry Collections](https://dev.epicgames.com/documentation/en-us/unreal-engine/geometry-collections-in-unreal-engine)
- [Fracture Tools](https://dev.epicgames.com/documentation/en-us/unreal-engine/fracture-tools-in-unreal-engine)
- Modular building architecture: `docs/technical/modular-building-architecture.md`
- Building types and locations: `docs/game-design/old-docs/buildings.md`
