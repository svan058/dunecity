# DuneCity C++ Migration Analysis

**Date:** 2026-03-27
**Analysis Focus:** MicropolisCore → Dune Legacy integration

---

## 1. MicropolisCore Files to Port

### High Priority (Core Simulation)

| File | LOC | Purpose | Port Status |
|------|-----|---------|--------------|
| `simulate.cpp` | ~1200 | Main 16-phase simulation loop | **STUB** - CitySimulation.cpp has basic phase routing |
| `zone.cpp` | ~1500 | Zone growth, building placement | PARTIAL - ZoneSimulation.cpp exists |
| `traffic.cpp` | ~800 | Vehicle pathfinding, traffic density | PARTIAL - TrafficSimulation.cpp exists |
| `scan.cpp` | ~600 | Map scanning for census | PARTIAL - CityScanner.cpp exists |
| `budget.cpp` | ~400 | Tax collection, fund allocation | **DONE** - CityBudget.cpp complete |
| `evaluate.cpp` | ~500 | City score, approval ratings | PARTIAL - CityEvaluation.cpp exists |

### Medium Priority (Supporting Systems)

| File | LOC | Purpose | Port Status |
|------|-----|---------|--------------|
| `power.cpp` | ~300 | Power grid connectivity | **DONE** - PowerGrid.cpp complete |
| `disasters.cpp` | ~400 | Sandstorm, sandworm, sabotage | PARTIAL - stubs in CitySimulation.cpp |
| `generate.cpp` | ~500 | Terrain generation | NOT STARTED |
| `update.cpp` | ~600 | Sprite updates, animations | NOT STARTED |
| `animate.cpp` | ~300 | Tile animations | NOT STARTED |

### Lower Priority (UI/Frontend)

| File | Purpose | Action |
|------|---------|--------|
| `graph.cpp` | Charts/graphs | Skip - Dune has own UI |
| `message.cpp` | City messages | Map to Dune's message system |
| `fileio.cpp` | Save/load | Use Dune's serialization |

---

## 2. Integration Points with Dune Legacy

### Files to Modify

| Dune Legacy File | Modification | Reason |
|-----------------|--------------|--------|
| `src/Game.cpp` | ✅ Already integrates CitySimulation | Line 2293: creates citySimulation_ |
| `src/Command.cpp` | ✅ executeCityCommand hooked | Line 432 |
| `src/Map.cpp` | May need tile type bridges | Zone placement integration |
| `src/structures/*` | WindTrap, etc. | Connect to registerPowerSource() |
| `src/Tile.cpp` | May need Dune→City tile mapping |  |

### Current Integration Status

```
Game.cpp ──────► CitySimulation (orchestrator)
                    │
        ┌───────────┼───────────┐
        ▼           ▼           ▼
   ZoneSim     TrafficSim    PowerGrid
   CityScanner CityBudget   CityEval
```

---

## 3. What's Changed Since Last Run

**First run** - establishing baseline.

### Key Findings:
- DuneCity module already exists in `src/dunecity/` with 7 source files
- Headers in `include/dunecity/`
- Tests in `tests/DuneCityTestCase/` using Catch2
- CitySimulation.h defines complete 16-phase architecture matching MicropolisCore

### Gaps Identified:
1. **simulate.cpp** - The core 16-phase engine needs full implementation
2. **zone.cpp** - Building construction/demolition logic is stubbed
3. **scan.cpp** - Census-taking (population, land value, pollution) needs completion
4. **disasters.cpp** - Dune-specific disasters (sandworm, sandstorm) need original algorithms

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
1. PowerGrid connectivity (isolated, deterministic)
2. CityBudget tax calculations (formula-based, testable)
3. ZoneSimulation growth rules (needs mock map)
4. TrafficSimulation pathfinding (needs mock map)

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
- [ ] **PENDING:** CITY_PHASE_INTERVAL value? (currently undefined in CityConstants.h)

### Blockers
1. **Missing CityConstants.h** - Need to define NUM_CITY_PHASES, MAP_SCAN_PHASES, CITY_PHASE_INTERVAL
2. **No test infrastructure for map-dependent code** - CityScanner, ZoneSimulation need mock Map
3. **Disaster algorithms** - Original Micropolis disasters (earthquake, tornado, flood) vs Dune disasters (sandstorm, sandworm)

---

## 6. Next Steps (Priority Order)

1. **Define CityConstants.h** with missing constants
2. **Write PowerGrid unit tests** - easiest to test in isolation
3. **Complete simulate.cpp** - port the 16-phase switch statement
4. **Complete zone.cpp** port - building placement logic
5. **Write integration tests** - end-to-end city growth scenario

---

## Appendix: File Paths

### Dune Legacy (Target)
```
~/development/dune/dunelegacy/
├── src/dunecity/           # Implementation
│   ├── CitySimulation.cpp
│   ├── ZoneSimulation.cpp
│   ├── TrafficSimulation.cpp
│   ├── PowerGrid.cpp
│   ├── CityBudget.cpp
│   ├── CityEvaluation.cpp
│   └── CityScanner.cpp
├── include/dunecity/       # Headers
└── tests/DuneCityTestCase/ # Unit tests
```

### MicropolisCore (Source)
```
~/development/simcity/MicropolisCore/MicropolisEngine/src/
├── micropolis.h            # Main class (2834 lines)
├── simulate.cpp            # 16-phase engine
├── zone.cpp                # Zone logic
├── traffic.cpp             # Pathfinding
├── scan.cpp                # Census
├── budget.cpp              # Economy
├── power.cpp               # Power grid
├── disasters.cpp           # Disaster events
└── ... (37 files total)
```
