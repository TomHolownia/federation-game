# Asset Creation Workflow Research

**Task:** FED-026 - Figure out assets creation workflow

## Problem Statement

We have figured out our method of automating placement of assets by using multiple levels with FederationEditor. However, we are going to need a lot of assets to create a galaxy. This document addresses:

1. **Where should we get generic assets from?**
2. **How do we build the custom assets we need?**
3. **How do we add variety to assets so that if we place two versions of Asset A, they don't look exactly the same?**
4. **How do we import thousands of assets into our project without Tom having to manually import them all?**

---

## 1. Where to Get Generic Assets

### Free Sources

#### Epic Games Fab Marketplace
- **Primary resource:** Epic Games partners with Fab creators to offer free assets that rotate every two weeks
- **Permanent library:** Fab maintains a permanent library of free resources including high-quality Megascans
- **Access:** Browse at [fab.com](https://fab.com) or [unrealengine.com/fabfreecontent](https://unrealengine.com/en-US/fabfreecontent)
- **Quality:** High-quality, production-ready assets
- **License:** Check individual asset licenses (most are CC0 or similar)

#### Unreal Engine Marketplace (Free Section)
- **Official source:** UE Marketplace has a dedicated free assets section
- **Filtering:** Can filter by content type (environments, props, characters, etc.)
- **Access:** [unrealengine.com/marketplace/free](https://www.unrealengine.com/marketplace/en-US/free)
- **Update frequency:** New free content added regularly

#### Asset Aggregators
- **Asset Freaks:** Aggregates free UE5 assets from Fab, including sci-fi specific items
- **Access:** [assetfreaks.com](https://assetfreaks.com)

### Paid Sources (Consider for Quality/Volume)

#### Unreal Engine Marketplace
- **POLYGON - Sci-Fi Space Pack:** Low-poly modular space game assets for environments
- **Space Frontier Stations & Ships:** Environment assets featuring space stations and ships
- **Quality:** Professional-grade assets, often optimized for UE5
- **Cost:** Varies ($10-$100+ per pack)

#### Megascans (via Quixel Bridge)
- **Integration:** Built into UE5 via Quixel Bridge
- **Content:** Photorealistic scanned assets (rocks, surfaces, vegetation)
- **Use case:** Planetary surfaces, natural environments
- **Cost:** Free for Unreal Engine projects

### Recommendations

1. **Start with Fab free content** - High quality, regularly updated, free
2. **Use Megascans for natural surfaces** - Integrated, free, photorealistic
3. **Purchase key packs** - For specific needs (ships, stations) where free options are limited
4. **Build custom assets** - For unique, game-specific content (see section 2)

---

## 2. How to Build Custom Assets

### External Tools

#### Blender (Recommended - Free)
- **Cost:** Free and open-source
- **Workflow:** Epic provides "Blender to Unreal Essentials" tutorial
- **Best for:** General 3D modeling, UV mapping, basic animation
- **Integration:** Direct export to FBX/GLTF for UE5
- **Learning curve:** Moderate, extensive community resources

#### Maya (Professional)
- **Cost:** Paid subscription
- **Workflow:** Epic provides documentation for Maya users
- **Best for:** Complex character rigging, advanced animation
- **Integration:** Direct FBX export, Maya Live Link plugin available
- **Learning curve:** Steeper, industry standard

#### Other Tools
- **3ds Max:** Alternative to Maya, similar capabilities
- **ZBrush:** High-detail sculpting for characters/props
- **Substance Painter:** Texture painting and material authoring
- **Houdini:** Procedural asset generation (advanced)

### Import Workflow

#### Standard Import Process
1. **Create asset** in external tool (Blender/Maya/etc.)
2. **Export** as FBX or GLTF
3. **Import into UE5** via:
   - Drag and drop into Content Browser
   - Content Browser → Import button
   - Right-click → Import to Current Folder

#### Import Settings
- **Static Meshes:** Enable "Import Mesh", set LODs if available
- **Skeletal Meshes:** Enable "Import Skeleton", "Import Animations"
- **Textures:** Auto-detects UDIM tiles during import
- **Materials:** Can auto-create materials or import existing

### Recommendations

1. **Use Blender for most custom assets** - Free, capable, well-documented
2. **Establish asset standards** - Consistent naming, scale, pivot points
3. **Create asset templates** - Base meshes for common types (ships, stations, buildings)
4. **Document workflow** - Keep notes on import settings, naming conventions

---

## 3. Adding Variety to Assets

### Instanced Static Meshes (ISM)

**What it is:** Render multiple copies of the same mesh efficiently with a single draw call.

**Creating ISMs:**
1. Packed Level Actor
2. Harvest Instances
3. Spawn in Blueprints/C++

**Performance benefit:** Significant performance improvement vs individual static meshes.

### Material Variations

#### Per Instance Custom Data
- **Method:** Use Per Instance Custom Data to vary colors/properties per instance
- **Implementation:** Set custom data (4 floats: R, G, B, Intensity) per instance
- **Material:** Material reads custom data to vary appearance
- **Use case:** Color variations, emissive intensity, material parameters

#### Random Colors Per Instance
- **Method:** Apply random colors through material parameters
- **Implementation:** Set random color per instance at spawn time
- **Use case:** Quick variety without custom materials

#### Smart Material Variations
- **Method:** Material variations that work across all instances
- **Implementation:** Material parameters that can be overridden per instance
- **Use case:** Different material types (metal vs painted, weathered vs new)

### Procedural Variations

#### Scale Variations
- **Method:** Randomize scale per instance (e.g., 0.8x to 1.2x)
- **Implementation:** Set scale in transform when adding instance
- **Use case:** Natural variation in size

#### Rotation Variations
- **Method:** Randomize rotation per instance
- **Implementation:** Set random rotation when adding instance
- **Use case:** Avoid uniform orientation

#### Material Parameter Variations
- **Method:** Vary material parameters (roughness, metallic, color) per instance
- **Implementation:** Use Per Instance Custom Data or Material Parameter Collections
- **Use case:** Different material states (rusty vs clean, damaged vs pristine)

### Recommendations

1. **Use ISM for repeated assets** - Performance-critical for galaxy-scale
2. **Combine multiple variation techniques** - Scale + rotation + material = significant variety
3. **Create variation presets** - Define common variation sets (e.g., "weathered", "new", "damaged")
4. **Use procedural generation** - Generate variations at runtime or during placement

---

## 4. Bulk Importing Assets

**Note:** This should extend the existing **FederationEditor** module rather than being a separate system. FederationEditor already handles actor placement via `PlaceActorsFromDataCommand`; we should add bulk import as a complementary command in the same module.

**Workflow:** Import assets (FBX/OBJ/etc.) → Creates .uasset files → Place actors using those assets (via existing placement system)

### Python Scripting (Alternative - Not Recommended for This Project)

#### Requirements
- Unreal Engine Python API (`unreal` module)
- Editor Scripting Utilities plugin (enabled)
- Python Editor Script Plugin (enabled)

#### Core APIs

**AssetImportTask (Legacy):**
```python
import unreal

task = unreal.AssetImportTask()
task.filename = "path/to/asset.fbx"
task.destination_path = "/Game/MyAssets"
task.automated = True  # Avoid dialogs
task.replace_existing = True
task.save = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
```

**Interchange Framework (Recommended):**
```python
import unreal

params = unreal.ImportAssetParameters()
params.is_automated = True  # Run without UI dialogs
params.override_pipelines = []  # Custom import pipelines
# Set callbacks for progress tracking
unreal.InterchangeManager.import_asset(...)
```

#### Bulk Import Script Example

```python
import unreal
import os

def bulk_import_assets(source_dir, destination_path):
    """
    Import all FBX files from source_dir into destination_path
    """
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    tasks = []
    
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file.endswith(('.fbx', '.obj', '.gltf')):
                source_path = os.path.join(root, file)
                relative_path = os.path.relpath(root, source_dir)
                dest_path = os.path.join(destination_path, relative_path).replace('\\', '/')
                
                task = unreal.AssetImportTask()
                task.filename = source_path
                task.destination_path = dest_path
                task.automated = True
                task.replace_existing = True
                task.save = True
                tasks.append(task)
    
    if tasks:
        asset_tools.import_asset_tasks(tasks)
        print(f"Imported {len(tasks)} assets")

# Usage
bulk_import_assets("C:/MyAssets", "/Game/ImportedAssets")
```

### C++ Implementation (Recommended - Extend FederationEditor)

**Approach:** Add bulk import functionality directly to the FederationEditor module, similar to how `PlaceActorsFromDataCommand` works.

#### Implementation Details
- **New Class:** `FBulkImportAssetsCommand` in `Source/federationEditor/BulkImportAssetsCommand.h/cpp`
- **API:** Use `FAssetToolsModule` and `AssetImportTask` (C++ equivalent of Python API)
- **Integration:** Register in `FfederationEditorModule::StartupModule()` alongside `PlaceActorsFromDataCommand`
- **Menu:** Add to Tools → Federation menu (same location as "Place Actors From Data")
- **Advantages:** 
  - Consistent with existing codebase
  - No Python dependency
  - Better integration with UE5 editor systems
  - Can share code/utilities with placement system

### Integration with Existing Workflow

#### Extend FederationEditor Module
- **Current:** `PlaceActorsFromDataCommand` places actors from JSON data (assumes assets already exist)
- **Extension:** Add bulk import functionality to FederationEditor
- **New Command:** `BulkImportAssetsCommand` - Import external assets (FBX, OBJ, GLTF) into UE5
- **Workflow:** Import assets → Place actors using imported assets (both via FederationEditor)

#### Implementation Approach
- **Add to FederationEditor:** New C++ class `FBulkImportAssetsCommand` in `Source/federationEditor/`
- **Menu Integration:** Add to same Tools → Federation menu alongside "Place Actors From Data"
- **Functionality:** 
  - Scan directory for asset files (FBX, OBJ, GLTF, textures)
  - Import using UE5's `AssetImportTask` API
  - Maintain folder structure from source
  - Generate import manifest/log for reference
- **UI:** Simple dialog to select source directory and destination path in Content/

### Recommendations

1. **Extend FederationEditor** - Use existing module rather than separate system
2. **C++ Implementation** - More control than Python, integrates with existing codebase
3. **Two-step workflow** - Import assets first, then place actors (both via FederationEditor)
4. **Automate organization** - Maintain folder structure from source, auto-organize by type
5. **Add progress tracking** - Show import progress for large batches
6. **Handle errors gracefully** - Log failures, continue with remaining assets
7. **Optional: Combined workflow** - Future enhancement: JSON can reference external files, auto-import then place

---

## Proposed Solution

### Phase 1: Asset Acquisition
1. **Set up Fab account** - Access free rotating assets
2. **Download key free packs** - Sci-fi space assets, Megascans surfaces
3. **Identify gaps** - List assets we need but don't have
4. **Purchase if needed** - Buy specific packs for gaps

### Phase 2: Custom Asset Pipeline
1. **Set up Blender workflow** - Install, configure export settings
2. **Create asset templates** - Base meshes for common types
3. **Document standards** - Naming, scale, pivot, LOD requirements
4. **Build initial custom assets** - Unique game-specific content

### Phase 3: Variety System
1. **Implement ISM system** - Use for repeated assets
2. **Create variation materials** - Per-instance custom data materials
3. **Build variation presets** - Common variation sets
4. **Integrate with placement system** - Apply variations during JSON placement

### Phase 4: Bulk Import Automation
1. **Extend FederationEditor** - Add `FBulkImportAssetsCommand` class
2. **Implement import functionality** - Use UE5 AssetImportTask API in C++
3. **Add menu command** - Tools → Federation → Bulk Import Assets
4. **Test with sample assets** - Verify import, organization, placement workflow
5. **Document workflow** - How to use bulk import tool (import → place actors)

### Phase 5: Integration
1. **Combine workflows** - Import → Place → Vary
2. **Create end-to-end pipeline** - From asset source to placed instance
3. **Document complete workflow** - Step-by-step guide
4. **Test at scale** - Import and place hundreds/thousands of assets

---

## Next Steps

1. **Review this document** - Get feedback on proposed approach
2. **Prioritize phases** - Decide what to tackle first
3. **Start with Phase 1** - Set up asset acquisition
4. **Build incrementally** - Test each phase before moving to next

---

## References

- [Unreal Engine Python API - AssetImportTask](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/AssetImportTask)
- [Unreal Engine Interchange Framework](https://dev.epicgames.com/documentation/en-us/unreal-engine/importing-assets-using-interchange-in-unreal-engine)
- [Epic Games Fab Free Content](https://unrealengine.com/en-US/fabfreecontent)
- [Blender to Unreal Essentials](https://dev.epicgames.com/community/learning/tutorials/Ed4W/unreal-engine-blender-to-unreal-essentials)
- [Instanced Static Meshes with Efficient Materials](https://dev.epicgames.com/community/learning/tutorials/33oJ/unreal-engine-using-instanced-static-meshes-with-efficient-materials)
