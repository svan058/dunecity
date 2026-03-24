# Micropolis (SimCity) — Game & Architecture Summary

> **Source**: `/Users/stefanvanderwel/development/simcity/micropolis`
> **Version**: 5.0 — GNU GPL v3+ (Electronic Arts open-source release for OLPC)

## Overview

Micropolis is the open-source release of Will Wright's original **SimCity Classic** (1989). It was published under the GPL as part of the One Laptop Per Child (OLPC) initiative. The codebase originates from the Unix port of SimCity (~1990) and has been modernized with a C++ engine, Python/GTK frontend, and SWIG bindings.

"Micropolis" is a registered trademark of Micropolis GmbH, licensed for non-commercial use.

---

## Directory Structure

```
micropolis/
├── MicropolisCore/                    # Core simulation engine (modern C++)
│   └── src/
│       ├── MicropolisEngine/          # Main simulation engine (C++)
│       ├── CellEngine/                # Cellular automata for terrain
│       ├── TileEngine/                # Tile rendering engine
│       └── pyMicropolis/              # Python frontend & SWIG bindings
│           └── micropolisEngine/      # Python UI modules (GTK)
│
├── micropolis-activity/               # Tcl/Tk frontend (OLPC Sugar activity)
│   ├── src/
│   │   ├── sim/                       # Legacy C simulation backend
│   │   ├── tcl/, tk/, tclx/          # Tcl/Tk libraries
│   ├── res/                           # Tcl UI scripts & resources
│   ├── images/                        # Game art assets
│   ├── cities/                        # Pre-built city save files
│   └── manual/                        # Game manual
│
├── turbogears/                        # TurboGears web framework integration
└── laszlo/                            # Laszlo framework integration
```

---

## Architecture

### Languages

| Layer | Language | Location |
|-------|----------|----------|
| Simulation engine | C++ | `MicropolisCore/src/MicropolisEngine/` |
| Legacy backend | C | `micropolis-activity/src/sim/` |
| Modern frontend | Python (GTK+) | `MicropolisCore/src/pyMicropolis/` |
| Legacy frontend | Tcl/Tk | `micropolis-activity/res/` |
| Bindings | SWIG | C++ → Python bridge |

### Engine/UI Separation

The architecture cleanly separates simulation from presentation:

- **Backend**: The `Micropolis` class contains all simulation state and logic — pure computation with no UI dependencies.
- **Frontend**: Multiple UI implementations can drive the same engine (Python/GTK, Tcl/Tk, web).
- **Communication**: A `CallbackFunction` system allows the engine to emit events to whichever frontend is attached, keeping the engine language-agnostic.

### World Representation

- **World size**: 120 × 100 tiles (`WORLD_W`, `WORLD_H`)
- **Tile encoding**: 16-bit values with status bit flags
  - Lower 10 bits: tile character (0–1023)
  - Status bits: `PWRBIT`, `CONDBIT`, `BURNBIT`, `BULLBIT`, `ANIMBIT`, `ZONEBIT`
- **Map layers**: Template-based `Map<DATA, BLKSIZE>` system with different resolutions per data type (1×1, 2×2, 4×4, 8×8 clusters)

---

## Game Systems

### Zones

Three zone types, each with distinct growth mechanics:

| Zone | Growth Factors | Side Effects |
|------|----------------|--------------|
| **Residential** | Land value, low pollution, low crime, power, traffic access | Population growth |
| **Commercial** | Distance to city center, traffic access, power | Commerce activity |
| **Industrial** | Power, traffic access | Generates pollution |

Zones are processed during the map scan phase. Each zone tile checks power status, evaluates growth conditions (land value, pollution, crime, traffic), updates population, and triggers traffic generation.

### Power System

- **Sources**: Coal plants (700-tile capacity), Nuclear plants (2000-tile capacity)
- **Distribution**: Flood-fill algorithm from power sources through conductive tiles
- **Tracking**: `powerGridMap` records which tiles are powered
- **Requirement**: Zones must be powered to develop

### Traffic System

- Stack-based pathfinding from zones to destinations (max 30 tiles)
- Traffic density map tracks road/rail usage
- Heavy traffic triggers helicopter dispatch
- Roads and rails both serve as traffic carriers

### Pollution & Environment

