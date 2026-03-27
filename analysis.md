# DuneCity: SimCity + Dune Legacy Design Analysis

## Executive Summary

This document analyzes the feasibility of merging SimCity-style city building mechanics with Dune Legacy (the open-source Dune 2 remake) using C++. The existing Dune Legacy codebase already contains a partial DuneCity simulation module with zone management, power grids, traffic, and disasters—this provides a strong foundation for the proposed hybrid game.

---

## 1. What Makes Each Game Fun and Unique

### Dune Legacy / Dune 2

| Aspect | What Makes It Fun |
|--------|-------------------|
| **Faction asymmetry** | Three distinct houses (Atreides, Harkonnen, Ordos) with unique units, abilities, and strategies |
| **Base building** | Construction queues, dependency chains (silo before refinery), power management |
| **Resource management** | Spice harvesting, limited deposits, contested territory |
| **Combat tactics** | Unit combos (infantry + tanks + ornithopters), flanking, base defense with turrets |
| **Atmosphere** | Desert aesthetics, sandworms, spice storms, fremen/mentat characters |

### SimCity (Classic)

| Aspect | What Makes It Fun |
|--------|-------------------|
| **Emergent gameplay** | Cities grow organically based on RCI zones, infrastructure |
| **Feedback loops** | Land value → tax revenue → better services → more growth |
| **Problem solving** | Traffic jams, pollution, crime, power outages—all interconnected |
| **Long-term progression** | Population milestones, city specialization, mayor rating |
| **Tile-level detail** | Individual roads, zones, buildings with density changes |

### The Synthesis Opportunity

Dune Legacy already has:
- Faction-based combat and base building
- Spice as a contested resource
- Desert map with unique hazards (sandworms, storms)

SimCity mechanics to add:
- civilian population separate from military units
- RCI zoning with density-based growth
- City services (police, fire, health)
- Economic simulation (taxes, budget, trade routes)

**The pitch**: You're not just building a military base—you're building a **city** on Arrakis. Civilians inhabit your base, generate tax revenue, and create strategic vulnerabilities.

---

## 2. Core Mechanics to Merge

### 2.1 Dual Population System

```
┌─────────────────────────────────────────────────────────┐
│                    HOUSE (Faction)                      │
├─────────────────────────────────────────────────────────┤
│  Military Units ←──── Combat ←──── Player Control      │
│       ↑                                               │
│       │ (consume)                                     │
├───────┼───────────────────────────────────────────────┤
│       │              DuneCity Simulation               │
│  Civilian Pop ←──── RCI Zones ←──── Growth Logic      │
│       ↓                                               │
│  Tax Revenue ←──── City Budget ←──── Player Control   │
└─────────────────────────────────────────────────────────┘
```

**Implementation**: Extend existing `Tile` city zone fields. Military units and civilian population coexist—military structures don't contribute to tax revenue but consume power; civilian zones generate tax but create traffic and vulnerability.

### 2.2 Hybrid Resource System

| Resource | Dune Source | SimCity Source |
|----------|-------------|----------------|
| Spice | Harvested by units | — |
| Credits | Market / combat | Tax revenue |
| Power | Windtraps / Solar | Windtraps / Solar (existing) |
| Population | — | RCI zone growth |
| Materials | Refinery output | Trade routes (new) |

**Key insight**: Spice becomes both a military resource AND an economic driver. High civilian population requires spice imports (trade routes) or creates unrest.

### 2.3 Combat-City Interaction

- **Defensive vulnerability**: Dense civilian areas attract enemy attacks (more targets)
- **Strategic targeting**: Enemy can destroy power grid, causing city panic + military power loss
- **Sandworm behavior**: City zones in desert tiles risk sandworm attacks (existing disaster system)
- **Liberation vs destruction**: Capturing enemy cities intact yields more tax revenue than destroying them

### 2.4 City Services (New)

| Service | Structure | Effect |
|---------|-----------|--------|
| Police | Security Office | Reduces crime, improves land value |
| Fire | Fire Station | Prevents building decay from fires |
| Health | Med Center | Increases population growth |
| Education | Academy | Unlocks advanced units (tech level) |
| Entertainment | Spice Circus | Reduces unrest, attracts commercial |

**Integration**: Reuse existing Dune structure system. Add new city-specific structures that don't exist in vanilla Dune 2.

### 2.5 Map Overlays (Existing + New)

Dune Legacy already supports overlay rendering. Expand with SimCity-style data views:
- Population density
- Pollution levels  
- Land value
- Traffic flow
- Power grid status
- Crime rates

