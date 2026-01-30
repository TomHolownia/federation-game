# Federation Internal Workings

> Source: `Federation Internal Workings.docx`

This document describes how the Federation operates, including game mechanics and world systems.

---

## Development Checklist

1. Make universe (planets, cities, asteroid fields, space stations)
2. Make story lines (4 generic, 5 race specific but all interweave slightly)
3. Make item list (items, vehicles)
4. Make the five races
5. Make a combat system - when shields are up, it makes no difference where you are hit
6. Make a vehicle/traveling system

---

## Game Overview

### Core Concept
Futuristic FPS free roam. Make sure all/most buildings are accessible. Flying cars, and interstellar travel.

### Key Features

#### Customizable Ships
- Buy a basic chassis from a list of twenty
- Add extra upgrades and equipment

#### Customizable Self
- Choose from three aliens, a human, or a robot of the RH-Union
- Edit your 'look'

#### No Loading Screens
You are always looking from your view (can go 3rd person too). When you go to other planets you have to wait that time inside your ship. Options:
- Go into cryogenic sleep and pass the time instantly (fadeout)
- Go to bed and fadeout
- If you die you will fadeout and wake up in a new body

#### Space Fights
- Lots of missiles, force fields, rail-guns, atom lasers and neutron lasers
- Ships can have parts blown off them
- Life support can go down
- Space fights will probably be 3rd person (from the ship's perspective)

---

## World Scale

### Planet Distribution
- **10 major planets** with three 20km² cities = 600km²
- **10 minor planets** with one 20km² city = 200km²
- **30 wild planets** with bonus 5km² area (small towns, settlements) = 150km²
- **All planets** have 10+ 1km² landing zones = 500km²
  - Refueling zones
  - Alien temples
  - Small settlements
  - Crash sites
  - Strange signals

### Space Structures
Satellites, space stations and ships float in orbit above worlds.

### Procedural Generation
Planets have cities, sites, and special things, but are largely procedurally generated. Different planet sizes generate different gravity and are obviously bigger.

### Atmosphere Physics
- Ships cannot travel as fast in an atmosphere
- Distance to leave atmosphere should be somewhat realistic (scaled down)
- Render distance will have to be reasonably large

---

## Combat System

FPS combat is heavily involved with:
- Cover
- Ion guns
- Forcefield armor

### Graphics Note
Not amazing graphics - more like Minecraft graphics, but this will make the giant areas easy to load.

---

## Economy

### Currency
Everyone has Federation credits, but a few other currencies are available and used on some planets. Exchange rates are part of the game.

### Dynamic Events
Natural disasters and planetary invasions affect:
- Shares and exchange rates
- Civilian behavior
- If a planetary invasion is not stopped, the entire world gets overrun by the invaders

### Property
You can buy and sell property:
- Store items
- Decorate

### Jobs
Get an interesting job - act as complex/generated quests to earn money. Range from:
- Garage worker
- Police officer
- Assassin

---

## Saving System

### Checkpoint System
Checkpoints occur when you:
- Sleep
- Come out of warp
- Enter or near a planet, asteroid, space station or any other place
- Leave or enter your ship

If you die, you respawn at the checkpoint.

### Save and Exit
- Saves exactly where you are and exits the game
- Also saves the previous checkpoint
- If you re-enter the game, you start exactly where you left off
- If you then die, you go back to your checkpoint

---

## Gameplay Features

### Exploration
Explore massive worlds and find unique items, systems, software and weapons.

### Military
Join the army - join the galactic struggle against the Pryzct. Get dropped into alien worlds to fight it out. These will be randomly generated - fighting an alien species similar to the Zurg, in a Star Wars Battlefront style assault.

### Destructible Environment
Structures in cities have a self-repair function which repairs them (but not extremely quickly). On alien worlds and smaller worlds with few villages, the terrain is completely destructible.

### Cities
See the city lights, shops, cafes and bars! Cities will be brightly lit with:
- Neon advertisements
- Huge screens
- Tons of laser lights
- Stylish bars with cool music, lava lamps and incredibly strange drinks
- Large multi-level department stores with lots of shelves (perfect for cover in shootouts)
- Food courts, alleys, pubs
- Sports stadiums and arenas

### Sports
In the future with genetic modification, individual sports have become impossible to rule - only team sports remain:
- Soccer
- Basketball
- Cricket
- **HackFist** (on backwater worlds) - a brutal team gladiatorial competition where two teams fight to the death with specific rules

### Survival
- Hunger and thirst meter (unless you are a robot)
- If it reaches zero, bad things happen
- Robots have inbuilt solar cells which prevent this
- You can also take narcotic and performance enhancing drugs

---

## Civilian System

### Population Distribution
Worlds have percentages of how many humans and aliens are on them, dictating spawn rates. There are also lots of other planetary stats.

### Class System
- Upper
- Middle
- Lower
- Homeless
- Police
- Army

Each class divided by race, dependent on planet/city/region.

### Example Distribution
- Upper Zaggnor make up 36% of population on ____
- Human police make up 0.7% of population on ____

### Behavioral Attributes
Each class has unique behaviors:
- Police travel in pairs 95% of the time
- Middle children travel with an adult 98% of the time
- Homeless children travel with an adult 15% of the time
- Lower worker humans will stand in line for food if near a food stall

---

## Immortality System

### Re-Life Clinics
Everyone receives cheap nano-injections which allow brain cells to retain information without error.

**Process:**
1. Clone bodies are grown in vats (~8-9 months to complete)
2. Old brain extracted and implanted into clone
3. Few weeks of coma while brain reacts to new body
4. ~3 months training of new body
5. Abnormalities ironed out (twitches, blackouts, shakes, vertigo, seizures, rarely death)
6. Client free to return to normal life

**Total time:** About 4 months for a re-life.

### Biotic Treatment
Available since 2195 - a death-free, side-effect-free way to live forever. Promotes health and quality of life as the nano bots fight disease and cancers.

Only available to the wealthiest 0.01% of the Federation.