- **Sources**: Industrial zones, traffic, fires, power plants, airports, seaports
- **Storage**: `pollutionDensityMap`
- **Impact**: Reduces land value, inhibits residential/commercial growth

### Crime

- **Factors**: Inversely related to land value; increases with population density; decreases near police stations
- **Storage**: `crimeRateMap`
- **Mitigation**: Police station coverage radius

### Land Value

- **Factors**: Distance from city center, development density, pollution (negative), crime (negative)
- **Storage**: `landValueMap`
- **Impact**: Drives zone desirability and growth type

### Budget & Economy

- **Tax collection**: Every 48 simulation time units (~1 game year)
- **Tax rate**: 0–20% (`cityTax`)
- **Expenditure categories**:
  - Roads (based on road/rail count and game difficulty)
  - Police (based on station count)
  - Fire (based on station count)
- **Allocation**: Priority order when funds are limited: roads → fire → police
- **Effect variables**: `roadEffect`, `policeEffect`, `fireEffect` scale service effectiveness

### Disasters

**Random disasters** (probability when triggered):

| Disaster | Chance | Notes |
|----------|--------|-------|
| Fire | 2/9 | |
| Flood | 2/9 | |
| Monster | 2/9 | Requires high pollution |
| Tornado | 1/9 | |
| Earthquake | 1/9 | |
| Nothing | 1/9 | |

**Scenario-specific disasters**:

| Scenario | City | Disaster |
|----------|------|----------|
| San Francisco | — | Earthquake |
| Hamburg | — | Fire bombs |
| Tokyo | — | Monster attack |
| Boston | — | Nuclear meltdown |
| Rio | — | Flooding |
| Dullsville | — | Boredom |
| Bern | — | Traffic problems |
| Detroit | — | Crime |

### Sprites (Moving Entities)

Sprites are the game's mobile objects, managed via a sprite pool with free-list reuse:

| Sprite | Behavior |
|--------|----------|
| Train | Follows rail network |
| Helicopter | Responds to fires and heavy traffic |
| Airplane | Flies between airports |
| Ship | Navigates to seaports |
| Bus | Public transportation |
| Monster | Destroys buildings in polluted areas |
| Tornado | Random-path destruction |
| Explosion | Visual effect |

### Map/Terrain Generation

Procedural generation (`generate.cpp`) with configurable parameters:

- **Rivers**: Curved paths with adjustable curviness (`terrainCurveLevel`)
- **Lakes**: Random placement (`terrainLakeLevel`)
- **Trees/Forests**: Generated with smoothing passes (`terrainTreeLevel`)
- **Islands**: Optional island generation mode (`terrainCreateIsland`)

### City Evaluation

- `cityEvaluation()` computes an overall city score
- `voteProblems()` tallies citizen complaints (crime, taxes, traffic, pollution, etc.)
- `getScore()` produces the final rating
- Dual-scale history tracking: 10-year and 120-year windows for population, money, crime, pollution

### UI / Rendering

**Python/GTK frontend** (`pyMicropolis/micropolisEngine/`):
- `micropolisgtkengine.py` — GTK+ UI engine
- `micropoliswindow.py` — Main application window
- `micropolisdrawingarea.py` — Map rendering
- `micropolistool.py` — Tool handling
- Pie menus for tool selection

**Tcl/Tk frontend** (`micropolis-activity/res/`):
- `micropolis.tcl` — Main UI script
- Window management, tool panels, budget window

### Sound

- Positional sound via `makeSound(channel, sound, x, y)`
- Sounds triggered by game events (disasters, construction, etc.)

---

## Simulation Loop

The simulation runs a **16-phase cycle** that distributes work across frames to avoid spikes:

| Phase | Action |
|-------|--------|
| 0 | Increment `cityTime`, set growth valves, clear census |
| 1–8 | `mapScan()` — process 1/8 of the map per phase (zone updates) |
| 9 | Census (`take10Census`, `take120Census`), tax collection, city evaluation |
| 10 | Power scan (`doPowerScan`) — flood-fill power distribution |
| 11 | Pollution, terrain, land value scan |
| 12 | Crime scan |
| 13 | Population density scan |
| 14 | Fire analysis |
| 15 | Disasters, sprite movement (`moveObjects`) |

### Time System

