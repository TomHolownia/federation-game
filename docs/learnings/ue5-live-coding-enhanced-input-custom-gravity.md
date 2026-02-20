# UE5 Learnings: Live Coding, Enhanced Input, Custom Gravity

This file captures a handful of practical UE5 “gotchas” encountered while implementing the FED-039 mannequin first-person character + a small “planet” (sphere) test environment.

## Live Coding / Hot Reload: `NewObject` in constructors can crash

- **Symptom**: Editor crashes during Live Coding / Hot Reload with an error similar to:
  - `NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor)`
- **Cause**: Calling `NewObject<>()` inside a UObject-derived constructor (directly or indirectly) is not allowed and can produce inconsistent object names.
- **Fix**:
  - Only create *default subobjects* in constructors via `ObjectInitializer.CreateDefaultSubobject<>` / `CreateDefaultSubobject<>`.
  - If you need to create UObjects via `NewObject`, do it in a safe lifecycle point (e.g. `BeginPlay`, `SetupPlayerInputComponent`, etc.), not during construction.

## Enhanced Input: binding order vs action creation

- **Symptom**: WASD / Space does nothing even though mappings “exist”.
- **Common pitfall**: `SetupPlayerInputComponent()` is called on **possession**, which typically occurs **before** the Pawn’s `BeginPlay()`.
  - If input actions / mapping contexts are created in `BeginPlay`, then at the time `SetupPlayerInputComponent()` runs the actions can still be null → bindings are skipped.
- **Fix pattern**:
  - Ensure actions + mapping context exist **before** binding.
  - Create runtime default actions/IMC inside `SetupPlayerInputComponent()` (or earlier), then bind immediately.
  - Add the mapping context to `UEnhancedInputLocalPlayerSubsystem` once the `LocalPlayer` exists (often in `BeginPlay`).

## World Settings can silently override your GameMode (flying camera surprise)

- **Symptom**: You spawn as a free-flying editor camera / default spectator pawn instead of your intended character.
- **Cause**: The level’s **World Settings → Game Mode Override** overrides project defaults.
- **Fix**:
  - In the level, set **Game Mode Override** to your intended GameMode (or clear it to use project defaults).
  - Verify **Default Pawn Class** is your character (or a BP subclass of it).

## “Planet” sphere placement: StaticMeshActor can spawn with `Static Mesh = None`

- **Symptom**: The actor exists in the Outliner but nothing is visible; Static Mesh is `None`.
- **Cause**: JSON/property-driven placement can fail to resolve asset references, especially when relying on string paths only.
- **Fix**:
  - In editor placement code, explicitly load and assign a known engine mesh (e.g. `/Engine/BasicShapes/Shape_Sphere.Shape_Sphere`, fallback to `/Engine/BasicShapes/Sphere.Sphere`) when `PlanetRadius` is specified.

## Custom gravity pitfalls (radial gravity around a sphere)

- **Gravity target identification**
  - If gravity is “toward the planet”, you need a reliable way to find the planet actor.
  - **Recommendation**: Tag the planet actor with an actor tag like `Planet` and search by tag.

- **Fallbacks can backfire**
  - A “pick the largest StaticMeshActor as planet” fallback can accidentally select a flat floor, causing gravity to pull toward the floor’s center.
  - If you add a fallback, restrict it to “planet-like” meshes (e.g. roughly uniform scale) and reset gravity to world down if no planet is found.

- **Movement / floor detection**
  - With custom gravity, the engine’s notion of “walkable floor” and “down” can get tricky.
  - If you slide or can’t stand/jump, you may need to tune:
    - `GroundFriction`
    - `BrakingDecelerationWalking`
    - Walkable floor thresholds (e.g. `SetWalkableFloorZ(...)`)

## Windows/PowerShell build quirks (Build.bat)

- PowerShell does not support `&&` as a statement separator (use `;`).
- To run `Build.bat` from PowerShell reliably, use the call operator `&`:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" `
  FederationEditor Win64 Development `
  -Project="C:/Users/TomHo/Documents/Code/Workspace/federation-game/federation.uproject" `
  -WaitMutex
```

