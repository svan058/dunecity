# DuneCity C++ Architecture Analysis
**Date:** 2026-03-28 (Updated: 2026-03-28 03:00)
**Sources:** Dune Legacy (`~/development/dune/dunelegacy/`), MicropolisCore (`~/development/simcity/MicropolisCore/`)

---

## Executive Summary

Dune Legacy contains a **partial reimplementation** of Micropolis-style city simulation in `src/dunecity/`. Core modules are implemented but remain **untracked in git** (3+ weeks). Migration is 60-70% complete - focus should shift to committing existing code and filling remaining gaps.

---

## 1. DuneCity Existing Implementation

### Files Present (7 .cpp + 8 .h):

| DuneLegacy File | Status | LOC | MicropolisCore Equivalent |
|-----------------|--------|-----|---------------------------|
| `CitySimulation.cpp` | **Complete** | ~500 | `simulate.cpp`, `update.cpp` |
| `ZoneSimulation.cpp` | **Complete** | ~400 | `zone.cpp`, `allocate.cpp` |
| `TrafficSimulation.cpp` | **Complete** | ~350 | `traffic.cpp` |
| `PowerGrid.cpp` | **Complete** | ~150 | `power.cpp` |
| `CityBudget.cpp` | **Complete** | ~200 | `budget.cpp` |
| `CityEvaluation.cpp` | **Complete** | ~250 | `evaluate.cpp` |
| `CityScanner.cpp` | **Complete** | ~200 | `scan.cpp` |

**Total DuneCity LOC:** ~2,050

**Headers (8 files):**
- CitySimulation.h, ZoneSimulation.h, TrafficSimulation.h, PowerGrid.h
- CityBudget.h, CityEvaluation.h, CityScanner.h, CityConstants.h

---

## 2. MicropolisCore Files to Port (Gap Analysis)

### HIGH PRIORITY - Missing Core Features:

| File | LOC | Purpose | DuneCity Gap |
|------|-----|---------|--------------|
| `disasters.cpp` | 418 | Earthquake, fire, flood, meltdown | **NOT STARTED** |
| `sprite.cpp` | ~600 | Vehicles, helicopters, trains | **NOT STARTED** |
| `graph.cpp` | ~350 | Historical stats/charts | **NOT STARTED** |
| `fileio.cpp` | ~400 | Save/load city files | **PARTIAL** (has basic save/load) |

### MEDIUM PRIORITY - Enhancement Candidates:

| File | LOC | Purpose | Integration |
|------|-----|---------|-------------|
| `generate.cpp` | ~300 | Terrain generation | Could enhance map generation |
| `animate.cpp` | ~250 | Tile animations | Could add visual polish |
| `message.cpp` | ~200 | City events/messages | Could enhance notification system |

### LOWER PRIORITY - Already Implemented:

- `simulate.cpp` → CitySimulation.cpp (16-phase cycle) ✓
- `zone.cpp` → ZoneSimulation.cpp ✓
- `traffic.cpp` → TrafficSimulation.cpp ✓
- `power.cpp` → PowerGrid.cpp ✓
- `budget.cpp` → CityBudget.cpp ✓
- `evaluate.cpp` → CityEvaluation.cpp ✓

---

## 3. Integration Points with Dune Legacy

### Entry Point: `src/Game.cpp`
- `Game::updateGameState()` calls `CitySimulation::advancePhase()`
- Already integrated via `include/Game.h` modifications

### Commands: `src/Command.cpp`
- City commands handled via `Command::executeCommand()` → `CitySimulation::executeCityCommand()`
- **Gap:** Missing disaster triggers (earthquake, fire)

### Structures that need city awareness:
- `WindTrap` → registers power source (modified `src/structures/WindTrap.cpp`)
- `Refinery` → industrial zone pollution
- `ConstructionYard` → building placement

### Header includes:
```cpp
#include <dunecity/CitySimulation.h>
```

---

## 4. What's Changed Since Last Run

**COMPARISON WITH PRIOR ANALYSIS:**

| Item | Prior Status | Current Status | Change? |
|------|-------------|----------------|---------|
| dunecity src files | 7 .cpp | 7 .cpp | NO CHANGE |
| dunecity headers | 8 .h | 8 .h | NO CHANGE |
| Test files | 2 files | 2 files | NO CHANGE |
| Git status | Untracked (??) | Untracked (??) | NO CHANGE |
| Integration | Partial | Partial | NO CHANGE |

