# DuneCity: Delivery Summary

**Date:** 2026-03-24
**Status:** All 9 phases complete. Build passes.
**Transcript:** [DuneCity Integration](c2579e37-7273-4e47-bd8e-c372a30f6882)

---

## What Was Built

Micropolis (open-source SimCity) city-building simulation has been migrated into the Dune Legacy RTS game engine. Both simulations run in the same game loop — Dune Legacy's fixed-timestep loop processes combat objects every tick, then advances one phase of the 16-phase Micropolis city simulation cycle, distributing city work across 16 Dune game cycles to avoid frame spikes.

**Total new code:** ~2,250 lines across 16 files (7 `.cpp`, 8 `.h`, plus modifications to 7 existing Dune Legacy files).

---

## File Inventory

### New Files (all under `~/development/dune/dunelegacy/`)

| File | Lines | Purpose |
|------|-------|---------|
| `include/dunecity/CityConstants.h` | 47 | Zone types, phase timing, disaster types, power strengths, tax limits |
| `include/dunecity/CityMapLayer.h` | 133 | `CityMapLayer<DATA,BLKSIZE>` template — multi-resolution overlay maps ported from Micropolis `map_type.h` |
| `include/dunecity/CitySimulation.h` | 225 | Main orchestrator — 16-phase scheduler, all state, subsystem ownership |
| `include/dunecity/PowerGrid.h` | 76 | Power flood-fill declarations |
| `include/dunecity/ZoneSimulation.h` | 55 | RCI zone growth/decline declarations |
| `include/dunecity/TrafficSimulation.h` | 59 | Traffic pathfinding declarations |
| `include/dunecity/CityScanner.h` | 45 | Analysis passes (pollution, crime, land value, pop density, fire) |
| `include/dunecity/CityBudget.h` | 54 | Tax collection, budget allocation, service effectiveness |
| `include/dunecity/CityEvaluation.h` | 66 | City scoring, population history, growth valves |
| `src/dunecity/CitySimulation.cpp` | 518 | 16-phase loop, disaster implementations, command handling, save/load |
| `src/dunecity/PowerGrid.cpp` | 113 | Flood-fill power distribution from WindTrap seeds |
| `src/dunecity/ZoneSimulation.cpp` | 239 | `doZone()`, `doResidential/Commercial/Industrial()`, growth/decline |
| `src/dunecity/TrafficSimulation.cpp` | 134 | `makeTraffic()`, stack-based DFS pathfinding, density tracking |
| `src/dunecity/CityScanner.cpp` | 264 | Pollution, crime, land value, population density, fire analysis scans |
| `src/dunecity/CityBudget.cpp` | 87 | `collectTax()`, `updateFundEffects()` |
| `src/dunecity/CityEvaluation.cpp` | 132 | `setValves()`, `cityEvaluation()`, `take10Census()`, scoring |

### Modified Dune Legacy Files

| File | Changes |
|------|---------|
| `include/Game.h` | Added `citySimulation_` unique_ptr, `citySimEnabled_` flag, `getCitySimulation()` accessor |
| `src/Game.cpp` | Hooked `advancePhase()` into `updateGameState()`, city sim init in `initializeGameLoop()`, save/load integration (version 9807+), economic victory check |
| `include/Map.h` | Included `CityMapLayer.h` |
| `include/Tile.h` | Added `cityZoneType_`, `cityZoneDensity_`, `cityTileId_`, `cityPowered_`, `cityConductive_` members with accessors |
| `src/Tile.cpp` | Save/load for city tile fields (version-gated to 9807+) |
| `include/Command.h` | Added `CMD_CITY_PLACE_ZONE`, `CMD_CITY_SET_TAX_RATE`, `CMD_CITY_SET_BUDGET`, `CMD_CITY_TOOL` to `CMDTYPE` enum |
| `src/Command.cpp` | Dispatch city commands to `CitySimulation::executeCityCommand()` |
| `src/structures/WindTrap.cpp` | Registers with city power grid via `registerPowerSource()` |
| `include/Definitions.h` | Bumped `SAVEGAMEVERSION` to 9807, added `WINLOSEFLAGS_ECONOMIC` (0x10) |
| `include/players/QuantBot.h` | Added `manageCityBuilding()` method, `cityBuildTimer` member |
| `src/players/QuantBot.cpp` | AI city-building logic — places zones and roads near base |
| `src/CMakeLists.txt` | Added all 7 `dunecity/*.cpp` files to build |

