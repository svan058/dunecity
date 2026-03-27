# DuneCity C++ Migration Analysis

**Date:** 2026-03-27 18:30  
**Analyzer:** hermes (cron job)  
**Goal:** Migrate SimCity/Micropolis city-building into Dune Legacy (C++)

---

## 1. Specific Files to Port from MicropolisCore

### Completed Ports (8 files, ~8,500 LOC ported)

| File | LOC | Purpose | Status |
|------|-----|---------|--------|
| `simulate.cpp` | 1,729 | Main simulation loop, 16-phase cycle | → `CitySimulation.cpp` ✓ |
| `budget.cpp` | ~800 | Tax, funding, economy | → `CityBudget.cpp` ✓ |
| `evaluate.cpp` | ~900 | City evaluation, ratings | → `CityEvaluation.cpp` ✓ |
| `power.cpp` | 195 | Power grid calculation | → `PowerGrid.cpp` ✓ |
| `scan.cpp` | 600 | Map scanning for effects | → `CityScanner.cpp` ✓ |
| `connect.cpp` | ~1,200 | Connectivity/road tracing | → `TrafficSimulation.cpp` ✓ |
| `tool.cpp` | 1,617 | Building tools (res/com/ind zones) | → `ZoneSimulation.cpp` ✓ |
| `traffic.cpp` | 519 | Vehicle pathfinding, traffic density | → `TrafficSimulation.cpp` ✓ |

### Remaining Unported Files

| File | LOC | Purpose | Priority |
|------|-----|---------|----------|
| `sprite.cpp` | 2,039 | City sprites (cars, planes) | Low - Use Dune's vehicles |
| `disasters.cpp` | 418 | Disaster effects | Low - Dune has own disasters |
| `animate.cpp` | ~700 | Animation state | Low |
| `zone.cpp` | 1,039 | Zone placement logic | Medium - overlaps with tool.cpp |
| `map.cpp` | 1,025 | Grid management | Medium - core but different from Dune's Map |
| `fileio.cpp` | ~500 | Save/load | Covered by Dune's existing serialization |
| `random.cpp` | ~100 | RNG wrapper | Dune uses Random.h |
| `graph.cpp` | ~400 | Graphing/history | Not critical for initial port |
| `message.cpp` | ~300 | City messages | Dune has notification system |
| `generate.cpp` | ~250 | Terrain generation | Dune has its own map gen |

---

## 2. Integration Points with Dune Legacy

### Architecture

```
Dune Legacy src/
├── include/dunecity/              # Headers (~2,200 LOC)
│   ├── CitySimulation.h         ✓ Core orchestration (16-phase)
│   ├── ZoneSimulation.h         ✓ Zone growth
│   ├── TrafficSimulation.h      ✓ Road traffic (FULLY PORTED)
│   ├── PowerGrid.h              ✓ Power grid
│   ├── CityScanner.h            ✓ Map scanning
│   ├── CityBudget.h             ✓ Economy
│   ├── CityEvaluation.h         ✓ Ratings
│   ├── CityConstants.h           ✓ Zone/Power constants
│   └── CityMapLayer.h            ✓ Data layers
├── src/dunecity/                  # Implementation (~5,500 LOC)
│   ├── CitySimulation.cpp       ✓
│   ├── ZoneSimulation.cpp       ✓
│   ├── TrafficSimulation.cpp    ✓ (traffic.cpp ported)
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
| `include/Tile.h` | Zone fields | ✓ INTEGRATED |
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

## 3. What's Changed Since Last Run (2026-04-09)

**NOTE:** Last run used future date (2026-04-09). Today's date is 2026-03-27. The analysis directory shows multiple files from early April 2026 - possible date discrepancy in the system.

### Current State Summary
1. **8/8 core MicropolisCore files ported** - All critical simulation systems complete
2. **Integration verified** - CitySimulation is hooked into Game.cpp game loop
3. **Testing infrastructure exists** - DuneCityTestCase/ uses Catch2
4. **BLOCKER: Git untracked** - All dunecity files remain uncommitted

### No Changes Detected
- Git status shows **no new commits** to dune/dunelegacy
- dunecity module **remains untracked** (same as previous runs)
- All 8 core simulation files **remain ported and integrated**

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

## 5. Blockers and Decisions Needed

### Critical Blockers

| Blocker | Impact | Resolution |
|---------|--------|------------|
| **Untracked dunecity files** | Code not versioned | Commit to dune/dunelegacy repo |

### Decisions Needed

| Decision | Options | Recommendation |
|----------|---------|----------------|
| Traffic/vehicle integration | Port traffic.cpp OR use existing Dune vehicle system | **Use existing Dune vehicles** - Harvester, Trike, Tank already exist |
| Sprite rendering | Port sprite.cpp OR use Dune's unit rendering | **Use Dune's unit rendering** - consistent with game art |
| Commit strategy | Commit dunecity as a branch OR commit directly to main | **Branch + PR** - allows review |
| Test framework | Continue with Catch2 OR migrate to gtest | **Keep Catch2** - already integrated |

### Immediate Next Steps
1. **Commit dunecity code** to Dune Legacy repo (pending user action)
2. **Add more Catch2 test cases** to DuneCityTestCase/ for simulation coverage
3. **Verify WindTrap power** integration works in actual gameplay
4. **Decide on vehicle system** - integrate Dune's existing vehicles or use Micropolis-style traffic density

---

## 6. File Reference

### MicropolisCore Source Files
- Path: `~/development/simcity/MicropolisCore/MicropolisEngine/src/`
- Total: ~18,173 LOC across 22 .cpp files
- Key files: `micropolis.h` (66,023 bytes), `simulate.cpp` (46,487 bytes), `tool.cpp` (47,263 bytes)

### Dune Legacy dunecity Module
- Headers: `~/development/dune/dunelegacy/include/dunecity/`
- Source: `~/development/dune/dunelegacy/src/dunecity/`
- Integration: `~/development/dune/dunelegacy/src/Game.cpp` (citySimulation_)

### Test Infrastructure
- Test case: `~/development/dune/dunelegacy/tests/DuneCityTestCase/DuneCityTestCase.cpp`
- Build: `./build/dunelegacy_tests "[dunecity]"`

---

*Generated by hermes cron job - DuneCity C++ Migration Analyzer*