| Constant | Value | Meaning |
|----------|-------|---------|
| `PASSES_PER_CITYTIME` | 16 | Phases per simulation tick |
| `CITYTIMES_PER_MONTH` | 4 | ~7.6 in-game days each |
| `CITYTIMES_PER_YEAR` | 48 | Full simulation year |

### Growth Valves

Three global valves (`resValve`, `comValve`, `indValve`) control the rate of zone growth, preventing runaway expansion and balancing the three zone types against each other.

---

## Key Source Files

### Core Engine (`MicropolisCore/src/MicropolisEngine/`)

| File | Purpose |
|------|---------|
| `micropolis.h` / `micropolis.cpp` | Main `Micropolis` class — all simulation state (~3000+ lines) |
| `simulate.cpp` | 16-phase simulation loop, valve calculation |
| `zone.cpp` | Zone processing: `doResidential()`, `doCommercial()`, `doIndustrial()` |
| `budget.cpp` | Budget management: `doBudgetNow()`, `collectTax()` |
| `disasters.cpp` | Disaster triggers: `doDisasters()`, `scenarioDisaster()` |
| `traffic.cpp` | Traffic generation and pathfinding: `makeTraffic()`, `tryDrive()` |
| `power.cpp` | Power grid flood-fill: `doPowerScan()` |
| `generate.cpp` | Map generation: `generateMap()` |
| `scan.cpp` | Density scans: pollution, crime, land value, population |
| `sprite.cpp` | Sprite management: `moveObjects()` |
| `evaluate.cpp` | City evaluation: `cityEvaluation()`, `getScore()` |
| `tool.cpp` | Tool application: `doTool()`, building/zone placement |
| `map_type.h` | Template `Map<DATA, BLKSIZE>` class for map layers |
| `data_types.h` | Core types: `Byte`, `Quad`, `UQuad`, `Ptr` |

---

## Notable Design Patterns

### 1. Phase-Based Simulation
Work is spread across 16 phases per tick so no single frame bears the full map scan cost. Different subsystems update at different frequencies.

### 2. Template-Based Map Layers
`Map<DATA, BLKSIZE>` provides type-safe, resolution-appropriate storage for each data layer (byte maps at 1:1, clustered maps at 2:1 through 8:1).

### 3. Flood-Fill Algorithms
Used for both power distribution and traffic pathfinding — stack-based position tracking with bounded search depth.

### 4. Callback-Driven UI Decoupling
The engine fires events through a `CallbackFunction` pointer, allowing any frontend (Python, Tcl, web) to subscribe without engine changes.

### 5. Tile Bit-Flags
The 16-bit tile word packs both identity and state into a compact representation, enabling fast per-tile queries without auxiliary lookups.

### 6. Sprite Pool with Free List
Sprite objects are recycled through a free list to avoid allocation churn during gameplay.

### 7. Tool Effects Batching
The `ToolEffects` class accumulates map modifications so they can be applied (or reverted) as a batch, supporting potential undo/redo.

### 8. Dual-Scale History
Both short-term (10-year) and long-term (120-year) history buffers track city metrics, enabling graphs at different zoom levels.

---

## Build System

### Dependencies
- **Tcl/Tk** — legacy frontend
- **Python** — modern frontend
- **GTK+** — Python UI toolkit
- **SWIG** — C++ ↔ Python bindings

### Build Commands

```bash
make all       # Build MicropolisCore + micropolis-activity
make install   # Install both components
make clean     # Clean build artifacts
```

The MicropolisCore build compiles the C++ engine, generates SWIG Python bindings, and produces shared libraries. The micropolis-activity build compiles the legacy C backend and bundles Tcl/Tk resources.

---

## Relevance to Dune City

Key architectural ideas from Micropolis that may inform Dune City design:

- **Phase-based simulation loop** — distributes workload evenly across frames
- **Layered map system** — separate data layers (terrain, power, traffic, pollution, crime, land value) at different resolutions
- **Tile bit-packing** — compact per-tile state with flag bits
- **Engine/UI separation** — pure simulation logic decoupled from rendering via callbacks
- **Growth valves** — global controls that balance competing zone/resource demands
- **Flood-fill infrastructure** — power and connectivity propagation through networks
- **Budget as lever** — funding levels directly scale service effectiveness
