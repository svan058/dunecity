# Architecture Comparison — Dune Legacy vs. Micropolis (SimCity)

> Side-by-side analysis of the two source projects that DuneCity will draw from.
> References: [dunelegacy-architecture.md](dunelegacy-architecture.md) | [simcity-architecture.md](simcity-architecture.md)

---

## 1. Project Overview

| Dimension | Dune Legacy | Micropolis (SimCity) |
|---|---|---|
| **Genre** | Real-time strategy (RTS) | City-building simulation |
| **Original** | Dune II: The Building of a Dynasty (1992, Westwood) | SimCity Classic (1989, Maxis / Will Wright) |
| **Nature** | Clean-room reimplementation of Dune II | Open-source release of the original SimCity |
| **Language** | C++17 | C++ engine, C legacy backend, Python/Tcl frontends |
| **License** | GPL v2+ | GPL v3+ |
| **Scale** | ~60,000 lines C++, ~150 source files | ~3,000+ line monolithic engine class + frontends |
| **Platform** | SDL2 — macOS, Windows, Linux | GTK+ / Tcl/Tk — Linux (OLPC origin) |
| **Build system** | CMake + vcpkg | Makefiles + SWIG |
| **External data** | Requires original Dune II PAK files | Self-contained tile/sprite assets |

### What they share

Both are tile-based 2D games with a top-down perspective, both manage a map of discrete tiles with per-tile state, both separate simulation from rendering, and both originated as reimplementations or releases of landmark games from the late 1980s / early 1990s.

### Where they diverge

Dune Legacy is a real-time combat game where the player commands individual units against opponents. Micropolis is a systemic simulation where the player shapes a city through zoning, infrastructure, and budgets — there are no opponents, no direct unit control, and no combat in the traditional sense.

---

## 2. Game Loop

This is one of the most fundamental architectural differences between the two projects.

### Dune Legacy — Fixed-Timestep Loop

```
per frame:
  1. processInput()           — keyboard, mouse, UI events
  2. for each pending tick:
       processObjects()       — update every unit, structure, bullet, trigger
  3. drawScreen()             — render the full frame
  4. networkSync()            — exchange commands (multiplayer only)
```

- Simulation ticks at a fixed rate (~62.5 Hz, 16ms per cycle).
- Rendering is decoupled and runs at the display refresh rate.
- Every tick processes **all** game objects — units move, structures produce, bullets fly.
- In multiplayer, commands are exchanged via deterministic lockstep before each tick advances.

### Micropolis — 16-Phase Cyclic Scan

```
per simulation pass (1 of 16 phases):
  phase 0:   increment cityTime, set growth valves, clear census
  phase 1–8: mapScan() — process 1/8 of the map per phase
  phase 9:   census, tax collection, city evaluation
  phase 10:  power scan (flood-fill)
  phase 11:  pollution, terrain, land value scan
  phase 12:  crime scan
  phase 13:  population density scan
  phase 14:  fire analysis
  phase 15:  disasters, sprite movement
```

- Work is **distributed across 16 phases** so no single frame bears the full cost.
- Different subsystems update at different frequencies (e.g., power every 16th pass, zones 1/8th of the map per pass).
- There is no fixed-timestep guarantee — the simulation advances one phase per call.
- There is no multiplayer synchronization.

### Comparison

| Aspect | Dune Legacy | Micropolis |
|---|---|---|
| **Tick model** | Fixed timestep (~62.5 Hz) | Phase-based (16 phases per city-time unit) |
| **Per-tick scope** | All objects every tick | 1/16th of work per phase |
| **Rendering** | Decoupled from simulation | Tied to phase advancement |
| **Multiplayer** | Deterministic lockstep sync | None |
| **Catch-up** | Runs multiple ticks per frame if behind | No catch-up mechanism |

### Implications for DuneCity

DuneCity will need elements of both: a fixed-timestep loop for real-time unit movement and combat (from Dune Legacy), combined with a phase-distributed city simulation scan (from Micropolis) that spreads zone/infrastructure updates across multiple ticks to avoid frame spikes.

---

## 3. World / Map Representation

### Dune Legacy

- **Grid**: Up to 512 x 512 tiles
- **Tile size**: 64 x 64 pixels at base zoom
- **Tile data**: `Tile` objects storing terrain type, fog-of-war state, spice amount, and references to occupying objects (units/structures)
- **Terrain types**: Sand, rock, dunes, spice, thick spice, mountains
- **Layers**: Single tile grid; fog-of-war is per-tile state; no separate overlay maps
- **Pathability**: Derived from terrain type and occupying objects
- **Revision tracking**: `Map::pathingRevision` increments on terrain changes for path cache invalidation

