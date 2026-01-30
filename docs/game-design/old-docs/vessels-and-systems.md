# Vessels, Properties and Systems

> Source: `Vessels, Properties and Systems.docx`

This document describes ships, vehicles, properties, and their component systems.

See also: [Object Hierarchy](object-hierarchy.md) for technical class structure.

---

## Ship Classes

### Capital Class Ships

The largest ships in the game.

**Capabilities:**
- Hold an impressive variety of systems
- Powerful hyperdrive
- Huge hangar(s)
- Big shields
- Large crew cabins
- **Cannot** enter a planet's orbit
- Customizable design

**Advanced:**
If you advance far enough in the game, you can make your own super ship capable of housing hundreds of people - maybe even your own portable city.

---

### Battleship Class Ships

Smaller, more nimble ships, but still rather large.

**Capabilities:**
- Often have a hangar for smaller vessels
- Capable of entering orbit and landing (on terrain or at spaceports)
- Usually have a hyperdrive
- Customizable design

---

### Fighter Class Ships

Often stored in hangars on battleships and capital vessels.

**Capabilities:**
- Can have a hyperdrive for long missions
- Only have a cockpit (no crew quarters)
- Systems included in the design
- Color customizable and upgradeable
- Cannot move systems around

---

## Atmospheric Vehicles

### Hover Car

Literally what they sound like.

**Capabilities:**
- Thrusters to get around cities and planets
- Cannot leave the atmosphere
- Can float ~50-75m off the ground (variable by type)

**Variant:** HoverCopter

---

### Hover Bike

Same as hover car but in bike form.

---

## Land Vehicles

### Cars

Literally cars with lots of different types. Can have weapons and gadgets.

---

### Motorcycles

Just a motorcycle.

---

### Large Land Vehicles

Highly customizable. Examples:
- Artillery tank
- Regular tank
- Troop carrier tank
- High speed armoured vehicle for breaking into compounds
- Mining truck

**Special:**
Can be fitted with underwater exploratory equipment (made submersible).

---

### Submersible Vehicles

Slightly customizable, for exploring depths of ocean.

**Note:** Battleships and Fighters can usually explore underwater/lava/whatever, but not as effectively as a submersible.

---

## Vehicle Hierarchy

What can fit inside what:

### Capital Vessel
Can house any other kind of ship, including Battleships (but not other Capital Vessels).

**Note:** Since Capital Vessels cannot enter atmosphere, having land vehicles inside is somewhat pointless unless the ship is so big you need them to get around.

### Battleships
Can hold anything other than Capital Class and other Battleships.

**Practical Uses:**
- Take tanks, submersibles, land vehicles into atmosphere
- House fighters for crew combat or errands/scouting

### Large Land Vehicles
Customizable, can fit in Battleships and Capital Ships.

**Can Hold:**
- Fighters
- All land vehicles (even other large land vehicles if small enough)
- Atmospheric flyers

---

## Garages

All vehicles and ships can be modified at intergalactic garages.

### Space Garages (for Inter-solar Vessels)
Located near space stations.

**Battleships & Capital Vessels:**
- Highly customizable
- Move system rooms and rooms around
- Add filler blocks, corridors, stairwells
- Change colors of various parts

**Fighters:**
- Color customizable
- Upgradeable
- Cannot move anything around (essentially cockpits with wings)

### Planetary Garages (for Atmospheric Flyers & Land Vehicles)
Located on planet surfaces.

**Modifications:**
- Color and upgrades only (except Large Land Vehicles)

**Large Land Vehicles:**
- Highly customizable
- Move systems and rooms around
- Change colors

### How to Edit

Drive your vehicle into a garage. You can edit vehicles you don't own.

**Costs:**
- Every rearrangement costs money
- New systems, upgrades, and paint jobs also cost money

**Base Structures:**
Ships are built around a set "base":
- Land vehicle: tracks/wheels and flat cover
- Capital class/Battleship: built around engines and hyperdrive

---

## Repairs

Sometimes your ship will be damaged beyond your ability to fix (this ability can be upgraded). You'll need to bring your ship to a garage.

---

## Owning Vessels

### Purchasing
- Bought from vendors all over the galaxy
- Immediate access to drive
- Protected against de-spawning
- Has a locator beacon

### Summoning
You can call your vehicle to fly/drive to your position at any time.

**Restrictions:**
- Capital ships hold position in orbit
- Battleships and fighters must dock at the nearest station if you're in a city (incurs docking fees warning!)
- Can only call vehicles in local orbit or nearby on the planet

### Claiming Unowned Vehicles

**Small Vehicles:**
Capture by driving into a hangar on one of your ships or properties.

**Large Vehicles (Battleship, Capital Ship, Land Vehicle):**
Capture if you're standing on it and no more enemies remain on board.

---

## Ships Inside Ships

Store ships and vehicles inside other ships via the Hangar system.

