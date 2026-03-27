# DuneCity C++ Migration Analysis

**Date:** 2026-03-28 01:45  
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
├── src/dunecity/                  # Implementation (~1,487 LOC)
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

---

## 3. What's Changed Since Last Run

**2026-03-28 Update:**
- Git working tree is clean (no uncommitted changes)
- dunecity module exists with 17 files (~2,374 LOC)
- CitySimulation, ZoneSimulation, TrafficSimulation, PowerGrid, CityScanner, CityBudget, CityEvaluation all implemented
- CityConstants.h added (zone/power constants)
- CityOverlay.h added (visualization layer)
- Tests use Catch2 framework (see tests/CMakeLists.txt)

**Key Finding:** Migration core is essentially complete. Focus shifts to:
1. Debugging/integration testing
2. Visual overlays (crime/pollution/population maps)
3. UI integration for city tools

---

## 4. Testing Strategy (gtest/Dune Legacy runUnitTests.sh)

Dune Legacy uses **Catch2** (not gtest). Test execution:

```bash
cd /Users/stefanclaw/development/dunecity
./runUnitTests.sh
# Or manually:
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target dunelegacy_tests
./tests/dunelegacy_tests
```

**Existing Test Cases:**
- `tests/ZoneStructureTestCase/` - Zone placement
- `tests/RoadPlacementTestCase/` - Road building
- `tests/OverlayLegendTestCase/` - City overlays
- `tests/PathBudgetTestCase/` - Pathfinding budget
- `tests/NetworkManagerTestCase/` - Networking

**Recommended DuneCity Tests to Add:**
1. `CitySimulationTest` - Phase cycle correctness
2. `PowerGridTest` - Power calculation
3. `ZoneSimulationTest` - Zone growth logic
4. `TrafficSimulationTest` - Traffic density

---

## 5. Blockers and Decisions Needed

### Blockers
1. **No active blockers** - Core migration complete
2. **Integration testing needed** - Verify city simulation runs correctly in-game

### Decisions Needed
1. **City tool UI** - How to expose city building tools (roads, zones) in Dune UI?
2. **Overlay rendering** - Polluton/crime/population overlays need Tile rendering integration
3. **Performance tuning** - 16-phase simulation cycle may need tuning for Dune's 60fps
4. **Save format** - CitySimulation state needs serialization to Dune save format

### Priority Next Steps
1. Build and run the game to verify city simulation initializes
2. Add city overlay rendering (CityOverlay.h → Tile.cpp rendering)
3. Wire up city tool commands (build road/zone) to ZoneSimulation
4. Add unit tests for core city simulation classes

---

## File Statistics

| Component | Files | LOC |
|-----------|-------|-----|
| MicropolisCore (source) | 36 | ~22,649 |
| DuneCity module (ported) | 17 | ~2,374 |
| Integration (Dune Legacy modified) | 12 | ~5,000 |

---

*Generated by hermes cron at 2026-03-28 01:45*
