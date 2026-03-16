# Planet Rendering Architecture

Technical reference for the spherical planet LOD and rendering system. This document covers the theory and design for the `USphericalQuadtreeLOD` system to be implemented in a future PR.

**Status: Design document. Not yet implemented.**

---

## 1. The Cube-Sphere: Why This Shape

A sphere cannot be divided into a flat regular grid without distortion. Common projections (equirectangular, Mercator) have singularities at the poles and non-uniform cell areas. We use a **cube-sphere** instead:

1. Start with a unit cube (6 faces).
2. For every point on a cube face, normalize the vector from the cube's center to that point — this projects the cube face onto the unit sphere.
3. Each face maps to a patch of the sphere with bounded, manageable distortion (~40% max area distortion at cube corners vs. ~200%+ for polar regions in equirectangular).

Each of the 6 faces is the root node of its own quadtree. This gives us a **6-tree** that covers the entire sphere without singularities.

**References:**
- Acko.net "Making Worlds 1 — Of Spheres and Cubes": the canonical visual explanation of cube-sphere distortion and parameterization.
- vterrain.org "Spherical LOD": comprehensive survey of all spherical LOD approaches.

---

## 2. LOD System Design

### Subdivision Criterion

A chunk subdivides when its **screen-space solid angle** exceeds a threshold — i.e., when the chunk is large enough on screen that its low-resolution geometry is visibly too coarse.

```
ScreenAngularSize = ChunkAngularRadius / CameraFov
if ScreenAngularSize > SubdivisionThreshold: subdivide
if ScreenAngularSize < MergeThreshold: merge
```

Angular radius in world space:
```
ChunkAngularRadius = atan(ChunkWorldRadius / DistanceToCamera)
```

This is **view-dependent LOD**: a chunk directly ahead at 1km subdivides; the same chunk 90° to the side at 1km does not.

### LOD Levels and Resolutions

For a planet at 1/10 real Earth scale (radius ~6.37e8 UU):

| LOD | Chunk count | Patch radius (UU) | Visible from |
|-----|------------|------------------|-------------|
| 0 | 6 | ~4e8 | Deep space |
| 3 | 384 | ~5e7 | High orbit |
| 6 | 24,576 | ~6e6 | Low orbit / atmosphere |
| 10 | 6,291,456 | ~390,000 | Low altitude flight |
| 14 | ~1.6 billion | ~24,000 | Walking |
| 18+ | — | ~1,500 | On-foot detail |

In practice, LOD 10–14 will be the walking range. Levels above 14 are used for close-up terrain features and small props. Maximum active leaf chunks at any time should be bounded to ~512–1024 visible chunks to maintain performance.

### Chunk Geometry

Each chunk is a subdivided quad mesh (default 32×32 vertices). At generation:
1. Each vertex position is computed from the cube-face UV and projected onto the sphere.
2. Height displacement is applied along the sphere normal (radial direction).
3. Normals are computed from the displaced surface (cross product of tangent vectors).
4. UVs are assigned in cube-face space for consistent tiling.

Total mesh memory per chunk at 32×32: ~120KB (positions + normals + UVs). Budget ~60MB for 512 active chunks.

---

## 3. Implementation: `USphericalQuadtreeLOD`

### Planned Class Structure

```cpp
// Source/federation/Planet/SphericalQuadtreeLOD.h (to be created)

struct FPlanetChunk
{
    int32 Face;           // 0–5 (cube face index)
    int32 LODLevel;       // subdivision depth
    FVector2D FaceUVMin;  // chunk region in cube-face UV space
    FVector2D FaceUVMax;
    TObjectPtr<UProceduralMeshComponent> Mesh;
    bool bSubdivided;
    TArray<FPlanetChunk*> Children; // 4 children when subdivided
};

UCLASS()
class USphericalQuadtreeLOD : public UActorComponent
{
    // ...
    void TickComponent(...) override; // evaluates split/merge each frame
    void SubdivideChunk(FPlanetChunk* Chunk);
    void MergeChunk(FPlanetChunk* Chunk);
    void AsyncGenerateChunkMesh(FPlanetChunk* Chunk);
    // ...
};
```

