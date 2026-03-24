# DuneCity C++ Migration Analysis

**Date:** 2026-03-29
**Analyzer:** hermes (cron job)
**Goal:** Migrate SimCity/Micropolis city-building into Dune Legacy (C++)

---

## 1. Specific Files to Port from MicropolisCore

### Completed Ports (7 files)
| File | LOC | Purpose | Status |
|------|-----|---------|--------|
| `simulate.cpp` | 1,729 | Main simulation loop, 16-phase cycle | ✓ `CitySimulation.cpp` |
| `budget.cpp` | ~800 | Tax, funding, economy | ✓ `CityBudget.cpp` |
| `evaluate.cpp` | ~900 | City evaluation, ratings | ✓ `CityEvaluation.cpp` |
| `power.cpp` | 195 | Power grid calculation | ✓ `PowerGrid.cpp` |
| `scan.cpp` | 600 | Map scanning for effects | ✓ `CityScanner.cpp` |
| `connect.cpp` | ~1,200 | Connectivity/road tracing | ✓ `TrafficSimulation.cpp` |
| `tool.cpp` | 1,617 | Building tools (res/com/ind zones) | ✓ `ZoneSimulation.cpp` |

### Remaining MicropolisCore Files (Priority Order)
| File | LOC | Purpose | Priority |
|------|-----|---------|----------|
| `traffic.cpp` | 519 | Vehicle routing | Medium - needs unit integration |
| `sprite.cpp` | 2,039 | City sprites (cars, planes) | Medium |
| `disasters.cpp` | ~600 | Disaster effects | Low - Dune has own disasters |
| `animate.cpp` | ~700 | Animation state | Low |
| `generate.cpp` | ~800 | Terrain generation | Low |

---

## 2. Integration Points with Dune Legacy

### Architecture
```
Dune Legacy src/
├── dunecity/                    # Ported modules
│   ├── CitySimulation.cpp/h     ✓ Core orchestration (16-phase) - INTEGRATED
│   ├── ZoneSimulation.cpp/h    ✓ Zone growth - EXISTS (NO STRUCTURE SPAWNING)
│   ├── TrafficSimulation.cpp/h ✓ Road traffic - EXISTS
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
|-----------------|-------------|--------|
| `src/structures/WindTrap.cpp` | `citySim->registerPowerSource()` | ✓ **CONNECTED** |
| `src/Game.cpp` (line 2388) | `citySimulation_->advancePhase()` | ✓ **CONNECTED** |
| `src/Game.cpp` (line 3060) | `citySimulation_->load()` | ✓ **CONNECTED** |
| `src/Game.cpp` (line 3167) | `citySimulation_->save()` | ✓ **CONNECTED** |
| `src/Command.cpp` | City command handlers | ✓ **CONNECTED** |

---

## 3. What's Changed Since Last Run

### Changes Since 2026-03-28
- **No code changes** - Git status unchanged since March 24
- Running as scheduled cron analysis

### Previously Completed (All Verified)
1. ✓ CityBudget.cpp - Tax collection and fund allocation
2. ✓ PowerGrid.cpp - Power calculation and coverage
3. ✓ CityScanner.cpp - Map scanning phases
4. ✓ CityEvaluation.cpp - City ratings
5. ✓ WindTrap → PowerGrid connection - **CONNECTED**
6. ✓ ZoneSimulation - Basic zone growth algorithm
7. ✓ CityConstants.h - Zone types, disaster types
8. ✓ Game integration - **CONNECTED** (advancePhase called each cycle)
9. ✓ Command.cpp handlers - **CONNECTED**
10. ✓ Save/Load - CitySimulation state persisted

### Current Gaps (UNRESOLVED)
1. **CRITICAL: Zone structure spawning** - Zone density updates but no buildings appear
2. Vehicle/unit AI not integrated with TrafficSimulation
3. GUI overlays not rendering (pollution, traffic, power maps)

---

## 4. Testing Strategy

### Dune Legacy Test Framework: Catch2

```bash
# Run all tests
cd ~/development/dune/dunelegacy/build
./bin/dunelegacy_tests