### Micropolis

- **Grid**: 120 x 100 tiles (fixed)
- **Tile size**: Not specified at pixel level (varies by zoom)
- **Tile data**: 16-bit packed values — lower 10 bits for tile character (0–1023), upper bits for status flags (`PWRBIT`, `CONDBIT`, `BURNBIT`, `BULLBIT`, `ANIMBIT`, `ZONEBIT`)
- **Terrain types**: Water, land, forest, plus all built tile types (roads, rails, zones, buildings)
- **Layers**: Multiple resolution-specific map layers via `Map<DATA, BLKSIZE>`:
  - 1:1 — main tile map
  - 2:1 — power grid, traffic density
  - 4:1 — pollution, crime, land value, population density
  - 8:1 — rate-of-growth maps
- **Bit flags**: Per-tile state encoded directly in the tile word — powered, conductable, burnable, bulldozable, animated, is-zone-center

### Comparison

| Aspect | Dune Legacy | Micropolis |
|---|---|---|
| **Max map size** | 512 x 512 | 120 x 100 (fixed) |
| **Tile encoding** | Rich object (`Tile` class) | Compact 16-bit word with bit flags |
| **Data layers** | Single tile grid | Multiple overlays at varying resolutions |
| **Terrain model** | Natural terrain (sand, rock, spice) | Urban terrain (zones, roads, buildings) |
| **Occupancy** | Object references per tile | Tile character implies building type |
| **Dynamic state** | Fog-of-war, spice amounts | Power, traffic, pollution, crime, land value |

### Implications for DuneCity

DuneCity's map needs to merge both models: a natural desert terrain layer (sand, rock, spice fields) with urban overlay layers (power grid, water distribution, pollution, habitability). The Micropolis multi-resolution layer system is well-suited for city simulation data, while the Dune Legacy tile object model handles occupancy by units and structures.

---

## 4. Entity / Object Systems

### Dune Legacy — Deep OOP Hierarchy

All game entities derive from a single root:

```
ObjectBase (abstract)
├── StructureBase (17 building types)
│   ├── BuilderBase (factories, starport)
│   ├── TurretBase (gun/rocket turrets)
│   ├── Refinery, Silo, Palace, WindTrap, ...
└── UnitBase (15 unit types)
    ├── GroundUnit → TrackedUnit → TankBase → Tank, SiegeTank, ...
    ├── InfantryBase → Soldier, Trooper, Saboteur
    ├── Harvester, MCV, SandWorm
    └── AirUnit → Carryall, Ornithopter, Frigate
```

- Managed by `ObjectManager` with unique IDs and O(1) lookup.
- `SpatialGrid` provides fast proximity queries for targeting and collision.
- Each object has per-tick `update()` logic (movement, targeting, production, etc.).
- Objects are individually addressable, selectable, commandable.

### Micropolis — Tile-Centric with Sprite Pool

Micropolis has no object hierarchy. Instead:

- **Buildings/zones** are encoded directly in the tile map — a 3x3 residential zone is just a cluster of tile values. There are no "building objects."
- **Moving entities** (trains, helicopters, ships, monsters, tornados) are managed as **sprites** in a flat pool with free-list recycling.
- Sprites have type-specific behavior but share a single `SimSprite` structure.
- There are only ~8 sprite types total, and they are cosmetic/disaster-oriented, not player-controlled.

### Comparison

| Aspect | Dune Legacy | Micropolis |
|---|---|---|
| **Entity model** | Deep class hierarchy, each entity is an object | Tiles *are* the buildings; sprites for movers |
| **Entity count** | Dozens to hundreds of individually simulated objects | Tile map + handful of sprites |
| **Management** | `ObjectManager` with unique IDs, spatial grid | Tile array + sprite linked list with free pool |
| **Player interaction** | Select, command, group individual units | Place zones/roads/buildings via tools on the tile map |
| **Per-entity state** | Health, position, target, orders, production queue | Tile bits (powered, burning, etc.) |
| **Polymorphism** | Virtual methods per entity type | Switch on sprite type |

### Implications for DuneCity

DuneCity will need both models coexisting: an object hierarchy for commandable units (harvesters, military, sandworms) following the Dune Legacy pattern, and a tile-encoded zone/infrastructure system following the Micropolis pattern for city buildings that grow and evolve organically rather than being individually managed.

