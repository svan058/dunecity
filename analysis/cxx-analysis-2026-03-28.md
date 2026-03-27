# DuneCity C++ Migration Analysis

**Date:** 2026-03-28 00:13  
**Analyzer:** hermes (cron job)  
**Goal:** Migrate SimCity/Micropolis city-building into Dune Legacy (C++)

---

## 1. Specific Files to Port from MicropolisCore

### Completed Ports (8 files, ~2,247 LOC in dunecity module)

| MicropolisCore File | LOC | Purpose | DuneCity Port |
|---------------------|-----|---------|---------------|
| `simulate.cpp` | 1,729 | Main simulation loop, 16-phase cycle | `CitySimulation.cpp` ✓ |
| `budget.cpp` | ~800 | Tax, funding, economy | `CityBudget.cpp` ✓ |
| `evaluate.cpp` | ~900 | City evaluation, ratings | `CityEvaluation.cpp` ✓ |
| `power.cpp` | 195 | Power grid calculation | `PowerGrid.cpp` ✓ |
| `scan.cpp` | 600 | Map scanning for effects | `CityScanner.cpp` ✓ |
| `connect.cpp` | ~1,200 | Connectivity/road tracing | `TrafficSimulation.cpp` ✓ |
| `tool.cpp` | 1,617 | Building tools (res/com/ind zones) | `ZoneSimulation.cpp` ✓ |
| `traffic.cpp` | 519 | Vehicle pathfinding, traffic density | `TrafficSimulation.cpp` ✓ |

### Remaining Unported Files (Low Priority)

| File | LOC | Purpose | Priority |
|------|-----|---------|----------|
| `sprite.cpp` | 2,039 | City sprites (cars, planes) | Low - Use Dune's vehicles |
| `disasters.cpp` | 418 | Disaster effects | Low - Dune has own disasters |
| `animate.cpp` | ~700 | Animation state | Low |
| `map.cpp` | 1,025 | Grid management | Medium - core but different from Dune's Map |
| `fileio.cpp` | ~800 | Save/load format | Skip - Use Dune's save system |
| `generate.cpp` | ~800 | Terrain generation | Skip - Use Dune's terrain |

---

## 2. Integration Points with Dune Legacy

### Architecture

```
DuneLegacy src/
├── include/dunecity/              # Headers (~887 LOC)
│   ├── CitySimulation.h         ✓ Core orchestration (16-phase)
│   ├── ZoneSimulation.h         ✓ Zone growth
│   ├── TrafficSimulation.h      ✓ Road traffic
│   ├── PowerGrid.h              ✓ Power grid
│   ├── CityScanner.h            ✓ Map scanning
│   ├── CityBudget.h             ✓ Economy
│   ├── CityEvaluation.h         ✓ Ratings
│   ├── CityConstants.h           ✓ Zone/Power constants
│   └── CityMapLayer.h            ✓ Data layers
├── src/dunecity/                  # Implementation (~1,360 LOC)
│   ├── CitySimulation.cpp       ✓
│   ├── ZoneSimulation.cpp       ✓
│   ├── TrafficSimulation.cpp    ✓
│   ├── PowerGrid.cpp            ✓
│   ├── CityScanner.cpp          ✓
│   ├── CityBudget.cpp           ✓
│   └── CityEvaluation.cpp       ✓
└── src/structures/
    └── WindTrap.cpp             ✓ Power source
```

### Integration Status

| Dune Legacy File | Integration | Status |
|------------------|-------------|--------|
| `src/Game.cpp` | CitySimulation init/advancePhase/load/save | ✓ INTEGRATED |
| `include/Game.h` | citySimulation_ member | ✓ INTEGRATED |
| `include/Tile.h` | Zone fields (zoneType, population, etc.) | ✓ INTEGRATED |
| `src/CMakeLists.txt` | dunecity module | ✓ INTEGRATED |
| `src/structures/WindTrap.cpp` | registerPowerSource() | ✓ INTEGRATED |
| `tests/CMakeLists.txt` | DuneCityTestCase Catch2 | ✓ INTEGRATED |

### Git Status
```
M include/Command.h
 M include/Definitions.h
 M include/Game.h
 M include/Map.h
 M include/Tile.h
 M include/players/QuantBot.h
 M src/CMakeLists.txt
 M src/Command.cpp
 M src/Game.cpp
 M src/Tile.cpp
 M src/players/QuantBot.cpp
 M src/structures/WindTrap.cpp
 M tests/CMakeLists.txt
 M vcpkg.json
?? AI_BOTS_GUIDE.md
?? build_test/
?? include/dunecity/    # UNTRACKED - 4+ weeks
?? src/dunecity/        # UNTRACKED - 4+ weeks
?? tests/DuneCityTestCase/  # UNTRACKED
```
**BLOCKER: All dunecity code remains uncommitted to the Dune Legacy repo.**

---

## 3. What's Changed Since Last Run (2026-03-28 00:12)

- **No code changes detected** - Static analysis, same git state
- **Dunecity module remains untracked** - 4+ weeks outstanding
- All 8 core simulation files remain ported and integrated

### Current Status Summary
1. **8/8 core MicropolisCore files ported** - All critical simulation systems complete
2. **Integration verified** - CitySimulation hooked into Game.cpp game loop
3. **Testing infrastructure exists** - DuneCityTestCase uses Catch2
4. **BLOCKER: Git untracked** - All dunecity files remain uncommitted

---

## 4. Testing Strategy (Catch2 via CMake)

### Dune Legacy Test Setup
```bash
cd ~/development/dune/dunelegacy
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/dunelegacy_tests "[dunecity]"
```

### Existing DuneCity Test Cases
- `tests/DuneCityTestCase/DuneCityTestCase.cpp` - Uses **Catch2** (not gtest)
- Tests ZoneType, DisasterType, CityConstants validation
- Run with: `./dunelegacy_tests "[dunecity]"`

### Recommended Test Plan (Prioritized)
1. **CitySimulation unit tests** (HIGH PRIORITY)
   - Test 16-phase cycle ordering
   - Test map scanning bounds
   - Test census accumulation

2. **PowerGrid unit tests** (HIGH PRIORITY)
   - Test power calculation
   - Test power source registration from WindTrap

3. **ZoneSimulation unit tests** (MEDIUM)
   - Test zone growth from RCI
   - Test building placement validation

4. **Integration tests** (MEDIUM)
   - Load/save roundtrip
   - Game cycle integration

---

## 5. Blockers & Decisions Needed

### Blockers

| Blocker | Severity | Status |
|---------|----------|--------|
| **All dunecity files uncommitted** | CRITICAL | UNCHANGED - 4+ weeks |
| Dune Legacy build system changes | LOW | Complete |
| No runtime verification of simulation | MEDIUM | Needs manual testing |

### Decisions Required

| Decision | Options | Recommended |
|----------|---------|-------------|
| Git commit strategy | a) Commit all, b) Staged | a) Commit all at once |
| Runtime testing | Manual playtest vs automated | Automated tests first |
| Module organization | Keep in dunelegacy vs separate repo | Keep in dunelegacy |

---

## Summary

- **8/8 MicropolisCore simulation systems ported**
- **Integration points complete** - Game.cpp, Tile.h, WindTrap.cpp
- **Testing infrastructure ready** - Catch2 tests exist
- **CRITICAL BLOCKER: Git commit required** - All dunecity code untracked for 4+ weeks

**Next actionable step:** Run `git add` on dunecity files and commit to enable CI testing.
