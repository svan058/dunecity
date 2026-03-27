# DuneCity C++ Migration Analysis

**Date:** 2026-03-27
**Analysis Focus:** MicropolisCore → Dune Legacy integration

---

## 1. MicropolisCore Files to Port

### High Priority (Core Simulation)

| File | LOC | Purpose | Port Status |
|------|-----|---------|--------------|
| `simulate.cpp` | ~1200 | Main 16-phase simulation loop | **STUB** - CitySimulation.cpp has basic phase routing |
| `zone.cpp` | ~1500 | Zone growth, building placement | PARTIAL - ZoneSimulation.cpp (239 LOC) |
| `traffic.cpp` | ~800 | Vehicle pathfinding, traffic density | PARTIAL - TrafficSimulation.cpp (134 LOC) |
| `scan.cpp` | ~600 | Map scanning for census | PARTIAL - CityScanner.cpp (264 LOC) |
| `budget.cpp` | ~400 | Tax collection, fund allocation | **DONE** - CityBudget.cpp (87 LOC) |
| `evaluate.cpp` | ~500 | City score, approval ratings | PARTIAL - CityEvaluation.cpp (132 LOC) |

### Medium Priority (Supporting Systems)

| File | LOC | Purpose | Port Status |
|------|-----|---------|--------------|
| `power.cpp` | ~300 | Power grid connectivity | **DONE** - PowerGrid.cpp (113 LOC) |
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
| `src/Game.cpp` | ✅ Already integrates CitySimulation | Creates citySimulation_ |
| `src/Command.cpp` | ✅ executeCityCommand hooked | City commands wired |
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

### Code Metrics (DuneCity Module)
- Total .cpp: 7 files, 1487 LOC
- Total .h: 8 files, 760 LOC
- **Module not yet committed** - shows as untracked in git

---

## 3. What's Changed Since Last Run

**Routine check** - No code changes detected since last analysis.

### Stable Architecture:
- All 7 DuneCity source files present and intact
- CityConstants.h fully defined (NUM_CITY_PHASES=16, CITY_PHASE_INTERVAL=1)
- 16-phase simulation cycle implemented in CitySimulation::advancePhase()

### Remaining Port Gaps:
1. **simulate.cpp** - Core 16-phase engine needs Micropolis algorithms ported
2. **zone.cpp** - Building construction/demolition logic needs ZoneSimulation completion
3. **scan.cpp** - Census-taking (population, land value, pollution) needs CityScanner completion
4. **disasters.cpp** - Dune-specific disasters (sandworm, sandstorm) algorithms need implementation

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
- [x] **DECIDED:** CITY_PHASE_INTERVAL = 1 (every game cycle)

### Blockers
1. **Module not committed** - DuneCity in `src/dunecity/` untracked in git
2. **No test infrastructure for map-dependent code** - CityScanner, ZoneSimulation need mock Map
3. **Disaster algorithms** - Need to implement sandstorm, sandworm, sabotage, spice blow

---

## 6. Next Steps (Priority Order)

1. **Commit DuneCity module to git** - First step to making progress visible
2. **Write PowerGrid unit tests** - easiest to test in isolation
3. **Complete simulate.cpp** - port the 16-phase switch statement from MicropolisCore
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
├── simulate.cpp            # 16-phase engine ← PRIMARY PORT TARGET
├── zone.cpp                # Zone logic
├── traffic.cpp             # Pathfinding
├── scan.cpp                # Census
├── budget.cpp              # Economy
├── power.cpp               # Power grid
├── disasters.cpp           # Disaster events
└── ... (37 files total)
```
