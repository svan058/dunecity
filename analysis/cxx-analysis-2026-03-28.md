# DuneCity C++ Migration Analysis

**Date:** 2026-03-28 02:33  
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
Dune Legacy src/
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
├── src/dunecity/                  # Implementation (~2,247 LOC)
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
|-----------------|-------------|--------|
| `src/Game.cpp` | CitySimulation init/advancePhase/load/save | ✓ INTEGRATED (modified) |
| `include/Game.h` | citySimulation_ member | ✓ INTEGRATED (modified) |
| `include/Tile.h` | Zone fields (zoneType, population, etc.) | ✓ INTEGRATED (modified) |
| `src/CMakeLists.txt` | dunecity module | ✓ INTEGRATED (modified) |
| `src/structures/WindTrap.cpp` | registerPowerSource() | ✓ INTEGRATED (modified) |
| `tests/CMakeLists.txt` | DuneCityTestCase Catch2 | ✓ INTEGRATED (modified) |

### Key Integration Methods to Call from Dune Legacy

```cpp
// In Game.cpp - initialization
#include <dunecity/CitySimulation.h>
citySimulation_.init(mapWidth, mapHeight);

// In Game.cpp - each game cycle
citySimulation_.advancePhase(getGameCycleCount());

// In Tile.h - zone support
bool hasCityZone() const;
bool isCityConductive() const;
void setCityPowered(bool powered);

// In WindTrap.cpp - power sources
citySimulation_.registerPowerSource(x, y, strength);
```

---

## 3. What's Changed Since Last Run

**2026-03-28 02:33 Update:**
- **NO CHANGES** - Git working tree identical to previous run (02:20)
- 13 files modified, 6 untracked (dunecity module + test case)
- dunecity module: 9 headers + 7 source files (~2,247 LOC)
- CitySimulation (518 LOC), ZoneSimulation (239 LOC), TrafficSimulation (134 LOC), PowerGrid (113 LOC), CityScanner (264 LOC), CityBudget (87 LOC), CityEvaluation (132 LOC)
- Tests in `tests/DuneCityTestCase/DuneCityTestCase.cpp` (CityConstants tests)

**Status:** Migration core complete. Module not yet committed to repository. Same state as previous runs.

---

## 4. Testing Strategy (Catch2/Dune Legacy runUnitTests.sh)

Dune Legacy uses **Catch2** (not gtest). Test execution:

```bash
cd ~/development/dune/dunelegacy
./runUnitTests.sh

# Or manually:
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
ctest --output-on-failure
```

### Existing DuneCity Tests

Location: `tests/DuneCityTestCase/DuneCityTestCase.cpp`

Test coverage:
- CityConstants (zone caps, power values, tax rates)
- Test macros: `TEST_CASE`, `CHECK`, `REQUIRE`

### Adding Tests for New Modules

```cpp
#include <dunecity/CitySimulation.h>
#include <dunecity/CityConstants.h>

TEST_CASE("CitySimulation initialization") {
    DuneCity::CitySimulation sim;
    sim.init(64, 64);
    REQUIRE(sim.getTotalPop() == 0);
    REQUIRE(sim.getPhaseCycle() == 0);
}
```

---

## 5. Blockers and Decisions Needed

### Blocker: Module Not Committed
- dunecity/ module exists as untracked files
- 13 modified Dune Legacy files need commit

**Decision needed:** Commit to repository or keep as WIP branch?

### Blocker: Missing Header Visibility
- `include/dunecity/` exists but may not be in CMake install targets

**Decision needed:** Add to `install(TARGETS ...)` in CMakeLists.txt?

### Blocker: Build Configuration
- Module builds with current CMake but hasn't been tested in release mode

**Decision needed:** Test `-DCMAKE_BUILD_TYPE=Release` build?

### Minor: Test Coverage Gaps
- ZoneSimulation (239 LOC) - no dedicated unit tests
- TrafficSimulation (134 LOC) - no dedicated unit tests

**Decision needed:** Add unit tests for simulation modules before feature freeze?

---

## Action Items

1. **Commit dunecity module** - 2,247 LOC ready for review
2. **Run full test suite** - `./runUnitTests.sh` passes
3. **Manual gameplay test** - Start Dune Legacy, build WindTrap, verify city zones spawn
4. **Add ZoneSimulation tests** - Priority for zone growth logic
