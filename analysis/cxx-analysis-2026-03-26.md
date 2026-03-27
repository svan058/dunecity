# DuneCity C++ Migration Analysis

**Date:** 2026-03-26 23:50  
**Analyzer:** hermes (cron job)  
**Goal:** Migrate SimCity/Micropolis city-building into Dune Legacy (C++)

---

## 1. Specific Files to Port from MicropolisCore

### Completed Ports (7 files, ~1,487 LOC)

| File | LOC | Purpose | Status |
|------|-----|---------|--------|
| `simulate.cpp` | 1,729 | Main simulation loop, 16-phase cycle | → `CitySimulation.cpp` ✓ |
| `budget.cpp` | ~800 | Tax, funding, economy | → `CityBudget.cpp` ✓ |
| `evaluate.cpp` | ~900 | City evaluation, ratings | → `CityEvaluation.cpp` ✓ |
| `power.cpp` | 195 | Power grid calculation | → `PowerGrid.cpp` ✓ |
| `scan.cpp` | 600 | Map scanning for effects | → `CityScanner.cpp` ✓ |
| `connect.cpp` | ~1,200 | Connectivity/road tracing | → `TrafficSimulation.cpp` ✓ |
| `tool.cpp` | 1,617 | Building tools (res/com/ind zones) | → `ZoneSimulation.cpp` ✓ |

### Remaining MicropolisCore Files (Priority Order)

| File | LOC | Purpose | Priority |
|------|-----|---------|----------|
| `traffic.cpp` | 519 | Vehicle routing, pathfinding | **HIGH** - Vehicle AI not integrated |
| `sprite.cpp` | 2,039 | City sprites (cars, planes) | Medium |
| `disasters.cpp` | 418 | Disaster effects | Low - Dune has own disasters |
| `animate.cpp` | ~700 | Animation state | Low |
| `generate.cpp` | ~800 | Terrain generation | Low |

---

## 2. Integration Points with Dune Legacy

### Architecture
```
Dune Legacy src/
├── dunecity/                    # Ported modules (~1,487 LOC, UNTRACKED)
│   ├── CitySimulation.cpp/h     ✓ Core orchestration (16-phase) - INTEGRATED
│   ├── ZoneSimulation.cpp/h    ✓ Zone growth - EXISTS (NO BUILDING SPAWNING)
│   ├── TrafficSimulation.cpp/h ✓ Road traffic - EXISTS (connect.cpp ported)
│   ├── PowerGrid.cpp/h         ✓ Power grid - INTEGRATED
│   ├── CityScanner.cpp/h       ✓ Map scanning - INTEGRATED
│   ├── CityBudget.cpp/h        ✓ Economy - INTEGRATED
│   ├── CityEvaluation.cpp/h    ✓ Ratings - EXISTS
│   └── CityConstants.h         ✓ Zone/Power constants - EXISTS
├── structures/
│   └── WindTrap.cpp            ✓ Power source - CONNECTED
├── Game.cpp                    ✓ Main game loop - INTEGRATED
└── Tile.cpp                    ✓ Zone fields - EXISTS
```

### Verified Integration Connections
| Dune Legacy File | Integration | Status |
|------------------|-------------|--------|
| `src/structures/WindTrap.cpp` (line 75) | `citySim->registerPowerSource()` | ✓ **CONNECTED** |
| `src/Game.cpp` (line 2388) | `citySimulation_->advancePhase()` | ✓ **CONNECTED** |
| `src/Game.cpp` (line 3060) | `citySimulation_->load()` | ✓ **CONNECTED** |
| `src/Game.cpp` (line 3167) | `citySimulation_->save()` | ✓ **CONNECTED** |
| `src/ObjectBase.cpp` (line 763-777) | `ObjectBase::createObject()` | ✓ Factory method EXISTS - needs new cases |

