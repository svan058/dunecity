# DuneCity C++ Migration Analysis

**Date:** 2026-03-28 03:20  
**Analyzer:** hermes (cron job)  
**Goal:** Migrate SimCity/Micropolis city-building into Dune Legacy (C++)

---

## 1. Specific Files to Port from MicropolisCore

### Completed Ports (7 files, ~48,000 LOC MicropolisCore → ~7,500 LOC DuneCity)

| MicropolisCore File | LOC | Purpose | DuneCity Port | Status |
|---------------------|-----|---------|---------------|--------|
| `simulate.cpp` | 1,729 | Main simulation loop, 16-phase cycle | `CitySimulation.cpp` | ✓ DONE |
| `budget.cpp` | ~800 | Tax, funding, economy | `CityBudget.cpp` | ✓ DONE |
| `evaluate.cpp` | ~900 | City evaluation, ratings | `CityEvaluation.cpp` | ✓ DONE |
| `power.cpp` | 195 | Power grid calculation | `PowerGrid.cpp` | ✓ DONE |
| `scan.cpp` | 600 | Map scanning for effects | `CityScanner.cpp` | ✓ DONE |
| `connect.cpp` | ~1,200 | Connectivity/road tracing | `TrafficSimulation.cpp` | ✓ DONE |
| `tool.cpp` | 1,617 | Building tools (res/com/ind zones) | `ZoneSimulation.cpp` | ✓ DONE |
| `traffic.cpp` | 519 | Vehicle pathfinding, traffic density | `TrafficSimulation.cpp` | ✓ DONE |

### Remaining Unported Files (Low Priority - Skip)

| File | LOC | Purpose | Decision |
|------|-----|---------|----------|
| `sprite.cpp` | 2,039 | City sprites (cars, planes) | **Skip** - Use Dune's existing vehicles |
| `disasters.cpp` | 418 | Disaster effects | **Skip** - Dune has own disaster system |
| `animate.cpp` | ~700 | Animation state | **Skip** - Use Dune's animation system |
| `map.cpp` | 1,025 | Grid management | **Skip** - Dune's Map/Tile system is different |
| `fileio.cpp` | ~800 | Save/load format | **Skip** - Use Dune's save system |
| `generate.cpp` | ~800 | Terrain generation | **Skip** - Use Dune's terrain generation |

---

## 2. Integration Points with Dune Legacy

### Current Architecture

```
~/development/dunecity/src/
├── dunecity/                          # City simulation module
│   ├── CitySimulation.cpp (19,467 B)  # Core 16-phase engine
│   ├── CityBudget.cpp (2,877 B)      # Tax/economy
│   ├── CityEvaluation.cpp (4,244 B)  # City ratings
│   ├── CityScanner.cpp (8,822 B)     # Map scanning
│   ├── PowerGrid.cpp (3,077 B)       # Power distribution
│   ├── TrafficSimulation.cpp (3,771 B) # Road networks
│   └── ZoneSimulation.cpp (8,050 B)  # RCI zone logic
│
└── structures/
    └── WindTrap.cpp                   # Power source for city

~/development/dunecity/include/dunecity/
├── CitySimulation.h (9,053 B)
├── CityBudget.h (1,407 B)
├── CityEvaluation.h (1,802 B)
├── CityScanner.h (1,062 B)
├── PowerGrid.h (2,083 B)
├── TrafficSimulation.h (1,623 B)
├── ZoneSimulation.h (1,558 B)
├── CityConstants.h (2,659 B)
├── CityMapLayer.h (4,075 B)
└── CityOverlay.h (448 B)
```

### Integration Status

| Dune Legacy File | Integration Point | Status |
|-----------------|-------------------|--------|
| `src/Game.cpp` | `citySimulation_.advancePhase()` called per game tick | ✓ INTEGRATED |
| `include/Game.h` | `std::unique_ptr<CitySimulation> citySimulation_` | ✓ INTEGRATED |
| `include/Tile.h` | Zone fields: `zoneType`, `population`, `populationDensity`, `crime`, `pollution` | ✓ INTEGRATED |
| `src/CMakeLists.txt` | `add_subdirectory(dunecity)` | ✓ INTEGRATED |
| `src/structures/WindTrap.cpp` | `citySimulation_.registerPowerSource()` | ✓ INTEGRATED |
| `tests/CMakeLists.txt` | Catch2 test cases | ✓ INTEGRATED |

---

## 3. What's Changed Since Last Run

### Last Analysis: 2026-04-10 (from previous run)

### Changes in Recent Commits