**Last file modification:** March 24, 2025 (4 days ago)
**Analysis frequency:** Every 10 minutes (cron job)

**CONCLUSION:** No changes detected in codebase since last analysis. Core migration work is complete - remaining work is git commit and testing.

---

## 5. Testing Strategy (Catch2)

### Actual Dune Legacy Test Setup:
- **Framework: Catch2** (not gtest)
- Build: `cmake -B build && cmake --build build`
- Run: `./build/dunelegacy_tests "[dunecity]"`
- Location: `tests/DuneCityTestCase/DuneCityTestCase.cpp`

### Existing Tests:
- `tests/DuneCityTestCase/DuneCityTestCase.cpp` - Tests ZoneType, DisasterType, CityConstants
- `tests/DuneCityTestCase/TestGlobals.h` - Test fixtures

### Recommended Test Structure:
```
tests/DuneCityTestCase/
├── DuneCityTestCase.cpp    # EXISTING - constants/types
├── test_city_simulation.cpp   # Phase cycling, census
├── test_zone_simulation.cpp   # RCI growth/decay
├── test_traffic_simulation.cpp # Traffic density
└── test_power_grid.cpp        # Power connectivity
```

### Run tests:
```bash
cd ~/development/dune/dunelegacy
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/dunelegacy_tests "[dunecity]"
```

---

## 6. Blockers & Decisions Needed

### BLOCKERS:

1. **CRITICAL: All dunecity code untracked in git** - 3+ weeks outstanding
   - `include/dunecity/` (8 headers)
   - `src/dunecity/` (7 .cpp files)
   - `tests/DuneCityTestCase/` (test files)

### DECISIONS REQUIRED:

| Decision | Options | Recommendation |
|----------|---------|----------------|
| Commit strategy | Branch + PR OR direct commit | Branch + PR (allows review) |
| Traffic/vehicle | Port traffic.cpp OR use Dune vehicles | Use Dune vehicles - Harvester, Trike exist |
| Test framework | Keep Catch2 OR switch to gtest | Keep Catch2 - already integrated |

---

## 7. Actionable Next Steps

### IMMEDIATE (This Week):
1. [ ] **Commit dunecity code** - Add to git and push to Dune Legacy repo
2. [ ] **Run existing tests** - `./build/dunelegacy_tests "[dunecity]"`
3. [ ] **Verify WindTrap integration** - Test power grid in actual gameplay

### SHORT-TERM (This Month):
1. [ ] Add more Catch2 tests for simulation coverage
2. [ ] Add graph/stats UI for city metrics
3. [ ] Verify save/load works with city data

### LONG-TERM:
1. [ ] Integrate Dune vehicles (Harvester, Trike) as city traffic
2. [ ] Add Dune-specific disasters (sandstorms, spice blooms)

---

## 8. Code References

### Key DuneLegacy files:
- `/Users/stefanclaw/development/dune/dunelegacy/src/dunecity/CitySimulation.cpp`
- `/Users/stefanclaw/development/dune/dunelegacy/include/dunecity/CitySimulation.h`
- `/Users/stefanclaw/development/dune/dunelegacy/src/Game.cpp` (integration point)

### Key MicropolisCore files:
- `/Users/stefanclaw/development/simcity/MicropolisCore/MicropolisEngine/src/simulate.cpp` (16-phase cycle)
- `/Users/stefanclaw/development/simcity/MicropolisCore/MicropolisEngine/src/disasters.cpp` (disaster logic)
- `/Users/stefanclaw/development/simcity/MicropolisCore/MicropolisEngine/src/zone.cpp` (RCI zones)
- `/Users/stefanclaw/development/simcity/MicropolisCore/MicropolisEngine/src/traffic.cpp` (traffic AI)

---

## 9. Git Status (As of 2026-03-28 03:00)

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
?? include/dunecity/      ← UNTRACKED (3+ weeks)
?? src/dunecity/         ← UNTRACKED (3+ weeks)
?? tests/DuneCityTestCase/ ← UNTRACKED (3+ weeks)
```

---

*Analysis generated by cron job at 10-minute intervals*
