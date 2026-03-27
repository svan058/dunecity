# DuneCity C++ Migration Analysis

**Date:** 2026-03-28 00:00  
**Analysis Focus:** MicropolisCore → Dune Legacy integration

---

## 1. MicropolisCore Files to Port

### High Priority (Core Simulation)

| File | LOC | Purpose | Port Status |
|------|-----|---------|--------------|
| `simulate.cpp` | 1729 | Main 16-phase simulation loop | **STUB** - Phase routing exists, algorithms NOT ported |
| `zone.cpp` | 1039 | Zone growth, building placement | PARTIAL - ZoneSimulation.cpp (239 LOC) |
| `scan.cpp` | 600 | Map scanning for census | PARTIAL - CityScanner.cpp (264 LOC) |
| `traffic.cpp` | ~800 | Vehicle pathfinding, traffic density | PARTIAL - TrafficSimulation.cpp (134 LOC) |
| `budget.cpp` | ~400 | Tax collection, fund allocation | **DONE** - CityBudget.cpp (87 LOC) |
| `evaluate.cpp` | ~500 | City score, approval ratings | PARTIAL - CityEvaluation.cpp (132 LOC) |

### Medium Priority (Supporting Systems)

| File | LOC | Purpose | Port Status |
|------|-----|---------|--------------|
| `power.cpp` | ~300 | Power grid connectivity | **DONE** - PowerGrid.cpp (113 LOC) |
| `disasters.cpp` | ~400 | Sandstorm, sandworm, sabotage | STUB - disasters exist but minimal logic |
| `generate.cpp` | ~500 | Terrain generation | NOT STARTED |
| `update.cpp` | ~600 | Sprite updates, animations | NOT STARTED |
| `animate.cpp` | ~300 | Tile animations | NOT STARTED |

---

## 2. Integration Points with Dune Legacy

### Files to Modify

| Dune Legacy File | Modification | Reason |
|-----------------|--------------|--------|
| `src/Game.cpp` | ✅ Already integrates CitySimulation | Creates citySimulation_ |
| `src/Command.cpp` | ✅ executeCityCommand hooked | City commands wired |
| `src/Map.cpp` | May need tile type bridges | Zone placement integration |
| `src/structures/WindTrap.cpp` | Connect power sources | registerPowerSource() calls |
| `src/Tile.cpp` | City tile flags | hasCityZone(), setCityZoneType() |

### Current Integration Status

```
Game.cpp ──────► CitySimulation (orchestrator)
                    │
        ┌───────────┼───────────┐
        ▼           ▼           ▼
   ZoneSim     TrafficSim    PowerGrid
   CityScanner CityBudget   CityEval
```

### Code Metrics (DuneCity Module)
- Total .cpp: 7 files, ~1487 LOC
- Total .h: 8 files, ~760 LOC
- **Module not committed** - shows as untracked in git

---

## 3. What's Changed Since Last Run

**2026-03-27 23:22 → 2026-03-28 00:00**

No code changes detected. Analysis is unchanged.

### Stable Architecture:
- All 7 DuneCity source files present and intact
- CityConstants.h fully defined (NUM_CITY_PHASES=16, CITY_PHASE_INTERVAL=1)
- 16-phase simulation cycle implemented in CitySimulation::advancePhase()

### Remaining Port Gaps:
1. **simulate.cpp (1729 LOC)** - Core 16-phase engine needs Micropolis algorithms ported
   - Phase 0-7: Map scanning (doZone for each tile)
   - Phase 9: Census and tax
   - Phase 10: Decay and messages
   - Phase 11: Power scan
   - Phase 12: Pollution, terrain, land value
   - Phase 13: Crime scan
   - Phase 14: Population density
   - Phase 15: Fire analysis + disasters
2. **zone.cpp (1039 LOC)** - Building construction/demolition logic needs ZoneSimulation completion
3. **scan.cpp (600 LOC)** - Census-taking (population, land value, pollution) needs CityScanner completion
4. **disasters.cpp** - Dune-specific disasters (sandworm, sandstorm) stubbed but incomplete

---

## 4. Testing Strategy

### Unit Tests
```bash
cd ~/development/dune/dunelegacy
./runUnitTests.sh "[dunecity]"
```

### Existing Tests
- `tests/DuneCityTestCase/DuneCityTestCase.cpp` - validates CityConstants
- Uses Catch2 framework