### Async Mesh Generation Pipeline

```
GameThread: Decide chunk needs to be created
    → Launch AsyncTask (TaskGraph or UE5 Tasks):
        1. Evaluate height noise for each vertex in planet-local coords
        2. Compute displaced positions, normals, UVs
        3. Return FProcMeshSection data to game thread
    → GameThread: Create UProceduralMeshComponent
    → Set mesh section data
    → Enable collision if at walking LOD
    → Show chunk, hide parent
```

Chunk creation must be **look-ahead predicted** based on player velocity to avoid pop-in. If the player is moving at 1000 UU/s and chunk generation takes 50ms, the chunk must be requested when the player is at least 50 UU away from the trigger distance.

---

## 4. Neighbor Stitching

When adjacent chunks are at different LOD levels, T-junctions appear where the higher-LOD chunk has more edge vertices than its lower-LOD neighbor expects. Two approaches:

### Skirts (Recommended for First Implementation)

Extend each chunk's edges downward (inward toward the planet center) by a skirt height equal to the maximum possible height delta between the chunk and its neighbor. Skirts are cheap to generate and eliminate visible seams regardless of LOD difference. The downside is a small amount of overdraw at chunk edges.

### Vertex Morphing (GEOMORPHS — Future Optimization)

Blend vertex positions smoothly between LOD levels during transitions. Eliminates pop artifacts but requires tracking per-vertex morph targets and adds complexity.

**Start with skirts.**

---

## 5. Terrain Generation on a Sphere

### Noise Evaluation in Spherical Coordinates

**Never evaluate noise using world XY.** At planet scale, world XY causes seams at cube face boundaries and non-uniform noise distribution.

Instead:
1. Convert chunk vertex from cube-face UV to unit sphere normal: `Normal = normalize(CubeFaceUVtoXYZ(UV))`
2. Evaluate 3D noise at `Normal * NoiseScale`: `Height = FractalBrownianMotion(Normal * NoiseScale, Octaves)`
3. Apply height along the radial direction: `VertexPos = PlanetCenter + Normal * (PlanetRadius + Height * HeightScale)`

This gives **seamless, rotationally consistent terrain** with no UV seams.

### Biome Assignment

Biomes are determined by:
- **Latitude**: dot product of the surface normal with the planet's north pole vector
- **Altitude**: height above sea level
- **Noise-based variation**: fractional brownian motion layer for natural boundaries

Biome determines material blend weights (rock, snow, grass, sand, etc.) which drive the terrain material's layer blend parameters.

### Triplanar Texturing

Surface materials use **triplanar projection** — the material samples three texture axes (XY, XZ, YZ planes) and blends by the surface normal. This ensures textures tile correctly on curved surfaces regardless of the cube-face UV layout.

```hlsl
// Pseudocode for triplanar sampling in UE5 material graph
float3 blend = pow(abs(Normal), TriplanarSharpness);
blend /= dot(blend, 1.0);
float4 xSample = Texture.Sample(Normal.yz * Scale);
float4 ySample = Texture.Sample(Normal.xz * Scale);
float4 zSample = Texture.Sample(Normal.xy * Scale);
return xSample * blend.x + ySample * blend.y + zSample * blend.z;
```

---

## 6. Actor Placement on a Sphere

All actors placed on a planet surface must be oriented relative to the planet core, not the world axes.

### Local Frame Construction

```cpp
FVector SurfaceUp = (ActorLocation - PlanetCenter).GetSafeNormal();
FVector TangentX = FVector::CrossProduct(SurfaceUp, FVector::UpVector).GetSafeNormal();
if (TangentX.IsNearlyZero())
{
    TangentX = FVector::CrossProduct(SurfaceUp, FVector::ForwardVector).GetSafeNormal();
}
FVector TangentY = FVector::CrossProduct(SurfaceUp, TangentX);
FMatrix LocalToWorld = FMatrix(TangentX, TangentY, SurfaceUp, ActorLocation);
```

`UPlanetGravityComponent::GetGravityUp()` returns `SurfaceUp` for the player's current position — reuse this for NPC and prop placement.

### PCG Integration (Future)