---

## Architecture

### Game Loop Integration

```
Game::updateGameState()
  ├── cmdManager.executeCommands()    // includes city commands
  ├── house[i]->update()              // QuantBot::manageCityBuilding() runs here
  ├── processObjects()                // WindTrap registers power sources here
  ├── citySimulation_->advancePhase() // one of 16 phases per tick
  └── economic victory check
```

### 16-Phase City Simulation Cycle

| Phase | Action |
|-------|--------|
| 0 | Increment cityTime, clear census, reset power flags |
| 1–8 | mapScan — process 1/8 of the map per phase (zone growth, road counting) |
| 9 | Census, tax collection, growth valve calculation |
| 10 | Decay rate-of-growth map, decay traffic density |
| 11 | Power flood-fill scan from WindTrap sources |
| 12 | Pollution, terrain, land value analysis |
| 13 | Crime scan |
| 14 | Population density scan |
| 15 | Fire analysis, Arrakis disasters |

### Subsystem Decomposition

The monolithic Micropolis class was decomposed into 6 focused modules, each holding a `CitySimulation*` back-pointer:

```
CitySimulation (orchestrator)
  ├── PowerGrid         — flood-fill from WindTrap seeds
  ├── ZoneSimulation    — RCI zone growth/decline based on desirability
  ├── TrafficSimulation — stack-based DFS pathfinding, density tracking
  ├── CityScanner       — 5 analysis passes with smoothing
  ├── CityBudget        — tax collection, service fund allocation
  └── CityEvaluation    — city scoring, valve adjustment, census history
```

### Determinism

- Dedicated `Random cityRng_` seeded from `currentGame->randomGen.getSeed() ^ 0xC17DEAD0`
- All city actions routed through `CommandManager` via 4 command types
- No floating-point math in simulation state
- RNG seed persisted in save files for reproducibility

### Data Model

**Per-tile fields** (added to `Tile`):
- `cityZoneType_` — None / Residential / Commercial / Industrial
- `cityZoneDensity_` — 0–255 density level
- `cityTileId_` — Micropolis-style tile ID
- `cityPowered_` / `cityConductive_` — power grid connectivity

**Overlay layers** (owned by `CitySimulation`, multi-resolution via `CityMapLayer<DATA,BLKSIZE>`):
- `powerGridMap_` (1:1), `trafficDensityMap_` (2:1), `pollutionDensityMap_` (2:1)
- `landValueMap_` (2:1), `crimeRateMap_` (2:1), `populationDensityMap_` (2:1)
- `terrainDensityMap_` (4:1), `rateOfGrowthMap_` (8:1), `comRateMap_` (8:1)
- `fireStationMap_` / `policeStationMap_` and their effect maps (8:1)

---

## Porting Approach per Micropolis Source

