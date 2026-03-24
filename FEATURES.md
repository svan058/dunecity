# DuneCity Feature Roadmap

## Keybinds
| Key | Action |
|-----|--------|
| `C` | Capture unit mode (existing) |
| `Shift+C` | Toggle city stats HUD |

---

## Phase 0: Foundation (DONE)
- [x] Fork Dune Legacy codebase
- [x] Integrate Micropolis city simulation engine
- [x] 16-phase city simulation loop (CitySimulation)
- [x] Zone simulation (R/C/I zones)
- [x] Power grid simulation
- [x] Traffic simulation
- [x] City budget and tax system
- [x] City evaluation and scoring
- [x] Save/Load city state
- [x] Economic victory condition

## Phase 1: Identity & First UI (DONE)
- [x] Rebrand: binary, window title, project name → DuneCity
- [x] City Stats HUD overlay (population, budget, power, score)
- [x] Feature tracking (this file)

## Phase 2: Zoning UI
- [ ] Zone placement tool (R/C/I zones on map tiles)
- [ ] Visual zone indicators on map (colored overlays or tile borders)
- [ ] Zone info tooltip on hover

## Phase 3: Power Grid Visualization
- [ ] Power grid overlay mode (show powered/unpowered areas)
- [ ] Wind Trap radius visualization
- [ ] Power shortage warnings

## Phase 4: City Building Menu
- [ ] Dedicated "City" build menu tab alongside military buildings
- [ ] City-specific structures (roads, power lines, zone markers)
- [ ] Build cost integration with city budget

## Phase 5: Economy Integration
- [ ] Dual economy display (Spice credits + City tax revenue)
- [ ] City budget management screen
- [ ] Tax rate adjustment UI
- [ ] Funding allocation sliders (police, fire, transport)

## Phase 6: Traffic & Roads
- [ ] Road placement tool
- [ ] Traffic density visualization overlay
- [ ] Road network affecting zone growth rates

## Phase 7: Map Overlays
- [ ] Data layer toggles: crime, pollution, land value, population density
- [ ] Color-coded overlay rendering on game map
- [ ] Legend/key for overlay values

## Phase 8: Disasters & Events
- [ ] Sandstorm damage to city zones
- [ ] Sandworm attacks on populated areas
- [ ] Fire spread from combat into city zones
- [ ] Disaster notification system

## Phase 9: Polish & Balance
- [ ] Balance city growth rates with RTS pacing
- [ ] Tutorial/tooltip for city mechanics
- [ ] City milestone notifications
- [ ] Sound effects for city events

## Future Ideas
- [ ] Multiplayer city comparison (competitive city scores)
- [ ] Faction-specific city bonuses (Atreides = higher approval, Harkonnen = cheaper industry)
- [ ] Inter-city trade routes between allied players
- [ ] City-produced units (militia from population)