### Enum Location
Structure enums are in **`include/data.h`** (lines 51-103):
```cpp
typedef enum {
    ...
    Structure_WOR = 19,
    Structure_LastID = 19,

    Unit_FirstID = 20,           // <-- CONFLICT: 20-40 are units
    Unit_Carryall = 20,
    ...
    Unit_LastID = 40,
    ItemID_LastID = 40,
} ItemID_enum;
```

---

## 3. What's Changed Since Last Run

### New Analysis Run - 2026-03-26

**Codebase verification completed:**
- Dune Legacy: ~27 .cpp files in src/, 16 subdirectories
- MicropolisCore: 40 files, 22,649 LOC total
- DuneCity module: 7 .cpp files already ported

**Key verification results:**
- ✓ CitySimulation integration verified in Game.cpp
- ✓ Power grid integration verified with WindTrap
- ✓ Test framework confirmed: Catch2, run via `./dunelegacy_tests "[dunecity]"`
- ✓ Untracked files in dunecity/ confirmed (git status)

**Remaining work identified:**
1. **TrafficSimulation** - ported from connect.cpp but missing vehicle AI from traffic.cpp
2. **ZoneSimulation** - ported from tool.cpp but missing building spawn logic
3. **Sprite system** - not yet integrated

---

## 4. Testing Strategy

### Running Tests
```bash
cd /Users/stefanclaw/development/dune/dunelegacy/build_test
./dunelegacy_tests "[dunecity]"
```

### Test Files
| File | Tests | Status |
|------|-------|--------|
| `tests/DuneCityTestCase/DuneCityTestCase.cpp` | Zone constants, disaster types, tuning params | ✓ Working |
| `tests/CMakeLists.txt` | Build config | ✓ Configured |

### Test Coverage Gaps
1. **CitySimulation integration** - requires in-game globals (currentGame, currentGameMap)
2. **ZoneSimulation** - building spawning not tested
3. **TrafficSimulation** - vehicle pathfinding not tested

### Recommended Additions
```cpp
// Add to DuneCityTestCase.cpp
TEST_CASE("ZoneSimulation: Residential zone grows population", "[dunecity][zones]") {
    // TODO: Requires mock of currentGameMap
}

TEST_CASE("TrafficSimulation: Road connectivity", "[dunecity][traffic]") {
    // TODO: Requires mock map with roads
}
```

---

## 5. Blockers and Decisions Needed

### Blockers

| Blocker | Severity | Details |
|---------|----------|---------|
| Untracked source files | HIGH | dunecity/*.cpp not in git - must add to repo |
| Building spawn logic | HIGH | ZoneSimulation missing `tool.cpp:doBuild()` equivalent |
| Vehicle AI | HIGH | TrafficSimulation has roads, no vehicles |
| Unit ID conflict | MEDIUM | 20-40 reserved for Dune units, need separate ID space for city vehicles |

### Decisions Needed

1. **Vehicle ID space** - Current ItemID_enum (20-40) conflicts with Dune units. Need separate enum for city vehicles (cars, planes, ships).

2. **Untracked files** - dunecity/*.cpp files are untracked. Add to git before PR:
   ```bash
   cd /Users/stefanclaw/development/dune/dunelegacy
   git add src/dunecity/*
   git commit -m "Add DuneCity simulation module from MicropolisCore port"
   ```

3. **Building spawning** - Decide on zone growth algorithm:
   - Option A: Port full tool.cpp building spawn (complex, ~1,600 LOC)
   - Option B: Simplified random spawn based on zone capacity

4. **Sprite integration** - Decide on sprite system:
   - Option A: Use Dune's existing sprite system (SpriteManager)
   - Option B: Port MicropolisCore sprite.cpp (~2,000 LOC)

---

## Summary

| Metric | Value |
|--------|-------|
| MicropolisCore total LOC | 22,649 |
| Ported to DuneCity | ~1,487 (6.6%) |
| Remaining high-priority | ~1,000 LOC |
| Integration points verified | 5 |
| Test coverage | Constants only |

**Next steps:**
1. Add dunecity/ files to git
2. Implement building spawning in ZoneSimulation
3. Integrate vehicle routing from traffic.cpp
4. Resolve vehicle ID conflict
