# Beings

> Converted from: `BeingMap.png`

This document describes the Being class hierarchy and its relationship with items and equipment.

![Being Map](images/being-map.png)

## Class Hierarchy

### Object (Base)
All game objects inherit from a base Object class.

### Being
The Being class represents all living entities in the game.

**Attributes:**
- `MvtSpeed` - Movement speed
- `Name` - Entity name
- `Health` - Health points

**Currently Equipped Slots:**
- `ShieldGen shield` - Shield generator
- `Weapon primary` - Primary weapon slot
- `Weapon secondary` - Secondary weapon slot
- `Wearing Head` - Head armor/gear
- `Wearing Body` - Body armor/gear
- `Wearing Legs` - Leg armor/gear
- `Wearing Feet` - Foot armor/gear
- `Ability ability1` - First ability slot
- `Ability ability2` - Second ability slot
- `Inventory` - List of items

### Being Types (Races)

| Race | Subtypes | Description |
|------|----------|-------------|
| **Human** | - | Standard human race |
| **Chiascura** | - | (Details in races.md) |
| **Zaggnor** | - | (Details in races.md) |
| **Hellir** | - | (Details in races.md) |
| **Pryzct** | Warrior, Grunt, Queen | Insectoid race with caste system |

## Item System

### Item Base Class

**Attributes:**
- `Name` - Item name

### Item Categories

#### Consumable
- Has `ability` attribute
- Single-use items that grant effects

#### Wearing (Armor/Clothing)
- Equipment worn on body slots
- Head, Body, Legs, Feet

#### Weapon
Two main categories:

**Ranged Weapons:**
- RocketLauncher
- GrenadeLauncher

**Close-Combat Weapons:**
- Plasma
- Kinetic

#### Miscellaneous Item
- General items that don't fit other categories

#### Shield Generator
- Provides shield protection
- Equipped in shield slot

## Relationship: Being ↔ Item

```
Being <--uses--> Item
         (via Inventory)
```

A Being uses Items through their Inventory. Items can be moved from inventory to equipped slots.

## Game Loop Reference

The diagram also shows the basic game loop:

```
while (true) {
    processInput();
    update();
    render();
}
→ Process → Display
```
