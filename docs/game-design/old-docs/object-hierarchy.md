# Object Hierarchy

> Converted from: `ObjectHeirarchy.png`

This document describes the technical class hierarchy for game objects.

![Object Hierarchy](images/object-hierarchy.png)

## Interfaces

These interfaces define common behaviors that can be implemented by various object types:

| Interface | Description |
|-----------|-------------|
| `StopsProjectile` | Object blocks projectiles |
| `Collidable` | Object has collision detection |
| `StopsFlyers` | Object blocks flying entities |

---

## Class Hierarchy

### Java Object (Root)
Base class for all objects.

### Terrain
Environmental objects with position and visual representation.

**Attributes:**
- `HitBoxSize` - Collision bounds
- `Sprite` - Visual representation
- `XYCoordinates` - World position

#### Non-Destructable
Objects that cannot be destroyed.

| Class | Interfaces | Properties |
|-------|------------|------------|
| `BuildingWalls` | StopsProjectile, Collidable, StopsFlyers | Invincible |

#### Destructable
Objects that can be destroyed.

**Attributes:**
- `HP` - Hit points

##### Obstacle
Obstacles that block movement or projectiles.

| Class | Interfaces | Special Properties |
|-------|------------|-------------------|
| `Asteroids` | StopsProjectile, Collidable, StopsFlyers | `RotationSpeed` |
| `HighObstacle` | StopsProjectile, Collidable, StopsFlyers | - |
| `TreeObstacle` | (inherits from HighObstacle) | Has top-level overlay (visual only, doesn't affect gameplay) |
| `RegularObstacle` | StopsProjectile, Collidable | - |
| `BarricadeObstacle` | Collidable | Ledges you can shoot over |

##### Shields
Defensive objects.

| Class | Interfaces | Description |
|-------|------------|-------------|
| `Shields` | StopsProjectile | Blocks projectiles only |

---

### Entity
Mobile objects that can move and interact with the world.

**Type Enum:**
- `FLYING` - Aerial entities
- `GROUND` - Ground-based entities

#### SpaceShip
Large vessels for space travel.

**Properties:**
- Slow movement speed
- Has abstract interior with physics inverse to movement vectors if you aren't seated
- Velocity is cumulative rather than resetting
- Has Ship Inventory

#### Vehicle
Smaller ground or air vehicles.

**Properties:**
- A being can enter a vehicle
- Has set weapons with unlimited ammo
- Good shields but health cannot be regenerated
- Has inventory

#### Being
Living entities (players and NPCs).

**Properties:**
- Already well documented in [beings.md](beings.md)
- Has inventory

---

## Collision Matrix

Based on interfaces, here's how different objects interact:

| Object Type | Blocks Projectiles | Has Collision | Blocks Flyers |
|-------------|-------------------|---------------|---------------|
| BuildingWalls | ✓ | ✓ | ✓ |
| Asteroids | ✓ | ✓ | ✓ |
| HighObstacle | ✓ | ✓ | ✓ |
| RegularObstacle | ✓ | ✓ | ✗ |
| BarricadeObstacle | ✗ | ✓ | ✗ |
| Shields | ✓ | ✗ | ✗ |