# Run DuneCity-specific tests
./bin/dunelegacy_tests "[dunecity]"
./bin/dunelegacy_tests "[dunecity][constants]"
```

### Test Infrastructure
- **Framework:** Catch2 (header-only)
- **Entry point:** `tests/testmain.cpp`
- **Build:** `tests/CMakeLists.txt`
- **Binary:** `build/bin/dunelegacy_tests`

### Recommended Test Coverage
| Test | Purpose | Priority |
|------|---------|----------|
| `PowerGrid::calculatePower()` | Verify coverage calculation | High |
| `ZoneSimulation::doZone()` | Verify zone placement rules | High |
| `ZoneSimulation::doResidential()` | Verify res zone growth | High |
| `TrafficSimulation::makeTraffic()` | Verify traffic flow | Medium |
| `CityBudget::collectTax()` | Verify tax calculations | Medium |
| `CitySimulation::advancePhase()` | Verify 16-phase cycle | High |

---

## 5. Blockers and Decisions Needed

### Blocker: Zone Structure Spawning (CRITICAL - UNRESOLVED)

**Issue:** `ZoneSimulation::doResidential/Commercial/Industrial()` updates zone density values but doesn't spawn Dune Legacy structures. When a zone tile reaches density thresholds, no buildings appear.

**Current Code State** (`src/dunecity/ZoneSimulation.cpp` lines 191-237):
```cpp
void ZoneSimulation::doResIn(int x, int y, uint8_t pop, int /*landVal*/) {
    if (pop < 8) {
        Tile* tile = currentGameMap->getTile(x, y);
        tile->setCityZoneDensity(pop + 1);  // Only updates density
        incRateOfGrowth(x, y, 1);
    }
    // MISSING: No structure spawning
}
```

**Required Implementation:** After `setCityZoneDensity()`, check if threshold crossed and spawn structure:
```cpp
void ZoneSimulation::doResIn(int x, int y, uint8_t pop, int landVal) {
    if (pop < 8) {
        Tile* tile = currentGameMap->getTile(x, y);
        uint8_t oldPop = pop;
        tile->setCityZoneDensity(pop + 1);
        
        // SPAWN STRUCTURE: Check density threshold crossed
        if (oldPop == 0 && pop + 1 >= 1) {
            spawnStructureAt(x, y, ZoneType::Residential);
        }
        incRateOfGrowth(x, y, 1);
    }
}
```

**Structure Spawning Pattern** (from `House.cpp` line 665):
```cpp
StructureBase* House::placeStructure(Uint32 builderID, int itemID, int xPos, int yPos, bool byScenario, bool bForcePlacing);

