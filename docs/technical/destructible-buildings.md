# Destructible Buildings

How building destruction works in Federation: rechargeable shields, Chaos Destruction on modular kit pieces, CPU budgeting, and phased implementation.

---

## 1. Goal

Buildings assembled from modular kit pieces (see `docs/technical/modular-building-architecture.md`) should be destructible during combat. Weapons fire, explosions, and vehicle impacts must first deplete a building's rechargeable shield before structural damage begins. Once shields are down, individual building pieces can be damaged and destroyed, and sufficient structural damage can cause floor collapse.

**CPU constraint:** Real-time physics destruction is expensive. The system must enforce strict CPU limits so that destruction enhances gameplay without tanking frame rate, even during large battles in dense cities. Shields serve as a natural throttle on the destruction queue -- buildings absorb sustained fire before any Chaos physics is triggered.

**Scope:** Destruction applies to **buildings and structures only**. Landscape terrain is not destructible (see section 8).

---

## 2. Rechargeable shields

All military assets in the Federation universe have rechargeable energy shields. Buildings are no exception -- every building has a shield that must be depleted before structural damage can occur.

### How building shields work

- **Shield health pool.** Each building has a shield with a configurable maximum HP. Shield HP scales with building importance: a military bunker has far more shield capacity than a coffee shop.
- **Damage absorption.** While the shield is active, all incoming damage is applied to the shield HP, not to the building structure. No kit pieces take damage, no destruction is triggered, no Chaos physics runs.
- **Visual feedback.** Shield hits produce a visual effect (ripple/flash on the shield surface, colour shift from blue/green toward red as HP drops). This tells the player the building is shielded and how close the shield is to failing.
- **Shield break.** When shield HP reaches zero, the shield drops with a visible collapse effect (shatter/flicker). From this point, all damage goes directly to building structure and the destruction system (sections 3--6) takes over.
- **Recharge.** If the building stops taking damage for a configurable duration (e.g. 10--30 seconds), the shield begins recharging. Recharge rate and delay are per-building properties. A shield that fully recharges means the attacker must sustain pressure or start over.
- **No partial structural protection.** Shields are all-or-nothing for a building. While the shield is up, the entire building is protected. There is no per-wall or per-floor shielding (this keeps the system simple and readable for the player).

### Why shields help performance

Shields are a **natural destruction budget throttle**:

- A city under bombardment doesn't immediately generate hundreds of Chaos destruction events. Most buildings are absorbing damage on their shields -- zero physics cost.
- Only buildings whose shields have actually failed enter the destruction pipeline. In a typical battle, this might be a handful at any given time, well within the destruction budget.
- Shield recharge means buildings cycle back to the protected state if the attacker shifts focus, further reducing the number of active destruction targets.

### Shield properties

| Property | Description | Example values |
|----------|-------------|---------------|
| **MaxShieldHP** | Total shield capacity | 500 (shop), 2000 (apartment), 10000 (military) |
| **RechargeDelay** | Seconds after last hit before recharge starts | 10--30s |
| **RechargeRate** | HP per second during recharge | 50--200 HP/s |
| **ShieldMaterial** | Material instance for the shield effect | Translucent, fresnel-based, colour-keyed to HP |

### Shared shield component

Since all military assets (characters, vehicles, ships, turrets, buildings) use rechargeable shields, the shield should be a shared `UShieldComponent` that attaches to any actor. Buildings, soldiers, and vehicles all use the same component with different HP/recharge values. This avoids duplicating shield logic across systems.

---

## 3. Damage pipeline summary

The full damage pipeline for a building, from first hit to rubble:

```
Incoming damage
  --> Shield absorbs (no structural effect, shield VFX)
  --> Shield breaks (shield-down VFX)
  --> Structural damage begins
  --> Material-driven scuffs (DamageLevel param)
  --> Damage-state mesh swap (Intact --> Damaged)
  --> Chaos destruction (Damaged --> Shattered/Destroyed)
  --> Structural integrity check --> possible floor collapse
```

Shields ensure only the right-hand side of this pipeline costs significant CPU, and only for buildings that are actively under sustained fire.

---

## 4. Approach: Chaos Destruction on modular pieces

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

## 5. CPU budget and limits

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

## 6. Damage-state system (cheap alternative)

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

This gives a full spectrum: shielded --> pristine --> scuffed (material) --> cracked (damaged mesh) --> shattered (Chaos) --> rubble/gone.

---

## 7. Structural integrity and floor collapse

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

## 8. Landscape is not destructible