| Micropolis File | LOC | Ported To | Key Adaptations |
|-----------------|-----|-----------|-----------------|
| `simulate.cpp` | ~1,600 | `CitySimulation.cpp` | Phase scheduler preserved; `simRobot()` sprite system dropped |
| `power.cpp` | 243 | `PowerGrid.cpp` | `doPowerScan()` flood-fill preserved; Micropolis power plants replaced by Dune `WindTrap` |
| `zone.cpp` | 1,039 | `ZoneSimulation.cpp` | Growth/decline logic preserved; tile IDs simplified; desirability checks use city overlay maps |
| `traffic.cpp` | 570 | `TrafficSimulation.cpp` | Stack-based DFS preserved; `tryGo()` uses deterministic direction selection |
| `scan.cpp` | 600 | `CityScanner.cpp` | All 5 analysis passes preserved; smoothing algorithm preserved |
| `budget.cpp` | 380 | `CityBudget.cpp` | Tax/budget logic preserved; funds feed into Dune economy |
| `evaluate.cpp` | 600+ | `CityEvaluation.cpp` | Census, scoring, valve calculation preserved; city class thresholds kept |
| `disasters.cpp` | 430 | `CitySimulation.cpp` | Fire→Sabotage, Flood→(removed), Earthquake/Tornado→Sandstorm, Monster→Sandworm, new SpiceBlow |
| `map_type.h` | 200 | `CityMapLayer.h` | `Map<DATA,BLKSIZE>` template preserved as `CityMapLayer<DATA,BLKSIZE>`, added save/load |

---

## Backward Compatibility

- `SAVEGAMEVERSION` bumped from 9806 to 9807
- Old saves (< 9807) load normally — city sim fields are skipped, `citySimulation_` is initialized fresh
- City tile fields in `Tile::load()` are version-gated
- Feature flag `citySimEnabled_` allows disabling the entire city layer

---

## AI Integration

`QuantBot::manageCityBuilding()` runs every 500 game cycles per AI player:
- Scans tiles within 15-tile radius of base center
- Places up to 4 zones per round (60% residential, 20% commercial, 20% industrial)
- Places up to 6 road segments connecting zones
- All actions go through `CommandManager` for multiplayer safety

---

## Victory Conditions

New `WINLOSEFLAGS_ECONOMIC` (0x10) flag. When enabled:
- Game is won when `citySimulation_->getTotalPop() >= economicVictoryThreshold_`
- Default threshold: 500 population
- Checked every tick in `Game::updateGameState()`

---

## Known Limitations / Future Work

1. **No rendering** — City zones, overlays, and density are tracked in data but have no visual representation yet. Tile sprites, overlay heatmaps, and zone UI panels still need to be wired to Dune Legacy's SDL2 rendering pipeline.
2. **No city UI** — Tax rate, budget allocation, and zone placement tools exist as commands but have no GUI. Need toolbar integration and info panels.
3. **AI city-building is basic** — Deterministic grid-walk pattern, no strategic optimization for desirability or growth potential.
4. **No tests** — CppUnit test files for DuneCity subsystems have not been created yet. The test directory `tests/DuneCityTestCase/` does not exist.
5. **Simulation fidelity is ~25%** — Line counts are lower than Micropolis originals because edge cases, sprite animation, multi-tile zone structures, and some statistical smoothing passes were simplified.
6. **PoliceStation / FireStation not connected** — These Dune structures don't yet register with `CitySimulation` (unlike WindTrap which does).
7. **Multiplayer lobby** — City info (population, score) not yet shown in the lobby screen.
8. **No desert city terrain generation** — Planned Arrakis-themed zone names and terrain generation not implemented.

---

## Build

```bash
cd ~/development/dune/dunelegacy/build
cmake --build . --target dunelegacy
# Compiles cleanly. Only warnings are pre-existing vcpkg macOS version mismatches.
```

---

## Repositories

- **DuneCity:** `~/development/dunecity/` — the canonical repo (git, `main` branch). Contains the full Dune Legacy engine with all DuneCity modifications.
- **Dune Legacy (upstream):** `~/development/dune/dunelegacy/` — original fork, remote `ssh://sfgit/p/dunelegacy/code`. DuneCity changes exist as uncommitted modifications here too but the dunecity repo is the source of truth.
- **Micropolis (reference):** `~/development/simcity/MicropolisCore/MicropolisEngine/src/` — C++, original SimCity algorithms used as porting source.
- **Analysis docs:** `~/development/dunecity/analysis/` — architecture comparisons, porting notes, this file.
- **Integration plan:** `~/.cursor/plans/dunecity_integration_plan_c90603b0.plan.md` — full 9-phase plan with file references.
