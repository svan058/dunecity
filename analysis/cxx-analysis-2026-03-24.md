# DuneCity C++ Architecture Analysis

**Date:** 2026-03-24  
**Status:** Active Development - Phase 2  
**Run:** 2:40 PM (10-minute update #3)

---

## 1. Specific Files to Port from MicropolisCore

### High Priority (Core Simulation - Gaps)

| MicropolisCore File | LOC | Purpose | Port Status | Action |
|---------------------|-----|---------|-------------|--------|
| `src/tool.cpp` | 1617 | Tool/building placement system | **STUB** (3/20 tools) | **PRIORITY** - Expand CMD_CITY_TOOL |
| `src/zone.cpp` | 1039 | R/C/I zone growth logic | **PARTIAL** (239 LOC) | Extend ZoneSimulation |
| `src/scan.cpp` | 600 | Map scanning for zone placement | **PARTIAL** (~300 LOC) | Extend CityScanner |

### Medium Priority (Support - Mostly Done)

| MicropolisCore File | LOC | Purpose | Port Status |
|---------------------|-----|---------|-------------|
| `src/budget.cpp` | 350+ | Tax/budget | **DONE** (CityBudget.cpp) |
| `src/evaluate.cpp` | 550+ | City ratings | **DONE** (CityEvaluation.cpp) |
| `src/traffic.cpp` | 519 | Vehicle traffic | **DONE** (TrafficSimulation.cpp) |
| `src/power.cpp` | 195 | Power grid | **DONE** (PowerGrid.cpp) |
| `src/disasters.cpp` | 418 | Fire, flood | **ADAPTED** (Dune disasters) |

### Source File Locations (Actual Paths)

```
~/development/simcity/MicropolisCore/MicropolisEngine/src/
├── tool.cpp         ← STUB needs expansion (3/20 tools)
├── zone.cpp         ← ZoneSimulation.cpp reference
├── scan.cpp         ← CityScanner.cpp reference  
├── budget.cpp       ✓ CityBudget.cpp (done)
├── evaluate.cpp     ✓ CityEvaluation.cpp (done)
├── traffic.cpp      ✓ TrafficSimulation.cpp (done)
├── power.cpp       ✓ PowerGrid.cpp (done)
├── disasters.cpp   ✓ Adapted (Dune-specific)
├── simulate.cpp    ✓ CitySimulation.cpp (done)
└── micropolis.h    ← Reference for constants
```

### Tool Enum from MicropolisCore (tool.h:142-167)

```cpp
enum EditingTool {
    TOOL_RESIDENTIAL,     // 0 - Residential zone
    TOOL_COMMERCIAL,      // 1 - Commercial zone  
    TOOL_INDUSTRIAL,     // 2 - Industrial zone
    TOOL_FIRESTATION,    // 3 - Fire station
    TOOL_POLICESTATION,  // 4 - Police station
    TOOL_QUERY,          // 5 - Query tool
    TOOL_WIRE,           // 6 - Power wire
    TOOL_BULLDOZER,      // 7 - Bulldoze
    TOOL_RAILROAD,       // 8 - Railroad
    TOOL_ROAD,           // 9 - Road
    TOOL_STADIUM,        // 10 - Stadium
    TOOL_PARK,           // 11 - Park
    TOOL_SEAPORT,        // 12 - Seaport
    TOOL_COALPOWER,      // 13 - Coal power plant
    TOOL_NUCLEARPOWER,   // 14 - Nuclear power plant
    TOOL_AIRPORT,        // 15 - Airport
    TOOL_NETWORK,        // 16 - Network (pipe)
    TOOL_WATER,          // 17 - Water
    TOOL_LAND,           // 18 - Level land
    TOOL_FOREST,         // 19 - Plant forest
    TOOL_COUNT
};
```

**CRITICAL GAP:** Dune Legacy CMD_CITY_TOOL uses different mapping (0=bulldoze, 1=road, 2=power) — needs alignment.

---

## 2. Integration Points with Dune Legacy

### Current Integration (Verified)

| Dune Legacy File | Integration | Line |
|------------------|------------|------|
| `include/Command.h` | CMD_CITY_* enums | 57-66 |
| `src/Command.cpp` | City command handlers | 428-434 |
| `src/Game.cpp` | Calls `advancePhase()` | 2386-2388 |
| `src/CMakeLists.txt` | Added dunecity/*.cpp | 302-308 |
| `src/players/QuantBot.cpp` | Uses CMD_CITY_TOOL (road placement) | 3784-3788 |
| `structures/WindTrap.cpp` | Registers power sources | 73-75 |
| `include/Game.h` | `getCitySimulation()` | 246, 796 |

### Files to Modify Next

| File | Modification | Priority |
|------|--------------|----------|
| `src/dunecity/CitySimulation.cpp` | Expand tool switch (lines 494-511) | HIGH |
| `src/Game.cpp` | Connect save/load to CitySimulation | HIGH |
| `src/structures/*.cpp` | Zone growth callbacks | MEDIUM |
| `include/GUI/*.h` | City stats panel | MEDIUM |

### Current Tool Implementation (CitySimulation.cpp:488-511)

```cpp
case CMD_CITY_TOOL: {
    int x = static_cast<int>(p1);
    int y = static_cast<int>(p2);
    int toolType = static_cast<int>(p3);
    if (currentGameMap && currentGameMap->tileExists(x, y)) {
        Tile* tile = currentGameMap->getTile(x, y);
        switch (toolType) {
            case 0: // Bulldoze
                tile->setCityZoneType(ZoneType::None);
                tile->setCityZoneDensity(0);
                tile->setCityConductive(false);
                break;
            case 1: // Road
                tile->setCityConductive(true);
                break;
            case 2: // Power line
                tile->setCityConductive(true);
                break;
            default:
                break;  // ← MISSING TOOLS 3-20
        }
    }
    break;
}
```

---

## 3. What's Changed Since Last Run (2:31 PM)

### Changes from Previous Analysis

- **No code changes** - All dunecity files remain untracked
- **Tool gap persists** - Still only 3/20 tools implemented
- **Tests still missing** - No dunecity tests created
- **Status unchanged** - Analysis shows consistent picture
- **Test framework confirmed** - Dune Legacy uses **Catch2**, not gtest

### Git Status (Dune Legacy Modified Files)

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
```

### New Files (Untracked - dunecity)

```
?? include/dunecity/    (headers)
?? src/dunecity/        (implementation)
```

### Current dunecity LOC Summary

| File | LOC |
|------|-----|
| CitySimulation.cpp | 518 |
| CityScanner.cpp | 264 |
| ZoneSimulation.cpp | 239 |
| CityEvaluation.cpp | 132 |
| TrafficSimulation.cpp | 134 |
| PowerGrid.cpp | 113 |
| CityBudget.cpp | 87 |
| **Total** | **1,487** |

---

## 4. Testing Strategy Using Catch2

### Dune Legacy Test Command

```bash
cd ~/development/dune/dunelegacy
./runUnitTests.sh
```

### Test Framework: Catch2 (CONFIRMED)

**Finding:** Dune Legacy uses **Catch2**, not gtest.

```bash
# Test runs via CMake
cd build
cmake .. -DBUILD_TESTS=ON
make dunelegacy_tests
./tests/dunelegacy_tests
```

### Test Framework Location

```
~/development/dune/dunelegacy/tests/
├── CMakeLists.txt
├── testmain.cpp
├── FileSystemTestCase/
├── INIFileTestCase/
├── NetworkManagerTestCase/
└── PathBudgetTestCase/
```

### Current Test Gap: Zero dunecity Tests

**Current state:** No test files for any dunecity component.

### Recommended Test Files

```
tests/dunecity/
├── CMakeLists.txt         # Add to main tests
├── test_zone_simulation.cpp
├── test_power_grid.cpp
├── test_traffic_simulation.cpp
├── test_city_budget.cpp
└── test_city_evaluation.cpp
```

### Priority Test Cases

```cpp
// test_zone_simulation.cpp
TEST_CASE("ZoneSimulationTest.ResidentialGrowth") {
    // Place residential zone
    // Verify population grows over time
}

TEST_CASE("ZoneSimulationTest.PowerDependency") {
    // Place unpowered zone
    // Verify no growth without power
}

// test_power_grid.cpp
TEST_CASE("PowerGridTest.PowerFlow") {
    // Place WindTrap
    // Verify power reaches adjacent tiles
}

TEST_CASE("PowerGridTest.NoPowerDrain") {
    // Verify power sources don't drain
}
```

### Test Integration Steps

1. Add to `tests/CMakeLists.txt`:
```cmake
add_subdirectory(dunecity)
```

2. Create `tests/dunecity/CMakeLists.txt`:
```cmake
add_executable(dunecity_tests
    test_zone_simulation.cpp
    test_power_grid.cpp
    test_traffic_simulation.cpp
)

target_link_libraries(dunecity_tests
    PRIVATE
        Catch2::Catch2WithMain
        DuneCity  # Link dunecity library
)
```

---

## 5. Blockers and Decisions Needed

### Critical Blockers

| Blocker | Impact | Status |
|---------|--------|--------|
| **No dunecity Catch2 tests** | Can't verify port correctness | **OPEN** |
| **Tool system incomplete** | Only 3/20 tools work | **CRITICAL** |
| **Save/load disconnected** | City state lost on save | PENDING |
| **Zone rendering missing** | Zones don't appear visually | INVESTIGATE |
| **Tool enum mismatch** | Different tool IDs than MicropolisCore | **DECISION NEEDED** |

### Decisions Needed

1. **Tool ID Alignment Strategy**
   - Option A: Keep Dune Legacy IDs (0=bulldoze, 1=road, 2=power) — breaks Micropolis port
   - Option B: Map Micropolis tool IDs directly — requires changes to QuantBot
   - **Recommendation:** Option B with wrapper layer for clarity

2. **Tool Expansion Priority (based on QuantBot needs)**
   - Tool 1 (Road): ✓ Already implemented
   - Tool 3 (Residential): Needed for zone expansion
   - Tool 4 (Commercial): Needed for trade
   - Tool 5 (Industrial): Needed for jobs
   - Tool 6 (Fire Station): Emergency response
   - Tool 7 (Police Station): Emergency response

3. **Zone Rendering**
   - Need: Tile::getCityOverlay() implementation
   - Current: Zones exist in data but not rendered
   - Action: Investigate `Tile.cpp` city overlay methods

4. **Economic Victory**
   - Current threshold: 500 (abstract)
   - Need: Define what triggers Dune victory condition
   - Action: Connect to game win state

---

## 6. Actionable Next Steps

### Immediate (Today)

1. **Expand tool system** (Priority - CRITICAL)
   - Target: 10 tools minimum by end of day
   - Add tool types 3-9 (Residential, Commercial, Industrial, Fire, Police, Road variants)
   - Map to Dune structure placement
   - Location: `src/dunecity/CitySimulation.cpp:494-511`

2. **Create test infrastructure**
   ```bash
   mkdir tests/dunecity
   # Create CMakeLists.txt + basic tests
   ```

3. **Align tool enum with MicropolisCore**
   - Either modify tool.h or create mapping layer

### Short-term (This Week)

4. **Connect save/load**
   - `CitySimulation::save()` exists
   - Need: Call from `Game::loadGame()` / `Game::saveGame()`

5. **Add zone rendering**
   - Connect CityMapLayer to Tile rendering
   - Show R/C/I overlays on minimap

6. **UI controls**
   - Tax rate adjustment
   - Budget allocation panel
   - City stats display

---

## 7. File Mapping Summary

### MicropolisCore → DuneCity Port

```
micropolis.h              → include/dunecity/CityConstants.h (reference)
simulate.cpp (1729 LOC)   → CitySimulation.cpp (518 LOC) ✓
zone.cpp (1039 LOC)       → ZoneSimulation.cpp (239 LOC) ◐
traffic.cpp (519 LOC)     → TrafficSimulation.cpp ✓
power.cpp (195 LOC)      → PowerGrid.cpp ✓
disasters.cpp (~418)     → CitySimulation.cpp::do*() ✓
budget.cpp (~350)         → CityBudget.cpp ✓
evaluate.cpp (~550)      → CityEvaluation.cpp ✓
scan.cpp (600 LOC)        → CityScanner.cpp ◐
tool.cpp (1617 LOC)      → CMD_CITY_TOOL (STUB - 3/20)
```

### Build Integration (Verified)

```
src/CMakeLists.txt        → lines 302-308: dunecity sources
include/Command.h         → lines 57-66: CMD_CITY_* enums
src/Command.cpp           → lines 428-434: command handlers
src/players/QuantBot.cpp  → lines 3784-3788: uses CMD_CITY_TOOL
include/Game.h            → lines 246, 796: getCitySimulation()
```

---

## 8. Migration Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Tool system too complex | Medium | High | Use Dune structures directly |
| Zone rendering gap | High | Medium | Priority investigation |
| No test coverage | High | High | Create tests immediately |
| Save/load breakage | Medium | High | Connect before release |
| QuantBot can't place zones | **HIGH** | **HIGH** | **Expand tool system NOW** |
| Tool enum mismatch | HIGH | HIGH | Align IDs before porting tools |

---

## 9. Key Integration Details Verified

### QuantBot Usage (Validated)

```cpp
// src/players/QuantBot.cpp:3784-3788
currentGame->getCommandManager().addCommand(
    Command(getPlayerID(), CMD_CITY_TOOL,
            static_cast<Uint32>(tx), static_cast<Uint32>(ty),
            static_cast<Uint32>(1)));  // Tool type 1 = road
```

### Zone Type Enum (Dune Legacy)

From `include/Tile.h`:
```cpp
enum class ZoneType {
    None = 0,
    Residential,
    Commercial,
    Industrial,
    // ...
};
```

---

## 10. Immediate Actions for This Session

1. **Add Tools 3-9 to CitySimulation.cpp** (30 min)
   - Residential, Commercial, Industrial zone placement
   - Fire Station, Police Station placement
   - Map to Tile::setCityZoneType()

2. **Create test directory structure** (15 min)
   - mkdir tests/dunecity
   - Add basic CMakeLists.txt

3. **Document tool mapping** (15 min)
   - Create table: Micropolis ID → Dune Legacy effect

---

*Analysis generated from codebase inspection. Next run: +10 minutes.*