Destruction is scoped to **buildings and structures only**. The landscape terrain itself is never deformed, cratered, or removed by weapons fire. UE5 Landscape runtime deformation is expensive, difficult to replicate in multiplayer, and hard to undo -- it would create permanent, accumulating performance and visual debt across a long play session.

### What does happen to terrain

Heavy weapons fire hitting the ground produces **visual-only effects** that persist for the duration of the battle or session:

- **Crater decals.** Large projected decals (circular scorch/crater texture) placed at the impact point. These are flat projections on the existing terrain -- no geometry change. They fade or are cleaned up after the battle ends or when the player leaves the area.
- **Blast mark decals.** Smaller scorch and burn marks from explosions, weapons fire, and debris impacts. Same decal approach.
- **Particle effects.** Dirt/dust kicked up on impact, lingering smoke for large hits. These are transient and self-clean.

### Decal budget

Decals are cheap individually but can accumulate. A decal manager limits the total count:

| Parameter | Recommended default | Purpose |
|-----------|-------------------|---------|
| **MaxTerrainDecals** | 200--500 | Total crater/blast decals in the world. When exceeded, oldest decals fade out. |
| **DecalLifetime** | 300--600 seconds | Decals fade after this duration even if under budget. |

### Why not terrain destruction

- **Performance:** Runtime Landscape heightmap modification is GPU-expensive and doesn't batch well.
- **Persistence:** Deformed terrain must be saved/replicated. Over a long session or multiplayer match, accumulated deformation becomes unmanageable.
- **Gameplay:** Terrain craters would interfere with navigation, vehicle pathing, and building placement in ways that are hard to control. Decals give the visual impact of a battlefield without the gameplay side-effects.
- **Scope:** The destruction system is complex enough with buildings. Adding terrain deformation would significantly increase implementation and testing effort for marginal gameplay benefit.

---

## 9. Networking considerations (future)

For multiplayer, destruction state must replicate:

- **Damage-state swaps** replicate as a single enum per piece (Intact/Damaged/Destroyed). Cheap.
- **Chaos destruction** does not replicate physics frame-by-frame. Instead: the server decides when a piece is destroyed, replicates the event, and each client runs its own local Chaos simulation. Results differ slightly per client but are visually similar enough.
- **Structural integrity** is server-authoritative. The server tracks integrity counters and triggers collapse events that replicate to all clients.

This is future work and not needed for the initial implementation.

---

## 10. Implementation phases

| Phase | Work | Depends on |
|-------|------|-----------|
| **1 -- Shield component** | Implement `UShieldComponent` (shared across buildings, vehicles, characters). HP, recharge delay, recharge rate, shield-hit VFX, shield-break VFX. Test on a single building: shoot until shield drops. | Modular building kit (Phase 1 from building architecture doc) |
| **2 -- Damage states** | Add Intact/Damaged/Destroyed mesh variants for 3--5 core kit pieces. Implement mesh swap on damage (only applies after shield is down). Test with a weapon hitting an unshielded wall. | Phase 1 |
| **3 -- Chaos destruction** | Create Geometry Collections for core kit pieces. Implement the destruction budget manager. Test Chaos shattering within budget limits on a single building. Profile CPU. | Phase 2 |
| **4 -- Terrain decals** | Implement crater and blast mark decal system with decal budget manager. Test with heavy weapons on open terrain. | -- |
| **5 -- Distance tiering** | Implement DestructionRadius-based tiering: full Chaos near, damage-state swap far. Test in a 50-building city with an explosion. | Phase 3 |
| **6 -- Structural integrity** | Add per-floor structural tracking. Implement collapse trigger and sequence. Test: destroy enough walls on a floor, watch the floor above collapse. | Phase 3 |
| **7 -- Scale and budget tuning** | Test in a 200+ building city battle scenario. Tune all budget parameters (destruction + shield + decal). Implement dynamic fallback under frame pressure. Profile extensively. | Phases 5, 6 |
| **8 -- Polish** | Particle effects (dust, sparks, fire), sound design, screen shake. Debris material variation (concrete chunks vs metal shards based on kit piece type). Shield VFX polish. | Phase 7 |

---

## 11. References

- [Chaos Destruction Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-destruction-overview-in-unreal-engine)
- [Geometry Collections](https://dev.epicgames.com/documentation/en-us/unreal-engine/geometry-collections-in-unreal-engine)
- [Fracture Tools](https://dev.epicgames.com/documentation/en-us/unreal-engine/fracture-tools-in-unreal-engine)
- Modular building architecture: `docs/technical/modular-building-architecture.md`
- Building types and locations: `docs/game-design/old-docs/buildings.md`
