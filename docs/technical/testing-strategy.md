# Testing Strategy

How we test C++ code in Federation Game, what frameworks UE5 provides, and when to adopt more granular testing.

## 1. Current Approach

We use UE5's **Automation Testing Framework** with `IMPLEMENT_SIMPLE_AUTOMATION_TEST`. Tests live alongside source code under `Tests/` subdirectories that mirror the module structure:

```
Source/
├── federation/Tests/
│   ├── Planet/Test_PlanetGravityComponent.cpp    (10 tests)
│   ├── Character/Test_FederationCharacter.cpp     (8 tests)
│   ├── Galaxy/Test_GalaxyStarField.cpp            (8 tests)
│   ├── Core/Test_LargeWorldCoordinates.cpp        (1 test)
│   └── Skybox/Test_SkySphere.cpp                  (2 tests)
└── federationEditor/Tests/
    ├── Test_PlacementAndDefaultMaterial.cpp        (3 tests)
    └── Test_BulkImportAssetsCommand.cpp            (2 tests)
```

Total: **34 automation tests** covering every C++ system in the project.

### What the tests cover

- **PlanetGravityComponent** — gravity direction, capsule alignment, quaternion camera orientation, pitch clamping, ground contact recovery.
- **FederationCharacter** — camera setup, first/third-person toggle, component attachment, rotation settings.
- **GalaxyStarField** — star count, seeded reproducibility, instanced meshes, spiral arm config, performance (10K stars < 1 second).
- **LargeWorldCoordinates** — position precision at 100M+ units.
- **SkySphere** — mesh existence, null-material safety.
- **Editor tools** — JSON placement config validation, default material loading, bulk import file filtering.

### How to run

In the editor: **Window → Developer Tools → Session Frontend → Automation** tab. Filter by `FederationGame` and run all.

Command line (headless):

```
UnrealEditor-Cmd.exe "federation.uproject" -ExecCmds="Automation RunTests FederationGame" -NullRHI -Unattended
```

## 2. Test Level

These are **integration tests**, not isolated unit tests. Every test spawns actors or components in a `UWorld`, which boots a chunk of the engine. This is the right level for testing "does the gravity component align the capsule correctly" or "does camera toggle work," because those behaviours depend on real engine state.

We do not currently have isolated unit tests (testing pure functions without the engine). That is intentional — see section 4.

## 3. UE5 Testing Frameworks Available

UE5 ships with several testing systems beyond what we use today:

| Framework | How it works | Best for |
|-----------|-------------|----------|
| **Simple Automation Test** | `IMPLEMENT_SIMPLE_AUTOMATION_TEST` macro. Single-frame, no fixtures. | Quick checks (what we use now). |
| **Automation Spec** | `BEGIN_DEFINE_SPEC` with `Describe()`/`It()`/`BeforeEach()`. BDD-style. | Reducing boilerplate when many tests share setup. |
| **CQTest** | `TEST_CLASS` with full fixture support (`BeforeAll`/`AfterAll`). From Rare's Sea of Thieves. | Gameplay tests at all scopes. |
| **Low-Level Tests (Catch2)** | Catch2 `TEST_CASE` compiled as a standalone executable. Runs outside the editor. | Pure logic, math, utilities — no `UWorld` needed. |
| **Functional Tests** | `AFunctionalTest` actors placed in test maps, logic in Blueprint. | Level and gameplay integration. |

## 4. When to Adopt More Granular Testing

We do not need to restructure existing code or set up additional test frameworks now. The current integration tests cover the riskiest code (gravity math, galaxy generation, coordinate precision) and the codebase is small enough that retrofitting later is cheap — the existing tests act as a safety net for any future refactor.

**When building new systems (combat, economy, faction logic):**

- If a function is pure math (damage formula, accuracy roll, economy calculation), write it as a **free function or in a namespace** rather than as a method on an Actor or Component. This makes it trivially testable in isolation later.
- If it needs engine state (collision, component lookups), keep it on the component and test it with the current integration pattern.

This is not extra work — it is normal separation of concerns. It keeps the door open for Catch2 Low-Level Tests or Automation Spec without requiring any upfront framework setup.

**Triggers for adopting new frameworks:**

- **Automation Spec or CQTest**: When test files start having significant duplicated setup code (e.g. every test spawning the same actor with the same config). These frameworks add `BeforeEach`/`AfterEach` to reduce that boilerplate.
- **Low-Level Tests (Catch2)**: When there is enough pure computational logic (combat math, economy systems, data parsers) that testing it through the editor is slow and unnecessary.
- **Functional Tests**: When gameplay scenarios (e.g. "player lands on planet, gravity transitions, camera aligns") need multi-frame, in-level verification.

## 5. CI / Automated Test Execution

There is no CI pipeline running tests automatically. Running UE5 in a container on GitHub Actions is expensive and complex for a project this size.

Current approach: run tests locally before pushing, as part of the PR workflow.

If CI becomes worthwhile later, options include:

- **Self-hosted GitHub Actions runner** on a machine with UE5 installed.
- **Pre-push script** that runs the headless command from section 1.
- **Epic's Horde build system** for larger-scale automation.

## 6. Conventions

- Test files: `Test_<ClassName>.cpp`
- Test location: `Source/<Module>/Tests/<Subdirectory>/`
- Test names: `FederationGame.<Module>.<Class>.<TestName>`
- Guard macro: `#if WITH_DEV_AUTOMATION_TESTS` / `#endif`
- Tests compile into the editor target only (no separate test target).

---

*See also: `ue5-project-setup.md` for project structure, `large-world-coordinates.md` for the LWC precision test.*