### Test Coverage Gaps
| Module | Test Status |
|--------|-------------|
| CityConstants | ✅ Complete |
| PowerGrid | ❌ No tests |
| CityBudget | ❌ No tests |
| ZoneSimulation | ❌ No tests |
| TrafficSimulation | ❌ No tests |
| CityScanner | ❌ No tests |

### Recommended Test Order:
1. **PowerGrid connectivity** - isolated, deterministic, easiest to test
2. **CityBudget tax calculations** - formula-based, testable
3. **ZoneSimulation growth rules** - needs mock map
4. **TrafficSimulation pathfinding** - needs mock map

---

## 5. Blockers & Decisions Needed

### Decision: Architecture
- [x] **DECIDED:** Keep DuneCity as separate namespace under `dunecity/`
- [x] **DECIDED:** Use Dune's Random class instead of Micropolis's rand()

### Decision: Tile Mapping
- [ ] **PENDING:** How to map Dune tile IDs → Micropolis zone types?
- [ ] **PENDING:** Which Dune structures count as R/C/I zones?

### Decision: Simulation Timing
- [x] **DECIDED:** 16-phase cycle matches MicropolisCore (phaseCycle_ 0-15)
- [x] **DECIDED:** CITY_PHASE_INTERVAL = 1 (every game cycle)

### Blockers
1. **Module not committed** - DuneCity in `src/dunecity/` untracked in git
2. **No test infrastructure for map-dependent code** - CityScanner, ZoneSimulation need mock Map
3. **Disaster algorithms** - Need to implement sandstorm, sandworm, sabotage, spice blow
4. **Missing ZoneSimulation building placement** - Currently only zone type setting, no building spawn logic

---

## 6. Next Steps (Priority Order)

1. **Commit DuneCity module to git** - First step to making progress visible
2. **Write PowerGrid unit tests** - easiest to test in isolation
3. **Complete simulate.cpp port** - port the 16-phase switch statement from MicropolisCore
4. **Complete zone.cpp port** - building placement logic
5. **Write integration tests** - end-to-end city growth scenario

---

## Appendix: File Paths

### Dune Legacy (Target)
```
~/development/dune/dunelegacy/
├── src/dunecity/           # Implementation
│   ├── CitySimulation.cpp  (518 LOC)
│   ├── ZoneSimulation.cpp  (239 LOC)
│   ├── TrafficSimulation.cpp (134 LOC)
│   ├── PowerGrid.cpp       (113 LOC)
│   ├── CityBudget.cpp      (87 LOC)
│   ├── CityEvaluation.cpp  (132 LOC)
│   └── CityScanner.cpp     (264 LOC)
├── include/dunecity/       # Headers
└── tests/DuneCityTestCase/ # Unit tests
```

### MicropolisCore (Source)
```
~/development/simcity/MicropolisCore/MicropolisEngine/src/
├── micropolis.h            # Main class (2834 lines)
├── simulate.cpp            # 16-phase engine ← PRIMARY PORT TARGET (1729 LOC)
├── zone.cpp                # Zone logic (1039 LOC)
├── traffic.cpp             # Pathfinding
├── scan.cpp                # Census (600 LOC)
├── budget.cpp              # Economy
├── power.cpp               # Power grid
├── disasters.cpp           # Disaster events
└── ... (37 files total)
```

### Specific Algorithm Reference Points

**simulate.cpp key functions to port:**
- `doAnimation()` (line ~200)
- `doNoPower()` (line ~300)
- `doMessages()` (line ~400)
- `takeCensus()` (line ~500)
- `cityEvaluation()` (line ~700)
- `setValves()` (line ~900)
- `makeSounds()` (line ~1000)
- `doDisasters()` (line ~1200)
- `fireAnalysis()` (line ~1400)

**zone.cpp key functions to port:**
- `zonePlop()` (line ~100) - Places zone foundation
- `placeBuilding()` (line ~200) - Spawns building in zone
- `getZoneSize()` (line ~300) - Determines zone dimensions
- `roadAdjacency()` (line ~400) - Checks road connectivity

**scan.cpp key functions to port:**
- `pollutionTerrainLandValueScan()` - Phase 12
- `crimeScan()` - Phase 13
- `populationDensityScan()` - Phase 14
- `fireAnalysis()` - Phase 15