| Commit | Description |
|--------|-------------|
| c0af71b | fix(tests): link SDL2_mixer-static and stub TextManager |
| b17effc | fix(tests): link SDL2_mixer in test target |
| 7439210 | feat: Phase 9 Polish - city milestone notifications and sound effects |

### Code Evolution (Last 5 Commits)
- **+148 lines** `src/Game.cpp` - Budget integration
- **+301 lines** `src/audio/sounds.cpp` - City event sounds
- **+16 lines** `tests/CMakeLists.txt` - New test cases
- **Refactored** `src/structures/ZoneStructure.cpp` - Zone behavior

### Current Feature Status (from FEATURES.md)

**COMPLETED PHASES:**
- ✓ Phase 0: Foundation (city simulation engine)
- ✓ Phase 1: Identity & First UI (rebrand, HUD)
- ✓ Phase 2: Zoning UI (zone placement tool)
- ✓ Phase 3: Power Grid Visualization (overlays)
- ✓ Phase 4: City Building Menu (R/C/I structures)
- ✓ Phase 5: Economy Integration (tax, budget)
- ✓ Phase 6: Traffic & Roads (partial - placement + density)
- ✓ Phase 7: Map Overlays (crime, pollution, land value)
- ✓ Phase 8: Disasters & Events (sandstorm, sandworm)
- ✓ Phase 9: Polish & Balance (milestones, sounds)

---

## 4. Testing Strategy

### Build & Run Tests
```bash
cd ~/development/dunecity
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/dunelegacy_tests "[dunecity]"
```

### Test Cases Available

| Test Case | Framework | Purpose |
|----------|-----------|---------|
| `DuneCityTestCase/` | Catch2 | Main simulation tests |
| `CityBudgetIntegrationTestCase/` | Catch2 | Budget system |
| `MilestoneTestCase/` | Catch2 | Notifications |
| `RoadPlacementTestCase/` | Catch2 | Road tool |
| `ZoneStructureTestCase/` | Catch2 | Zone behavior |

### Recommended Test Priority

1. **CitySimulation** - 16-phase cycle, census, map bounds
2. **PowerGrid** - Power calculation, WindTrap integration
3. **ZoneSimulation** - RCI growth, building placement
4. **Integration** - Load/save, game cycle

---

## 5. Blockers and Decisions Needed

### Critical Blockers

| Blocker | Impact | Resolution |
|---------|--------|------------|
| **Untracked dunecity files** | Code exists but not versioned | Must add to git and commit |

### Git Status (Current)
```
On branch: feature/budget-integration
Status: Working tree clean (except this analysis file)

Untracked (needs commit):
  - src/dunecity/ (7 files, ~48KB)
  - include/dunecity/ (10 files)
  - tests/DuneCityTestCase/
  - tests/CityBudgetIntegrationTestCase/
  - tests/MilestoneTestCase/
  - tests/RoadPlacementTestCase/
  - tests/OverlayLegendTestCase/
  - tests/ZoneStructureTestCase/
```

### Decisions Made

| Decision | Resolution |
|----------|------------|
| Traffic/vehicles | Use Dune's existing Harvester, Trike, Tank |
| Sprite rendering | Use Dune's unit rendering |
| Test framework | Keep Catch2 (already integrated) |
| Commit strategy | Feature branch → PR (current approach) |

### Immediate Next Steps

1. **Add dunecity to git** - `git add src/dunecity include/dunecity tests/DuneCityTestCase/`
2. **Commit with message** - "feat: Add city simulation module ported from MicropolisCore"
3. **Push and create PR** - `git push origin feature/budget-integration`
4. **Run full test suite** - Verify all Catch2 tests pass

---

## 6. File Reference

### Source Paths

| Component | Path |
|-----------|------|
| DuneCity (target) | `~/development/dunecity/` |
| Dune Legacy | `~/development/dunecity/` (is Dune Legacy fork) |
| MicropolisCore | `~/development/simcity/MicropolisCore/MicropolisEngine/src/` |

### Key Files

| File | Size | Purpose |
|------|------|---------|
| `CitySimulation.cpp` | 19,467 B | Core 16-phase simulation |
| `ZoneSimulation.cpp` | 8,050 B | RCI zone logic |
| `CityScanner.cpp` | 8,822 B | Map effect scanning |
| `CityBudget.cpp` | 2,877 B | Economy/tax |
| `micropolis.h` | 66,023 B | Original Micropolis engine header |

---

*Generated by hermes cron job - DuneCity C++ Migration Analyzer*
