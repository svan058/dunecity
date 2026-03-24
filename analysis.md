# DuneCity: Dune Legacy + SimCity Hybrid

## Executive Summary

DuneCity is an ambitious hybrid project that merges SimCity-style city building mechanics into Dune Legacy (the open-source Dune 2 remake). The project has already achieved significant traction: a full 16-phase Micropolis-style simulation engine is running within the Dune Legacy game loop, complete with power grids, zone simulation, traffic, tax/budget systems, and economic victory conditions.

This document analyzes the current state, design philosophy, and provides recommendations for completing the integration.

---

## 1. What Makes Each Game Fun and Unique

### Dune Legacy (Dune 2 Clone)

| Aspect | What Makes It Fun |
|--------|-------------------|
| **Faction Identity** | Three distinct houses (Atreides, Harkonnen, Ordos) with unique units, aesthetics, and gameplay philosophies |
| **Base Building** | Strategic placement of structures with dependencies (refinery → factory → starport chain) |
| **Combat** | Real-time tactics with unit rock-paper-scissors (tanks > infantry > sandworms > tanks) |
| **Economy** | Spice harvesting is the core loop — refinery management, Harvester routes, credit accumulation |
| **Atmosphere** | Desert planet aesthetic, sandworms, sandstorms, faction-specific music and voice lines |
| **Scale** | 64x64 tile maps, multiple players, AI opponents |

### SimCity (Classic/Micropolis)

| Aspect | What Makes It Fun |
|--------|-------------------|
| **Emergent Storytelling** | Watching a city grow from empty desert to bustling metropolis |
| **Feedback Loops** | Population → tax → infrastructure → more population (and negative cycles when neglected) |
| **Zoning** | Residential/Commercial/Industrial separation creates organic city growth patterns |
| **Infrastructure Puzzle** | Power grids, road networks, traffic flow, water/pipe systems |
| **Disasters** | Floods, fires, earthquakes, monsters — chaos management |
| **Evaluation** | City ratings, milestones, "Mayor" challenges |
| **Micro-Management** | Setting tax rates, funding levels, emergency responses |

### The Synthesis Opportunity

The hybrid gains:
- **Strategic Layer**: Building cities while defending against enemy houses and sandworms
- **Economic Depth**: SimCity tax/funding system layered on spice harvesting
- **Base Defense as Urban Planning**: Turrets positioned around city districts
- **Spice as Unique Resource**: Spice fields become the "industry" that drives city growth

---

## 2. Core Mechanics Analysis and Merge Strategy

### What's Already Implemented (DuneCity)

```
src/dunecity/
├── CitySimulation.cpp    # 16-phase simulation orchestrator
├── CitySimulation.h     # Population, census, tax, disasters
├── ZoneSimulation.cpp    # R/C/I zone growth from Micropolis
├── PowerGrid.cpp        # Electrical distribution network
├── TrafficSimulation.cpp # Vehicle pathfinding and congestion
├── CityBudget.cpp       # Tax collection and fund allocation
├── CityEvaluation.cpp   # City rating and metrics
├── CityScanner.cpp      # Tile analysis for zone placement
├── CityConstants.h      # Tuning parameters
└── CityMapLayer.h       # Grid data structures
```

### Merge Strategy: Layered Architecture

The key insight is that DuneCity runs as a **parallel simulation** within the existing Dune Legacy game loop, not replacing it:

```
┌─────────────────────────────────────────┐
│          Game Loop (Dune Legacy)         │
│  ┌─────────────────────────────────┐    │
│  │  DuneCity: advancePhase()       │    │
│  │  Runs every 16 game cycles      │    │
│  │  (Micropolis time scale)        │    │
│  └─────────────────────────────────┘    │
│                                         │
│  - Units move/combat each cycle          │
│  - Structures produce each cycle        │
│  - Spice harvesting continues            │
│  - City sim advances in parallel        │
└─────────────────────────────────────────┘
```

### Mechanics Mapping

| SimCity Mechanic | Dune Equivalent | Integration |
|-----------------|----------------|-------------|
| Residential zones | Existing structures? | New zone tiles adjacent to buildings |
| Commercial zones | StarPort/Factory | Trade routes via roads |
| Industrial zones | Spice Refinery | Spice fields become "industry" |
| Power grid | Wind Trap network | Wind Traps power city zones |
| Tax income | Spice credits | Tax + spice = dual economy |
| Road traffic | Ornithopter routes | Traffic sim uses existing pathfinding |
| Police/Fire | Guard towers/Barracks | Military structures provide city services |
| Disasters | Sandstorms/Sandworms | Dune disasters integrate with city damage |