---

## 3. Technical Approach

### 3.1 Architecture Overview

```
┌────────────────────────────────────────────────────────────┐
│                     Main Game Loop                          │
├────────────────────────────────────────────────────────────┤
│  DuneEngine (existing)    │    DuneCity Engine (new)      │
│  ─────────────────────   │    ─────────────────────       │
│  • Unit AI               │    • Zone Simulation           │
│  • Combat System         │    • Population Growth          │
│  • Pathfinding (A*)      │    • Traffic Simulation        │
│  • Structure Build       │    • Economic Model            │
│  • Spice Harvesting      │    • Disaster System           │
│  • Network Sync          │    • Budget Management         │
└────────────────────────────────────────────────────────────┘
                              │
                    Shared World State
                              │
         ┌────────────────────┼────────────────────┐
         ↓                    ↓                    ↓
    ┌─────────┐        ┌─────────────┐      ┌──────────┐
    │  Tile   │        │  PowerGrid  │      │  Radar  │
    │ Layer   │        │  (shared)   │      │  View   │
    └─────────┘        └─────────────┘      └──────────┘
```

### 3.2 C++ Module Structure

```
src/dunecity/                    # NEW - City simulation (existing)
├── CitySimulation.cpp/h         # Main simulation loop
├── ZoneSimulation.cpp/h         # RCI zone growth logic
├── PopulationModel.cpp/h       # Birth/death/migration
├── TrafficSimulation.cpp/h    # Vehicle pathfinding
├── EconomicModel.cpp/h        # Tax/budget/trade
├── PowerGrid.cpp/h            # Power distribution
├── CityScanner.cpp/h          # Map analysis (pollution, crime)
├── CityBudget.cpp/h           # Funding allocation
├── CityEvaluation.cpp/h       # Metrics/milestones
├── Structures/
│   ├── CityHall.cpp/h         # Mayor interface
│   ├── PoliceStation.cpp/h
│   ├── FireStation.cpp/h
│   ├── Hospital.cpp/h
│   └── School.cpp/h
└── UI/
    ├── CityHUD.cpp/h          # Population, funds, time
    ├── BudgetPanel.cpp/h      # Tax/service sliders
    ├── ZoneTool.cpp/h        # RCI placement
    └── DataViewOverlay.cpp/h # Map overlays
```

### 3.3 Integration Points

| Dune System | Integration Point |
|-------------|-------------------|
| `Tile` class | Add `cityZoneType_`, `cityZoneDensity_` (exists) |
| `Game` class | Add `citySimulation_` member |
| `Command` system | Add `CMD_CITY_*` commands |
| `StructureManager` | Add city structure IDs |
| `GUI/Menu` | Add city view panels |
| `Savegame` | CitySimulation save/load |
| `Network` | Sync city state (if multiplayer) |

### 3.4 Rendering Stack

Dune Legacy uses SDL2. No changes needed—reuse existing rendering:
- **Tile renderer**: Extended for zone overlays
- **Sprite system**: New civilian building sprites
- **GUI**: New city management panels
- **Minimap**: Toggle between unit/city views

### 3.5 Build System

Existing CMake structure is sound:

```cmake
# src/CMakeLists.txt (existing pattern)
add_subdirectory(dunecity)

# src/dunecity/CMakeLists.txt
set(DUNECITY_SOURCES
    CitySimulation.cpp
    ZoneSimulation.cpp
    PopulationModel.cpp
    TrafficSimulation.cpp
    EconomicModel.cpp
    PowerGrid.cpp
    CityScanner.cpp
    CityBudget.cpp
    CityEvaluation.cpp
)

add_library(dunecity STATIC ${DUNECITY_SOURCES})
target_link_libraries(dunecity PRIVATE
    dune-core
    dune-gui
    SDL2::SDL2
)
```

---

## 4. Compelling Features

### 4.1 Persistent Civilian Population

- Civilians appear as small sprites in residential zones
- They walk along roads, commute between zones
- Population count displayed in HUD
- Growth depends on jobs, services, land value

### 4.2 Economic Warfare

- Destroying enemy civilian infrastructure reduces their tax revenue
- Sabotage missions target power grids (military + city)
- Capture intact cities for immediate economic boost

### 4.3 Hybrid Victory Conditions

- **Military victory**: Destroy all enemy units/structures
- **Economic victory**: Achieve X population + Y credits per minute
- **Domination**: Control 80% of map population

### 4.4 Dune-Specific City Hazards