### Storage Capacities

**Large Slot Contents:**
| 1 Large Slot = |
|----------------|
| 1 Battleship |
| 1 Large Land Vehicle |
| 4 Fighters, Hover Cars |
| 8 Cars or Submersibles |
| 16 Hover Bikes or Motorcycles |
| 8 Suit Systems |
| 32 Units of Cargo |

**Small Slot Contents:**
| 1 Small Slot = |
|----------------|
| 1 Fighter or Hover Car |
| 2 Cars or Submersibles |
| 4 Hover Bikes or Motorcycles |
| 2 Suit Systems |
| 8 Units of Cargo |

---

## Ship Systems

Systems are housed in specific rooms of a vessel or property.

### Hangars

Can be put in: Capital Vessels, Battleships, Large Land Vehicles, Properties

**Description:**
A large box of varying ratios with one side protected by a forcefield instead of a wall. Atmospherically sealed (can even go underwater).

**Controls:**
- Owner can manually open/close metal doors covering the forcefield (can be done remotely)
- Hangar has a ramp that can be manually engaged (not on Capital Vessels or properties)

---

### Cargo Hold

A must-have for miners and looters.

**Capacity:** 32-128 units of cargo (size varies accordingly)

**Note:** You can also store stuff in your hangar.

---

### Crew Quarters

Houses your crew for sleeping or quiet moments.

**Required:** On any vessel with a hyperdrive.

Comes in many shapes and sizes.

---

### Medical Unit

Heals crew automatically when badly wounded.

---

### Life Support

Provides oxygen, thermal protection, and toxin protection.

**Required:** On any vessel not operating on oxygen-rich planets and moderate climates.

---

### Shields

Protection from:
- Missiles, Lasers
- Nuclear explosions
- Storms, Asteroids
- General space wear
- Inter-solar radiation

**Required:** Every Fighter, Battleship, and Capital Vessel.

---

### Large Engines and Coolant Room

Big engines on the back of your capital ship with an attached coolant room.

**If damaged:** Unable to fly.

---

### Small Engines and Coolant Room

For battleship class vessels.

---

### Hyperdrive

Allows FTL travel by warping space in front and expanding it behind.

**Components:**
- Central unit
- Two external mechanisms

All must be in place to warp. Can function without engines (if undamaged and no warp interference).

---

### AI Core Room

Houses Artificial Intelligence systems for your personal AI during space combat.

**If damaged:** Lose access to some systems and AI functionality.

---

### Navigation Core Room

Controls warp navigation and inertial thrusting for space and atmospheric maneuvering.

---

### Cockpit/Bridge

Where all the action happens.

- **Cockpit:** Few people strapped in
- **Bridge:** Large room with chairs for accessing systems

---

### Weapons Systems

Placeable weapons on the ship. Usually mounted turrets (sometimes fixed position). Some weapons have entire rooms (e.g., Nuclear Missile Launcher).

---

### Weapons Control

Advanced control over weapons.

**If damaged:** AI can take over with limited functionality.

---

### Drone Control

Controls drones aboard your ship.

**If damaged:** AI can take over with limited functionality.

---

### Electronic Warfare Control

Attack and defense of electronic warfare programs (hacking).

**If damaged:** AI can take over with limited functionality.

---

### Generator

Powers everything. **Absolutely essential.**

**Properties:**
- Very tough
- Usually has own shield generator
- Protect at all costs

**Fuel Management:**
AI can automatically transfer fuel from cargo bay, hangar, or storage room.

**Power Modes:**
Can run ship in low power mode to cut energy costs.

**Combat:**
Battles in space are fought largely against each other's generators. If you can wear the enemy's power down, you can destroy them easily.

---

### Scanners/Radar

Various types:
- Thermal detection
- Warp rift detection
- Power surge detection
- Electronic detection
- Radiation detection

Mounted on the outside of the ship.

---

### Testing Range/Practice Arena

Test weapons, gadgets, whatever against various targets and small landscapes. Can spawn fake enemies.

---

### Storage Rooms

Includes storage for:
- Drones
- Weapons
- 4-16 units of cargo
- Anything else

---

### Gravity Generator

Creates gravity on the ship. A luxury that makes getting around easier.

---

### Elevator

For luxurious capital class vessels and properties.

---

## Building Blocks

Not really systems but important for ship structure.

### Corridors/Stairs

Provide connections around a ship or property.

---

### General Room

Comes in varying sizes, can be placed anywhere.

---

### Aesthetic Rooms and Furniture

Anything else that spices up the ship.

---

## Properties

You can buy properties on:
- Space stations
- In cities
- In the wild (pay government for land, build anywhere)
- Your own space station

Properties can house the same systems as vessels.

---

## Vehicle Weapons Systems

*[Details to be added - similar mounting systems to personal weapons but scaled for vehicles]*