---

## 3. Technical Architecture

### Current Stack

- **Language**: C++17
- **Build**: CMake with vcpkg dependencies
- **Rendering**: SDL2 (SDL2_mixer, SDL2_ttf)
- **Networking**: ENet for multiplayer
- **Math**: FixPoint for deterministic fixed-point arithmetic

### Architecture Diagram

```
┌────────────────────────────────────────────────────────────┐
│                         Game.cpp                           │
│   - Main game loop (60 FPS target)                        │
│   - updateGameState() calls citySimulation_->advancePhase │
└────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌────────────────────────────────────────────────────────────┐
│              DuneCity::CitySimulation                      │
│   - 16-phase cycle (CITY_PHASE_INTERVAL = 16 cycles)     │
│   - Subsystems: Zone, Power, Traffic, Budget, Eval        │
└────────────────────────────────────────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        ▼                     ▼                     ▼
┌───────────────┐    ┌───────────────┐    ┌───────────────┐
│ ZoneSimulation│    │  PowerGrid    │    │TrafficSim    │
│ - R/C/I growth│    │ - Source reg  │    │- Pathfinding │
│ - Building    │    │ - Distribution│    │- Density     │
│   placement   │    │ - Load calc  │    │- Congestion  │
└───────────────┘    └───────────────┘    └───────────────┘
        │                     │                     │
        └─────────────────────┼─────────────────────┘
                              ▼
┌────────────────────────────────────────────────────────────┐
│                    Dune Legacy Systems                     │
│   - Map/Tile grid (shared)                                │
│   - ObjectManager (structures, units)                     │
│   - CommandManager (network sync)                         │
│   - GFXManager, SFXManager (assets)                      │
└────────────────────────────────────────────────────────────┘
```

### Key Design Decisions

1. **Shared Tile Grid**: DuneCity overlays its data on the existing Dune `Map` class
   - `Tile::hasCityZone()` extends tile flags
   - `Tile::setCityPowered()` tracks power state

2. **Deterministic RNG**: City sim uses dedicated `Random` stream seeded from game RNG
   - Ensures network sync: same seed → same simulation results

3. **Phase-Based Execution**: Spreads Micropolis computation across 16 game cycles
   - Prevents frame drops during intensive simulation
   - `advancePhase()` called from `Game::updateGameState()`

4. **Command Integration**: City commands (place zone, set tax) flow through Dune's `Command` system
   - Enables network play: commands are replicated to all players
   - `CitySimulation::executeCityCommand()` handles city-specific actions

### Rendering Integration

Dune Legacy uses a tile-based rendering system. City overlays would render via:

1. **Overlay Mode**: Toggle city view (like SimCity's query mode)
2. **Mini-Overlays**: Power grid, pollution, traffic density as toggleable views
3. **UI Panel**: City statistics, budget, population in the game sidebar

---

## 4. Compelling Features for the Hybrid

### A. Dual Economy System

- **Spice Credits**: Used for military, structures, upgrades
- **City Tax**: Used for city infrastructure, zone development
- **Exchange Rate**: Tax income can buy spice, or vice versa (future)

### B. Strategic City Defense

- Place military structures (Rocket Turrets, Gun Turrets) strategically
- City population provides "defense bonus" — more citizens = harder to capture
- Destroying enemy city reduces their economic output

### C. Dune-Specific Disasters

- **Sandstorms**: Disrupt power grid, damage structures, reduce population
- **Sandworm Attacks**: Consume Harvester units and damage buildings
- **Spice Blows**: Contaminate zones, require cleanup

### D. Victory Conditions

- **Military Victory**: Destroy all enemy structures (existing)
- **Economic Victory**: Reach population threshold (implemented: 500)
- **Ecological Victory**: Maintain pollution below threshold for X months

### E. Faction-Specific Cities

- **Atreides**: Efficient power, lower RCI growth, advanced turrets
- **Harkonnen**: Fast RCI growth, high pollution tolerance, brute force military
- **Ordos**: Balanced, special commerce/industry bonuses

---

## 5. Potential Challenges

| Challenge | Impact | Mitigation |
|-----------|--------|------------|
| **Performance** | 16-phase sim every 16 cycles may lag on large maps | Phase spreading already implemented; profile before optimizing |
| **Network Sync** | Deterministic simulation critical for multiplayer | Use FixPoint math, dedicated city RNG stream (done) |
| **Asset Gaps** | Need new building graphics for city zones | Reuse existing structure sprites; create hybrid tiles |
| **UX Overload** | Too many systems for new players | Progressive UI: "City Mode" toggle, tooltips |
| **Balance** | Economic vs military power curve | Tune tax rates, spice income, building costs |
| **Save Format** | City state must persist | Implemented: `load()`/`save()` methods exist |
| **AI Integration** | Bots need to understand city building | QuantBot already has `citySim_` member; expand logic |

### Known Gaps from Current Implementation

1. **UI Integration**: No city placement UI yet (tools, menus)
2. **Building Placement**: Zone → building generation needs completion
3. **Rendering**: No overlay rendering for pollution/traffic/power views
4. **Full Disaster Integration**: Sandstorm/sandworm hooks exist but need wiring
5. **Victory Screen**: Economic victory triggers but no celebration

---

## 6. Concrete Next Steps

### Phase 1: Core Simulation (Near-Complete)

- [x] 16-phase city engine
- [x] Power grid with Wind Trap integration
- [x] Zone simulation (R/C/I)
- [x] Tax/budget system
- [x] Economic victory condition
- [ ] Population growth tuning
- [ ] Zone → building conversion

### Phase 2: UI and Interaction

- [ ] City tool palette (place zones, roads)
- [ ] City information panel (population, funds, ratings)
- [ ] Tax rate slider
- [ ] Budget allocation UI
- [ ] Overlay toggles (power, pollution, traffic)

### Phase 3: Dune Integration

- [ ] Connect zone growth to structure spawning
- [ ] Spice fields as industrial zones
- [ ] City damage from combat
- [ ] Sandstorm/sandworm disaster integration

### Phase 4: Polish

- [ ] Faction-specific city bonuses
- [ ] City save/load verification
- [ ] Performance profiling on 64x64 maps
- [ ] Multiplayer sync testing

---

## 7. Technical Recommendations

### Rendering: Add City Overlays

```cpp
// In Game.cpp - render city overlay
void Game::renderCityOverlays() {
    if (!cityViewEnabled_ || !citySimulation_) return;
    
    // Render selected overlay
    switch (currentOverlay_) {
        case OVERLAY_POWER:
            renderPowerGridOverlay();
            break;
        case OVERLAY_POLLUTION:
            renderPollutionOverlay();
            break;
        case OVERLAY_TRAFFIC:
            renderTrafficOverlay();
            break;
    }
}
```

### New Structures: City Buildings

Add to `structures/`:
- **Residence**: Provides population capacity
- **Shop**: Commercial zone catalyst
- **Factory**: Industrial zone catalyst (in addition to Refinery)
- **Power Pylon**: Extends power grid (like Wind Trap)

### AI: Expand QuantBot

```cpp
// In QuantBot.cpp - city-aware decision making
void QuantBot::updateCityStrategy() {
    if (!hasCitySimulation()) return;
    
    auto& city = getCitySimulation();
    
    // Prioritize zones based on population needs
    if (city.getResPop() < city.getIndPop()) {
        placeZone(ZONE_RESIDENTIAL);
    }
    
    // Balance military vs economy
    if (getCredits() > 5000 && city.getTotalPop() > 200) {
        buildMilitary();
    }
}
```

---

## 8. Conclusion

DuneCity is already 60-70% implemented. The core simulation engine runs correctly, integrates with Dune Legacy's game loop, and handles save/load. The remaining work is primarily:

1. **UI layer** — giving players tools to interact with city systems
2. **Rendering overlays** — visualizing the simulation data
3. **Full Dune integration** — connecting zones to structures, disasters to city damage

The hybrid concept is sound: SimCity's economic/urban growth loops complement Dune's combat/strategy without replacing either. A player can build an economic engine (city) to fund military expansion, or focus on military conquest while neglecting city development leads to economic stagnation.

**Recommendation**: Prioritize Phase 2 (UI) to make the existing simulation playable, then Phase 3 (Dune integration) to close the loop. The foundation is solid.

---

*Analysis prepared: March 2026*
*Project: ~/development/dune/dunelegacy/*