---

## 5. Simulation Model

### Dune Legacy — Object-Driven, Deterministic

- Every game object runs its `update()` method every tick.
- **Deterministic**: Fixed-point math (`FixPoint` / `fix32`) replaces all floating-point in game logic to guarantee identical results across platforms.
- **Seeded RNG**: A single deterministic random generator ensures reproducibility.
- **Command pattern**: All player actions are serialized as `Command` objects, queued, and executed at specific game cycles — enabling replays and multiplayer lockstep.
- Simulation rate: ~62.5 Hz (16ms per cycle).

### Micropolis — Map-Scan, Statistical

- Simulation processes the tile map in sweeps rather than updating individual objects.
- Each zone tile evaluates its surroundings (land value, pollution, crime, power, traffic) and probabilistically grows or declines.
- **Growth valves** (`resValve`, `comValve`, `indValve`) are global controls that throttle zone expansion to maintain balance.
- The simulation is **not deterministic** — it relies on `rand()` calls without seeding guarantees.
- No command serialization; tool effects are applied immediately to the tile map.
- Tax collection and budget cycles occur every 48 simulation time units (~1 game year).

### Comparison

| Aspect | Dune Legacy | Micropolis |
|---|---|---|
| **Update granularity** | Per-object | Per-tile (map scan) |
| **Determinism** | Strict (fixed-point, seeded RNG, lockstep) | Non-deterministic (unsecured `rand()`) |
| **Player actions** | Serialized command objects | Immediate tile map mutations |
| **Time scale** | Real-time seconds (~62.5 ticks/sec) | Abstract city-time (48 ticks/year) |
| **Balance mechanism** | Unit stats, house abilities, AI difficulty | Growth valves, tax rate, budget allocation |
| **State tracking** | Per-object state machines | Per-tile bit flags + aggregate density maps |

### Implications for DuneCity

DuneCity should adopt Dune Legacy's deterministic model as the foundation (critical for any future multiplayer), while incorporating Micropolis's statistical zone-growth mechanics as a higher-level simulation layer that runs on top. Unit-level actions remain command-driven; city-level growth uses valve-controlled probabilistic scanning.

---

## 6. Pathfinding & Spatial Queries

### Dune Legacy

- **Algorithm**: A* search over the tile grid (`AStarSearch.cpp`).
- **Budget system**: A per-cycle token budget (5,000–25,000 tokens, adaptive) caps computational cost. Requests are queued and processed in priority order.
- **Multiplayer sync**: The host negotiates a shared pathfinding budget so all clients spend the same computation, preventing desync from performance differences.
- **Spatial grid**: `SpatialGrid` partitions the map for O(1) proximity queries (targeting, collision avoidance).
- **Cache invalidation**: `Map::pathingRevision` increments on terrain changes so stale paths can be detected.

### Micropolis

- **Algorithm**: Stack-based flood-fill / depth-limited search (max 30 tiles) from zones to destinations.
- **Purpose**: Traffic generation — zones try to "drive" to matching destinations (residential to commercial/industrial, etc.).
- **No budget system**: The search is hard-capped at 30 tiles depth.
- **No spatial indexing**: The map is small enough (120 x 100) that brute-force scanning suffices.
- **Power distribution**: Also flood-fill based — spreads from power plants through conductive tiles.

### Comparison

| Aspect | Dune Legacy | Micropolis |
|---|---|---|
| **Algorithm** | A* (optimal paths) | Stack-based flood-fill (connectivity check) |
| **Scope** | Per-unit, continuous movement | Per-zone, abstract traffic generation |
| **Budget** | Adaptive token budget, multiplayer-synced | Fixed depth limit (30 tiles) |
| **Spatial indexing** | `SpatialGrid` for proximity queries | None needed at 120 x 100 scale |
| **Reuse** | Path caching with revision-based invalidation | No caching; recomputed per zone scan |

### Implications for DuneCity

DuneCity needs A*-based pathfinding with budgeting for unit movement (from Dune Legacy), plus a separate flood-fill system for infrastructure connectivity — power grids, water networks, and road/traffic networks (from Micropolis). These are complementary rather than competing approaches.

---

## 7. Resource / Economy Systems

### Dune Legacy — Spice Economy

