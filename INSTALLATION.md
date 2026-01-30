# Installation Guide

Complete setup guide for Federation Game development environment.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Step 1: Install Git and Git LFS](#step-1-install-git-and-git-lfs)
3. [Step 2: Install Visual Studio 2022](#step-2-install-visual-studio-2022)
4. [Step 3: Install Unreal Engine 5](#step-3-install-unreal-engine-5)
5. [Step 4: Clone the Repository](#step-4-clone-the-repository)
6. [Step 5: Open the Project](#step-5-open-the-project)
7. [Step 6: Set Up Your Editor (Optional)](#step-6-set-up-your-editor-optional)
8. [Troubleshooting](#troubleshooting)

---

## Prerequisites

| Requirement | Minimum | Recommended |
|-------------|---------|-------------|
| OS | Windows 10 (64-bit) | Windows 11 |
| RAM | 16 GB | 32 GB |
| Storage | 150 GB free | 250 GB SSD |
| GPU | DirectX 12 compatible | RTX 3060 or better |

---

## Step 1: Install Git and Git LFS

### Install Git

1. Download Git from [git-scm.com](https://git-scm.com/download/win)
2. Run the installer with default settings
3. Verify installation:
   ```powershell
   git --version
   ```

### Install Git LFS

Git LFS (Large File Storage) is **required** for this project. It handles binary assets like textures, models, and Unreal assets.

1. Download Git LFS from [git-lfs.com](https://git-lfs.com/)
2. Run the installer
3. Initialize Git LFS globally:
   ```powershell
   git lfs install
   ```
4. Verify installation:
   ```powershell
   git lfs --version
   ```

---

## Step 2: Install Visual Studio 2022

Visual Studio provides the C++ compiler that Unreal Engine uses to build the game.

> **Note:** You'll write code in your preferred editor (Cursor, VS Code, etc.), but Visual Studio must be installed for its compiler.

### Download

1. Go to [visualstudio.microsoft.com](https://visualstudio.microsoft.com/downloads/)
2. Download **Visual Studio 2022 Community** (free)

### Installation

Run the installer and select these workloads:

#### Required Workloads

- **Desktop development with C++**
  - This is the core requirement for UE5

#### Required Individual Components

In the "Individual components" tab, ensure these are selected:

- **MSVC v143 - VS 2022 C++ x64/x86 build tools** (latest version)
- **Windows 10/11 SDK** (latest version, e.g., 10.0.22621.0)
- **C++ CMake tools for Windows**

#### Optional but Recommended

- **Game development with C++**
  - Includes additional tools useful for game development

### Verify Installation

After installation, open "Developer PowerShell for VS 2022" and run:
```powershell
cl
```
You should see the Microsoft C++ compiler version info.

---

## Step 3: Install Unreal Engine 5

### Install Epic Games Launcher

1. Download from [epicgames.com](https://www.epicgames.com/store/download)
2. Install and create/sign in to your Epic Games account

### Install Unreal Engine 5.4+

1. Open the Epic Games Launcher
2. Click **Unreal Engine** in the left sidebar
3. Click **Library** tab
4. Click the **+** button next to "Engine Versions"
5. Select version **5.4.x** (or latest 5.4+)
6. Click **Install**

### Installation Options

When prompted, you can customize the installation:

| Option | Recommendation |
|--------|----------------|
| Location | Default or SSD with 100GB+ free |
| Target Platforms | Keep Windows selected |
| Starter Content | Optional (not used by this project) |

> **Note:** This download is large (~50-100 GB) and may take 1-2 hours.

### Verify Installation

1. In Epic Games Launcher, click **Launch** next to UE 5.4
2. The Unreal Project Browser should open
3. Close it for now - we'll open our project next

---

## Step 4: Clone the Repository

### Clone with Git LFS

```powershell
# Navigate to where you want the project
cd C:\Projects  # or your preferred location

# Clone the repository
git clone https://github.com/TomHolownia/federation-game.git

# Enter the project directory
cd federation-game

# Verify Git LFS files are downloaded
git lfs pull
```

### Verify Clone

```powershell
# Check that LFS is tracking files correctly
git lfs ls-files
```

If you see a list of tracked file types (or "No files" if assets haven't been added yet), LFS is working correctly.

---

## Step 5: Open the Project

### First Time Setup

1. Navigate to the project folder in File Explorer
2. Right-click on `FederationGame.uproject`
3. Select **Generate Visual Studio project files**
   - This creates the `.sln` and project files
   - Wait for the process to complete

### Open in Unreal Editor

1. Double-click `FederationGame.uproject`
2. If prompted about missing modules, click **Yes** to rebuild
3. Wait for shaders to compile (first launch takes 10-30 minutes)

### Verify Everything Works

1. In the Content Browser, you should see the project folders (Core, Galaxy, etc.)
2. Click **Play** to run the empty scene
3. No errors should appear in the Output Log

---

## Step 6: Set Up Your Editor (Optional)

### Cursor / VS Code

For the best experience with this project, use Cursor or VS Code:

1. Open the `federation-game` folder in Cursor/VS Code
2. Install recommended extensions:
   - **C/C++** (Microsoft)
   - **Unreal Engine 4 Snippets** (works with UE5)

### Configure Unreal for External Editor

1. In Unreal Editor: **Edit > Editor Preferences**
2. Search for "Source Code Editor"
3. Set to **Visual Studio Code** or your preferred editor
4. Now double-clicking on C++ files will open them in your editor

### Enable Live Coding

For instant code updates without restarting the editor:

1. **Edit > Editor Preferences**
2. Search for "Live Coding"
3. Enable **Enable Live Coding**
4. Set hotkey (default: Ctrl+Alt+F11)

Now when you save C++ files, press the hotkey to compile and apply changes without restarting.

---

## Troubleshooting

### "Failed to generate project files"

**Cause:** Visual Studio or required components not installed correctly.

**Fix:**
1. Open Visual Studio Installer
2. Modify your installation
3. Ensure "Desktop development with C++" is selected
4. Ensure Windows SDK is installed

### "Git LFS: pointer file" errors

**Cause:** LFS files weren't downloaded properly.

**Fix:**
```powershell
git lfs install
git lfs pull --all
```

### Shader compilation hangs

**Cause:** First-time shader compilation can take a very long time.

**Fix:**
- Be patient - this is normal for first launch
- Check Task Manager - "ShaderCompileWorker" processes should be running
- Consider closing other applications to speed this up

### "Module 'FederationGame' could not be found"

**Cause:** Project hasn't been built yet.

**Fix:**
1. Right-click `FederationGame.uproject`
2. Select "Generate Visual Studio project files"
3. Open the generated `.sln` in Visual Studio
4. Build the solution (Ctrl+Shift+B)

### Live Coding not working

**Cause:** Live Coding requires specific setup.

**Fix:**
1. Ensure no compile errors exist
2. Use Ctrl+Alt+F11 (or your configured hotkey)
3. Check Output Log for "Live Coding" messages

---

## Next Steps

After installation:

1. Read [AGENTS.md](AGENTS.md) if you're an AI agent
2. Check [TODO.md](TODO.md) for available tasks
3. Review `.cursor/rules/` for project conventions
4. Explore `docs/technical/` for technical documentation

---

## Getting Help

- **Project Issues:** Create an issue on GitHub
- **UE5 Questions:** [Unreal Engine Documentation](https://docs.unrealengine.com/)
- **Contact:** tom.holownia15@gmail.com

---

*Last updated: 2026-01-30*