UE5's Procedural Content Generation (PCG) graph can scatter actors on a sphere by:
1. Using a custom PCG point source that generates points on the sphere surface
2. Applying the local frame (above) to each point's transform
3. Feeding the PCG graph planet parameters (seed, biome map, density curves)

This enables entire forests, boulder fields, and settlement clusters to be generated deterministically from planet parameters.

---

## 7. Depth Buffer — Dual Frustum

### The Problem

Standard depth buffer precision requires a near/far ratio of at most ~100,000:1 for acceptable Z-fighting. From orbit:
- Near plane: spaceship at 1m → `Near = 1`
- Far plane: planet surface 40,000 km away → `Far = 4e10`
- Ratio: 4e10 — far beyond acceptable range

### Solution: Two Render Passes

**Pass 1 — Space Backdrop**
- Near: 1e6 UU (1km)
- Far: 1e13 UU (1 AU)
- Renders: stars, distant planet spheres (low-poly + baked normal map), nebulae
- Depth buffer cleared after this pass

**Pass 2 — Local Scene**
- Near: 1 UU (1cm)
- Far: 5e7 UU (50km) when on surface; 5e8 UU (500km) when in orbit
- Renders: ship, terrain, characters, nearby props, nearby planet surface

The local near/far ratio is always within the acceptable range. Distant planet bodies rendered in Pass 1 never compete with local geometry in Pass 2.

### UE5 Implementation

UE5 does not natively expose multi-frustum rendering for this use case. Implementation requires either:
- A custom rendering extension (SceneViewExtension) that injects an additional depth-only pre-pass
- Or using the existing sky atmosphere system's "skybox depth" as the space backdrop pass

This is a non-trivial rendering engineering task. **Deferred until the quadtree system is functional.**

At early development scale (planets at 2e6 UU with the player rarely more than 1e5 UU from the surface), the default UE5 depth buffer with reverse-Z is sufficient.

---

## 8. Distant Planet Representation

Planets not currently being approached are rendered as:
1. **LOD 0 sphere mesh** — the `APlanet` static mesh (a plain sphere at planet scale)
2. **Baked normal map** — a single 2K or 4K normal map capturing continent-scale terrain silhouette
3. **Atmosphere material** — a simple additive sphere slightly larger than the planet for atmospheric glow

No terrain chunks are active for distant planets. The switch to full LOD (activating `USphericalQuadtreeLOD`) happens when the player enters the planet's sphere of influence (defined by `MaxInfluenceDistanceMultiplier` on `UPlanetGravitySourceComponent`).

---

## 9. Reference Implementations

| Reference | What it demonstrates |
|-----------|---------------------|
| [PlanetaryTerrain (Unity, GitHub)](https://github.com/mathis-s/PlanetaryTerrain) | Quadtree LOD on cube-sphere. Algorithm is directly portable to UE5 despite Unity origin. |
| [Outerra: Logarithmic Depth Buffer (2009)](https://outerra.blogspot.com/2009/08/logarithmic-z-buffer.html) | Canonical reference for log depth buffer — useful if the dual-frustum approach proves insufficient. |
| [Outerra: Log Depth Optimizations (2013)](https://outerra.blogspot.com/2013/07/logarithmic-depth-buffer-optimizations.html) | Fragment-shader implementation of log depth. |
| [Acko.net Making Worlds 1](https://acko.net/blog/making-worlds-1-of-spheres-and-cubes/) | Visual explanation of cube-sphere parameterization and distortion. |
| [Elite Dangerous: Generating the Universe](https://80.lv/articles/generating-the-universe-in-elite-dangerous) | Elite's cube-face quadtree, tri-planar texturing, and 64-bit noise evaluation. |
| [vterrain.org Spherical LOD](http://vterrain.org/LOD/spherical.html) | Survey of all known spherical LOD techniques. |
| [Cesium for Unreal (GitHub)](https://github.com/CesiumGS/cesium-unreal) | Production UE5 implementation of spherical tile streaming (3D Tiles). Multi-frustum depth management source available. |
| [Depth Precision Visualized — Nathan Reed](https://www.reedbeta.com/blog/depth-precision-visualized/) | Excellent mathematical explanation of depth buffer precision across projection types. |