- **Resource**: Spice (harvested from spice fields on the map).
- **Harvesting loop**: Harvester units drive to spice fields → collect → return to Refinery → credits deposited.
- **Storage**: Silos and Refineries provide spice storage capacity.
- **Spending**: Credits are spent to build structures and train units (instant deduction).
- **CHOAM**: A marketplace where certain items can be purchased (Starport).
- **Power**: WindTraps generate power; structures consume it. Under-power degrades performance.
- **Per-house tracking**: Each faction (`House`) independently tracks credits, storage, power balance, unit/structure counts.

### Micropolis — Tax & Budget Economy

- **Revenue**: Property tax collected annually (every 48 simulation ticks), rate adjustable 0–20%.
- **Expenditure**: Three categories — roads, police, fire — each funded as a percentage of their ideal budget.
- **Effect scaling**: Funding levels directly scale service effectiveness (`roadEffect`, `policeEffect`, `fireEffect`).
- **Zone growth**: Zones grow organically based on desirability factors (land value, pollution, crime, power, traffic access) — not built directly by the player.
- **Growth valves**: Three global valves (`resValve`, `comValve`, `indValve`) control the rate of residential, commercial, and industrial expansion.
- **No physical resource harvesting**: The economy is abstract — population produces tax revenue.

### Comparison

| Aspect | Dune Legacy | Micropolis |
|---|---|---|
| **Revenue source** | Spice harvesting (physical, map-based) | Tax collection (abstract, population-based) |
| **Spending model** | Direct purchase (build unit/structure) | Budget allocation (fund services) |
| **Resource flow** | Harvester → Refinery → Credits → Build | Population → Tax → Budget → Service effectiveness |
| **Player control** | Micro (place harvesters, choose builds) | Macro (set tax rate, allocate budget) |
| **Growth model** | Player explicitly builds everything | Zones grow/decline organically based on conditions |
| **Power** | WindTraps → structures (binary: powered or not) | Coal/Nuclear plants → flood-fill through wires |
| **Scarcity** | Spice fields deplete | Tax revenue scales with population |

### Implications for DuneCity

DuneCity's economy should layer both models: spice harvesting as the primary resource (Dune Legacy), funding a budget that allocates to city services (Micropolis). The player harvests spice to earn credits, then allocates credits to infrastructure and services. Zones could grow organically near well-serviced, low-danger areas, while military structures and units are built directly.

---

## 8. Infrastructure Systems

### Dune Legacy — Minimal Infrastructure

Dune Legacy has almost no infrastructure simulation:

- **Power**: WindTraps produce power, structures consume it. Under-powered structures degrade. But there is no power grid topology — power is a global per-house balance.
- **No roads/traffic**: Units pathfind freely over terrain.
- **No utilities**: No water, sewage, or service networks.
- **No pollution/crime**: No environmental simulation.

### Micropolis — Rich Infrastructure Simulation

Infrastructure is central to Micropolis:

| System | Mechanics |
|---|---|
| **Power** | Coal/nuclear plants generate capacity. Power propagates via flood-fill through conductive tiles (power lines, roads, zones). Unpowered zones cannot develop. |
| **Traffic** | Zones generate traffic to reach complementary zones. Roads and rails carry traffic. Traffic density accumulates on the density map. Congestion impacts desirability. |
| **Pollution** | Industrial zones, traffic, power plants, airports generate pollution. Pollution spreads via density map. Reduces land value and inhibits growth. |
| **Crime** | Inversely related to land value. Increases with population density. Police stations reduce crime in a radius. |
| **Land value** | Composite of distance to center, development, pollution (negative), crime (negative). Drives which zone types develop. |
| **Fire** | Fires spread between burnable tiles. Fire stations reduce fire damage in a radius. |

### Comparison

| System | Dune Legacy | Micropolis |
|---|---|---|
| **Power** | Global balance (production vs. consumption) | Topological grid (flood-fill connectivity) |
| **Roads/Traffic** | None — free movement over terrain | Roads/rails carry traffic; density tracking |
| **Pollution** | None | Per-tile density map from multiple sources |
| **Crime** | None | Per-tile rate based on land value and police coverage |
| **Land value** | None | Composite metric driving zone desirability |
| **Water** | None | None (neither has it) |
| **Fire** | Structures can be destroyed by combat | Fires spread organically; fire department response |

### Implications for DuneCity

DuneCity should adopt Micropolis's layered infrastructure model — power grids, road/traffic networks, pollution, and habitability — but re-themed for Arrakis. For example: moisture reclamation instead of water, sandstorm exposure instead of generic pollution, shield generator coverage instead of (or in addition to) police/fire. The topological power model (flood-fill through connected tiles) is more interesting gameplay than Dune Legacy's global balance.

---

## 9. UI / Rendering