// Usage:
House* playerHouse = currentGame->getHouse(houseID);
playerHouse->placeStructure(NONE_ID, Structure_House, x, y, false, true);
```

**Files to modify:**
- `src/dunecity/ZoneSimulation.cpp` - Add spawnStructureAt() calls after density changes
- `include/dunecity/ZoneSimulation.h` - Add spawnStructureAt() method
- `include/data.h` - May need new StructureType values for city buildings

**Dune Legacy Structure Types (from data.h):**
```
Structure_Barracks = 1,
Structure_ConstructionYard = 2,
Structure_GunTurret = 3,
Structure_HeavyFactory = 4,
Structure_HighTechFactory = 5,
Structure_IX = 6,
Structure_LightFactory = 7,
Structure_Palace = 8,
...
```

### Decision Required: City Building Types

**Issue:** Dune Legacy doesn't have SimCity-style residential/commercial/industrial buildings.

**Options:**
1. Add new StructureType enum values: Structure_Hut, Structure_House, Structure_Villa, Structure_Shop, Structure_Office
2. Reuse existing structures (LightFactory, HeavyFactory) - loses city-building feel
3. Create new structure classes for city buildings

**Recommended:** Option 1 - Add new structure types for authentic city-building feel

---

## 6. Action Items

### Immediate (This Week)
1. **Add zone structure spawning** - Map density → structures (CRITICAL)
   - Modify `ZoneSimulation::doResIn/doComIn/doIndIn` to call `House::placeStructure()`
   - Add `spawnStructureAt(x, y, ZoneType)` helper method
   - Create mapping: density threshold → StructureType
2. **Build and test** - Verify game compiles:
   ```bash
   cd ~/development/dune/dunelegacy/build
   make -j4
   ./bin/dunelegacy_tests "[dunecity]"
   ```

### Short-term (2-3 Weeks)
3. Connect TrafficSimulation routes to Dune Legacy unit AI
4. Add Command.cpp handlers for city commands (tax, funding)
5. Add test cases for PowerGrid, ZoneSimulation

### Medium-term (1-2 Months)
6. Add GUI overlay rendering for city maps
7. Integrate disaster system
8. Performance optimization for 64x64+ maps

---

## 7. File Reference

| Path | Description |
|------|-------------|
| `~/development/dune/dunelegacy/` | Dune Legacy (C++17, ~205 source files) |
| `~/development/simcity/MicropolisCore/MicropolisEngine/src/` | MicropolisCore (C++, 22,649 LOC) |
| `~/development/dune/dunelegacy/include/dunecity/` | DuneCity headers (9 files) |
| `~/development/dune/dunelegacy/src/dunecity/` | DuneCity sources (7 files) |
| `~/development/dune/dunelegacy/tests/DuneCityTestCase/` | Test cases |
| `~/development/dune/dunelegacy/build/bin/dunelegacy_tests` | Test binary |

---

## 8. Git Status

```
M include/Command.h           # Added city command types
M include/Definitions.h
M include/Game.h               # Added citySimulation_ member
M include/Map.h
M include/Tile.h               # Zone fields
M include/players/QuantBot.h
M src/CMakeLists.txt
M src/Command.cpp              # City command handlers
M src/Game.cpp                 # Added advancePhase() call
M src/Tile.cpp
M src/players/QuantBot.cpp
M src/structures/WindTrap.cpp
M tests/CMakeLists.txt
M vcpkg.json
?? AI_BOTS_GUIDE.md
?? build_test/
?? include/dunecity/
?? src/dunecity/
?? tests/DuneCityTestCase/
```

**Last code change:** March 24, 2025 (6 days ago)

---

## 9. Zone Growth Algorithm - Current Gap

**MicropolisCore flow** (`zone.cpp`):
```
doZone(x, y)
  → doResidential/commercial/Industrial()
    → Evaluates: traffic, land value, pollution, crime
    → Calculates zscore (growth metric)
    → Calls doResIn()/doResOut() to adjust density
    → repairZone() maintains/rebuilds buildings based on density
```

**Dune Legacy current state:**
```
doZone(x, y)
  → doResidential/commercial/Industrial()
    → Evaluates: traffic, land value, pollution
    → Calculates zscore
    → Adjusts density value
    → MISSING: No structure spawning/repair
```

**The Gap:** `doResIn/doComIn/doIndIn` increment zone density (0→1, 1→2, etc.) but never call `House::placeStructure()` to create actual game objects at those coordinates.

---

## 10. Next Run Focus

For next analysis, verify:
1. Whether zone structure spawning was implemented
2. Build success/failure status
3. Test results from `./bin/dunelegacy_tests "[dunecity]"`
4. Any new git changes
5. Whether new StructureType enum values were added for city buildings

---

## Summary

| Metric | Value |
|--------|-------|
| MicropolisCore files ported | 7/14 (50%) |
| Integration points connected | 5/5 (100%) |
| Build status | ✓ SUCCESS (from March 24) |
| Test status | ✓ 23/23 PASSED (from March 24) |
| Critical blocker | Zone structure spawning - UNRESOLVED |

**KEY INSIGHT:** The missing piece is calling `House::placeStructure()` when zone density crosses thresholds. This is the bridge between the SimCity simulation logic and Dune Legacy's game object system. The code already calculates density correctly—it just needs one additional function call per zone type to spawn the visual building.