| Hazard | Effect | Mitigation |
|--------|--------|------------|
| Sandstorm | Zone damage, visibility loss | Wind walls, forecasts |
| Sandworm | Zone destruction (radius) | Sound detectors, bait |
| Spice bloom | Explosive growth opportunity | Harvest quickly |
| Water shortage | Population decline | Windtraps, trade |

### 4.5 Mayor Rating System

- Tracks city achievements (population milestones, economic stability)
- Affects unit morale (happy city = disciplined troops)
- Unlock special buildings or abilities at thresholds

### 4.6 Trade Routes

- Connect to allied/enemy spice refineries
- Convoys transport goods (vulnerable to attack)
- Creates strategic choke points

---

## 5. Potential Challenges

### 5.1 Technical Challenges

| Challenge | Risk | Mitigation |
|-----------|------|------------|
| Performance (simulation + rendering) | Medium | Phase-based updates, spatial partitioning exists |
| Savegame compatibility | Low | Versioned save format (existing) |
| Network sync for city state | High | Simplify: only sync essential data |
| AI integration | Medium | QuantBot already builds zones; extend logic |

### 5.2 Design Challenges

| Challenge | Risk | Mitigation |
|-----------|------|------------|
| Game pacing | High | Military + city building time balance |
| Complexity overload | High | Progressive tutorial, toggleable features |
| Balance (RTS vs Sim) | High | Faction-specific city bonuses |
| UI clutter | Medium | Tabbed views, keyboard shortcuts |

### 5.3 Scope Management

**Recommended phases**:

1. **Phase 1**: Basic city building (zones, power, population)
2. **Phase 2**: Economic layer (taxes, budget, services)
3. **Phase 3**: Combat integration (city as target)
4. **Phase 4**: Advanced features (trade routes, disasters)

---

## 6. Existing DuneCity Implementation

The codebase already contains significant DuneCity infrastructure:

### Already Implemented

- `CitySimulation` class with 16-phase simulation cycle
- Zone types: Residential, Commercial, Industrial
- Map layers: power, traffic, pollution, land value, crime, population
- Power grid with windtrap/solar sources
- Disaster system: sandstorms, sandworm attacks, sabotage, spice blows
- `QuantBot` AI places zones and roads
- Zone density growth/decay logic
- Budget and tax collection
- City evaluation and metrics

### Integration Status

```
Tile class:         ✓ Has cityZoneType_, cityZoneDensity_, cityPowered_
Game class:         ✓ getCitySimulation() returns pointer
Command system:     ✓ CMD_CITY_PLACE_ZONE, CMD_CITY_SET_TAX_RATE, etc.
Savegame:           ✓ CitySimulation serialization
```

### Next Integration Steps

1. **HUD integration**: Display population, funds, city time
2. **Tool integration**: Allow player to place zones (currently QuantBot only)
3. **Structure integration**: Add city service buildings
4. **UI panels**: Budget screen, data view overlays

---

## 7. Recommendations and Next Steps

### Immediate Actions

1. **Survey existing code**: Review `src/dunecity/` in full, verify all systems compile
2. **Run existing tests**: `cd build && ctest` to ensure DuneCity module works
3. **Create proof-of-concept**: Enable city mode in skirmish, verify QuantBot builds city
4. **Write specification**: Detailed design doc for each city system

### Short-Term (1-2 months)

- Add city HUD overlay (population, funds, date)
- Enable manual zone placement tool
- Add 3-5 city service structures
- Implement tax/budget UI

### Medium-Term (3-6 months)

- Combat-city interaction (civilian casualties, economic damage)
- Trade route system
- Sandworm/sandstorm city hazards
- Faction-specific city bonuses

### Long-Term (6-12 months)

- Full mayor rating system
- Campaign integration (story-driven city building)
- Network sync for city state

### Technical Priorities

1. Don't break existing Dune 2 gameplay
2. Keep city simulation decoupled (can be disabled)
3. Use existing SDL2 rendering (no new dependencies)
4. Follow existing code style and architecture patterns

---

## 8. Conclusion

The merger of SimCity and Dune Legacy is technically feasible and strategically compelling. The existing DuneCity module provides a strong foundation—most of the simulation framework already exists. The primary challenges are design (balancing RTS and city-builder pacing) rather than technical.

The key differentiator: **a city that fights back**. Unlike static SimCity maps or disposable Dune bases, your city is both an economic engine and a military asset. Destroy it and you lose resources; defend it and you project power.

---

*Analysis prepared for DuneCity project development*
*Based on Dune Legacy codebase at ~/development/dune/dunelegacy/*