### Dune Legacy

- **Framework**: Custom widget-based GUI built on SDL2.
- **Widget hierarchy**: `Widget` → `Window`, `Container`, `Button`, `ListBox`, `ScrollBar`, `DropDownBox`, `Slider`, etc.
- **Styling**: `GUIStyle` base with `DuneStyle` for Dune-themed rendering.
- **In-game HUD**: Sidebar (build menus, minimap/radar), top bar (credits, options), news ticker.
- **Rendering**: SDL2 surface/texture rendering with pre-rendered sprites at multiple zoom levels via `GFXManager`.
- **Tight coupling**: GUI is part of the same C++ application as the engine.

### Micropolis

- **Framework**: Callback-driven decoupled architecture — the engine emits events via `CallbackFunction`; any frontend can subscribe.
- **Multiple frontends**: Python/GTK+, Tcl/Tk, and web (TurboGears/Laszlo) frontends all drive the same engine.
- **Rendering**: Frontend-specific — GTK drawing areas, Tcl/Tk canvases, or web canvases.
- **Tool system**: `ToolEffects` batches map modifications for potential undo/redo.
- **Loose coupling**: Engine has zero UI dependencies.

### Comparison

| Aspect | Dune Legacy | Micropolis |
|---|---|---|
| **Coupling** | Engine and UI in same process, same language | Engine fully decoupled via callbacks |
| **Frontend** | Single (SDL2 custom widgets) | Multiple (Python/GTK, Tcl/Tk, web) |
| **Widget system** | Full custom widget toolkit | Relies on host toolkit (GTK, Tk) |
| **Rendering** | SDL2 textures, multi-zoom sprites | Frontend-dependent |
| **Tool system** | Direct command dispatch | `ToolEffects` batching with undo potential |
| **Extensibility** | Add new widgets in C++ | Add new frontends via SWIG/callbacks |

### Implications for DuneCity

Micropolis's engine/UI separation is the stronger architectural pattern — it allows iterating on the frontend without touching simulation code, and opens the door to multiple rendering targets. DuneCity should adopt this decoupled approach while building a richer widget system (from Dune Legacy's design) for the in-game HUD.

---

## 10. AI Systems

### Dune Legacy

Three AI implementations, all deriving from `Player`:

| AI | Role | Approach |
|---|---|---|
| `CampaignAIPlayer` | Campaign missions | Scripted reinforcement/attack patterns |
| `QuantBot` | Skirmish/multiplayer | Configurable via INI; economic/military priority weights; difficulty levels |
| `SmartBot` | Alternative skirmish | Different strategic heuristics |

- AIs receive event callbacks (`onObjectWasBuilt`, `onDamage`, etc.).
- AIs make decisions during their `update()` call each cycle.
- AIs share the same `Player` interface as human players — they issue commands through the same `CommandManager`.

### Micropolis

- **No AI opponent**: Micropolis is a single-player sandbox with no competing factions.
- The simulation itself acts as the "opponent" — the city's systems (crime, pollution, traffic, disasters) create challenge.
- Sprites (monster, tornado) have autonomous behavior but are not strategic agents.

### Comparison

| Aspect | Dune Legacy | Micropolis |
|---|---|---|
| **AI opponents** | Yes — 3 implementations with varying sophistication | None |
| **AI interface** | Same as human player (issues commands) | N/A |
| **Event system** | Callback notifications for game events | Callback system exists but for UI, not AI |
| **Challenge source** | Opponent factions + environmental hazards | City systems + random disasters |
| **Difficulty** | Configurable AI aggressiveness and skill | Disaster frequency; scenario constraints |

### Implications for DuneCity

DuneCity should adopt Dune Legacy's AI framework — AI opponents that share the player interface and issue commands through the same pipeline. The city simulation layer (from Micropolis) provides additional systemic challenge on top of factional competition. Rival houses could have their own cities that grow according to the same zone/infrastructure rules.

---

## 11. Networking

### Dune Legacy

Full multiplayer implementation:

- **Transport**: ENet (reliable UDP), embedded in source.
- **Sync model**: Deterministic lockstep — all clients simulate identically; only commands are exchanged.
- **Discovery**: LAN announcer/finder + internet meta-server.
- **NAT traversal**: UPnP port forwarding + STUN for peer-to-peer connections.
- **Performance sync**: Host collects FPS stats and adjusts shared pathfinding budget to accommodate the slowest client.

### Micropolis

- **No networking**: Micropolis is strictly single-player. No multiplayer of any kind.

