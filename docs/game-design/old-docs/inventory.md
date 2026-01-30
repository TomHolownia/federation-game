# Inventory System

> Converted from: `Inventory.png`, `Inventory Equipment.jpg`, `Inventory Weapons.jpg`

This document describes the inventory UI and item management system.

## Inventory Layout Overview

![Inventory Layout](images/inventory-layout.png)

The inventory screen is divided into three main sections:

### Left Panel - Inventory
- **Item Grid**: 6 slots visible (expandable)
- **Quest Items**: Separate section with expandable/scrollable list (infinite capacity)

### Center Panel - Currently Equipped
Equipment slots arranged around a body representation:
- **Head** (top center)
- **Primary Weapon** (left)
- **Body** (center)
- **Secondary Weapon** (right)
- **Shield** (bottom left)
- **Ability 1** (bottom center-left)
- **Ability 2** (bottom center-right)

### Right Panel - Shop
- Grid of purchasable items (12 slots shown)
- Available when at shop locations

---

## Equipment Tab

![Equipment Tab](images/inventory-equipment.jpg)

### Tab Navigation
Tabs across the top: Equipment | Weapons | Food | Software | Miscellaneous

### Equipment View Components

#### Picture Area
Large preview of selected item at top of screen.

#### Clothes Section
- Row of item slots for clothing
- Additional rows for shields and batteries:
  - Light blue box → Shield slot
  - Purple box → Battery slot

#### Biomorphs Section
- Three yellow boxes for biomorph items
- Biomorphs provide effects or bonuses

#### Body Representation (Right Side)
Color-coded body slots:
| Color | Body Part |
|-------|-----------|
| Green | Head |
| Red | Chest |
| Blue | Pants |
| Orange | Shoes |
| Brown | Gloves |

#### Item Info Panel
- Scrolling, detailed information appears when item is selected
- Located at bottom of screen

#### Weight Indicator
- Shows current total weight of all items
- Located at bottom right

### Interaction Controls
- **Right-click** an item → Add to body (equip)
- **Left-click** an item → View picture and description

---

## Weapons Tab

![Weapons Tab](images/inventory-weapons.jpg)

### Weapons View Components

#### Picture Area
- Large preview of selected weapon
- Shows weapon name (e.g., "Kinetic Pistol")

#### Weapons Section
- Grid of owned weapons
- Each weapon shows icon/preview

#### Attachments Section
Types of attachments:
- Kinetic Attachment (e.g., Silencer)
- Scope Attachment (e.g., Laser Scope)
- Grip Attachment
- Mounted Attachment

#### Right Sidebar
Quick access to:
- Upgrades
- Ammo
- Kinetic Attachment
- Scope Attachment
- Grip Attachment
- Mounted Attachment

#### Weight System
- Weight increases with more attachments
- Total weight shown at bottom right

### Interaction Controls
- **Left-click** a weapon → Select it, shows in Picture area, info appears on right
- **Right-click** an attachment → Add it to currently selected weapon
- **Double-click** a weapon → Equip it
- **Mouse wheel gesture** → Hold scroll wheel over weapon, point in direction to add to that quick-access slot

---

## Weight System

All items contribute to total weight:
- Heavier loadouts may affect movement speed or stamina
- Attachments add to weapon weight
- Total weight displayed at all times
