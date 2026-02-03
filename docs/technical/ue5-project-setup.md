# Unreal Engine 5 Project Setup Guide

Setup steps for the Federation Game UE5 project.

## Prerequisites

1. **Unreal Engine 5.4+** installed via Epic Games Launcher
2. **Visual Studio 2022** with the following workloads:
   - "Desktop development with C++"
   - ".NET desktop development" (for UE5 tools)
   - Individual component: "Windows 10/11 SDK"
3. **Git LFS** installed and initialized:
   ```bash
   git lfs install
   ```

## Step 1: Create the UE5 Project

1. Open **Unreal Engine 5.4+** from the Epic Games Launcher
2. Select **Games** category
3. Choose **Blank** template
4. Configure the project:
   - **Project Type:** C++ (important - not Blueprint only)
   - **Quality Preset:** Maximum
   - **Raytracing:** Enabled (optional, for high-end systems)
   - **Starter Content:** No (we'll add our own assets)
   - **Project Name:** `FederationGame`
   - **Project Location:** This repository root (`federation-game/`)
5. Click **Create** (project generation may take a few minutes).

## Step 2: Verify Project Files

After creation, you should see these new files in the repository:

```
federation-game/
├── FederationGame.uproject      # Main project file
├── Config/
│   ├── DefaultEditor.ini
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   └── DefaultInput.ini
├── Content/                     # Already created with folder structure
├── Source/
│   └── FederationGame/
│       ├── FederationGame.Build.cs
│       ├── FederationGame.cpp
│       ├── FederationGame.h
│       └── FederationGameGameModeBase.cpp/h
```

## Step 3: Enable Large World Coordinates

Required for galaxy-scale positioning. In Unreal Editor:
1. Open the project
2. Go to **Edit > Project Settings**
3. Navigate to **Engine > General Settings**
4. Find and enable **Enable Large Worlds**
5. Restart the editor when prompted.

With LWC, `FVector` is double-precision for galaxy-scale coordinates.

## Step 4: Configure Recommended Settings

### Engine Settings

In **Project Settings > Engine**:

| Setting | Value | Location |
|---------|-------|----------|
| Enable Large Worlds | ✓ | General Settings |
| Use World Partition | ✓ | World Settings (per level) |
| Enable Nanite | ✓ | Rendering |
| Global Illumination | Lumen | Rendering |

### Editor Settings

In **Project Settings > Editor**:

- Enable "Load Assets on Demand" for better editor performance with large projects

## Step 5: Verify Folder Structure

The project should use our pre-created folder structure:

### Content Folders
```
Content/
├── Core/       # Shared utilities, base classes
├── Galaxy/     # Galaxy-level systems
├── SolarSystem/# Solar system scale
├── Planet/     # Planetary systems
├── UI/         # User interface assets
├── Characters/ # Player and NPC characters
├── Vehicles/   # Ships, ground vehicles
└── Audio/      # Sound effects, music
```

### Source Folders
```
Source/FederationGame/
├── Core/       # Core utilities
├── Galaxy/     # Galaxy systems
├── Combat/     # Combat systems
└── Tests/      # Unit tests
```

## Step 6: Configure Source Control

`.gitignore` and `.gitattributes` are pre-configured. Verify Git LFS:
```bash
git lfs track
```

You should see entries for `.uasset`, `.umap`, `.png`, `.fbx`, etc.

## Step 7: Build and Test

### Build from Editor

**Platforms > Windows > Build**. First build ~5–10 min; check Output Log for errors.

### Build from Command Line

```bash
# Windows (from repository root)
"C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" ^
    FederationGame Win64 Development ^
    -Project="FederationGame.uproject" ^
    -WaitMutex

# Or use the provided build script (if available)
.\Scripts\build.bat
```

### Run the Project

Press **Play**; confirm no console errors.

## Step 8: Create Initial Commit

After verifying everything works:

```bash
git add .
git status  # Verify correct files are staged
git commit -m "[FED-001] Add UE5 project structure

- Created FederationGame UE5 project with C++ support
- Enabled Large World Coordinates for galaxy scale
- Set up Content and Source folder structure
- Configured recommended engine settings"
```

## Troubleshooting

### "Failed to compile"

Visual Studio 2022 with C++ workloads. Right-click `.uproject` → "Generate Visual Studio project files", then build from the `.sln`.

### Git LFS issues

If you see errors about large files:
```bash
git lfs install
git lfs pull
```

### Missing shaders

First launch compiles shaders (10–30 min is normal).

## Next Steps

After completing setup:

1. [ ] Create an initial test level in `Content/Core/Maps/`
2. [ ] Set up the automated testing framework ([FED-004])
3. [ ] Configure CI/CD pipeline ([FED-015])

---

*Last updated: 2026-02-03*