### Comparison

| Aspect | Dune Legacy | Micropolis |
|---|---|---|
| **Multiplayer** | Up to 6 players, LAN or internet | None |
| **Sync model** | Deterministic lockstep | N/A |
| **Transport** | ENet (reliable UDP) | N/A |
| **Discovery** | LAN broadcast + meta-server | N/A |
| **NAT traversal** | UPnP + STUN | N/A |

### Implications for DuneCity

If multiplayer is a goal for DuneCity, Dune Legacy's deterministic lockstep model is a proven foundation. The city simulation layer would need to be made deterministic as well (replacing Micropolis's `rand()` calls with a seeded RNG, using fixed-point math for any simulation arithmetic that affects gameplay state).

---

## 12. Design Patterns — Side by Side

| Pattern | Dune Legacy | Micropolis |
|---|---|---|
| **Game loop** | Fixed-timestep with catch-up | 16-phase cyclic scan |
| **Entity management** | Object hierarchy + ObjectManager | Tile map + sprite free-list pool |
| **Spatial indexing** | SpatialGrid (cell-based partitioning) | None (small map) |
| **Pathfinding** | A* with adaptive budget | Stack-based flood-fill (depth 30) |
| **Player actions** | Command pattern (serialized, deterministic) | Immediate tile mutation via ToolEffects |
| **Determinism** | Fixed-point math + seeded RNG | Not guaranteed |
| **UI coupling** | Tight (same-process widget system) | Loose (callback-driven, multi-frontend) |
| **Resource loading** | Centralized managers (GFX, SFX, Font, Text) | Asset files loaded by frontend |
| **Factory pattern** | PlayerFactory (AI/human by type string) | None |
| **Observer pattern** | AI event callbacks (onDamage, onBuilt, etc.) | Engine → UI callbacks |
| **Simulation distribution** | All objects every tick | 1/16th of work per phase |
| **Data encoding** | Rich objects with full state | Compact bit-packed tile words |
| **Map layers** | Single tile grid | Multi-resolution template maps |
| **Tool effects** | Direct command execution | Batched ToolEffects (undo-capable) |
| **History tracking** | None (replays via command log) | Dual-scale (10-year + 120-year) metric buffers |

---

## 13. Key Takeaways for DuneCity

### Adopt from Dune Legacy

- **Deterministic simulation core** — fixed-point math, seeded RNG, command pattern. Non-negotiable for multiplayer correctness and replay support.
- **Object hierarchy for commandable entities** — units, military structures, harvesters, sandworms need individual state, targeting, movement, and AI.
- **A* pathfinding with budgeting** — essential for real-time unit movement at scale.
- **SpatialGrid** — fast proximity queries for targeting, collision, area-of-effect.
- **AI framework** — opponents that share the player interface and issue commands through the same pipeline.
- **Faction system** — houses with independent resource pools, allegiances, and unique abilities.

### Adopt from Micropolis

- **Phase-based city simulation** — distribute zone/infrastructure updates across multiple ticks to avoid frame spikes.
- **Multi-resolution map layers** — separate overlays for power, traffic/transport, pollution, habitability, population density at varying resolutions.
- **Zone-based organic growth** — residential, commercial, and industrial areas that grow or decline based on local conditions rather than explicit player placement.
- **Growth valves** — global throttles that balance competing zone demands.
- **Infrastructure topology** — flood-fill connectivity for power grids, water networks, and road systems.
- **Budget as gameplay lever** — funding levels that scale service effectiveness.
- **ToolEffects batching** — accumulate map changes for potential undo/redo.
- **Engine/UI decoupling** — callback-driven architecture enabling frontend flexibility.
- **Dual-scale history tracking** — short-term and long-term metric buffers for graphs and evaluation.

### Invent for DuneCity

Neither source project provides these, but DuneCity will need them:

- **Hybrid simulation loop** — a fixed-timestep core (for units and combat) with an embedded phase-based city scan (for zones and infrastructure), unified under a single deterministic tick.
- **Arrakis-specific systems** — sandstorm exposure maps, moisture reclamation networks, shield generator coverage, sandworm threat zones, ornithopter patrol routes.
- **Dual interaction model** — direct command of units (RTS) coexisting with zone/tool placement (city builder) in the same interface.
- **Spice-to-budget pipeline** — spice harvesting feeds credits, which fund both direct construction (military) and service budgets (city infrastructure).
- **Rival house cities** — AI opponents that build and grow their own settlements using the same zone/infrastructure simulation, not just military bases.
