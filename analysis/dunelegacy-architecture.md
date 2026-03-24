# Dune Legacy — Project Nature & Architecture

> Analysis of the Dune Legacy codebase at `/Users/stefanvanderwel/development/dune/dunelegacy/`
> Version 0.99.5 | License: GPL v2+ | Language: C++17

## 1. What Is Dune Legacy?

Dune Legacy is an open-source, modernized reimplementation of Westwood Studios' **Dune II: The Building of a Dynasty** (1992), widely regarded as the game that defined the real-time strategy genre. It is **not** a source port — it is a clean-room engine that reads the original game's data files (PAK format) and recreates the gameplay with modern enhancements.

### Key characteristics

- **Faithful recreation**: Same houses (Atreides, Harkonnen, Ordos), units, structures, campaign missions, and mechanics as Dune II.
- **Modern engine**: SDL2-based rendering, cross-platform (macOS, Windows, Linux), widescreen support, configurable resolution.
- **Multiplayer**: LAN and internet multiplayer via ENet with meta-server lobby, NAT traversal (UPnP/STUN), and deterministic lockstep synchronization.
- **AI bots**: Multiple AI implementations (CampaignAI, QuantBot, SmartBot) with configurable difficulty.
- **Map editor**: Full-featured built-in editor with procedural generation.
- **Modding support**: Mod loading system with INI-based unit/structure data overrides.
- **Cutscenes & campaigns**: Intro, briefings, intermission scenes, and finale sequences from the original game.

## 2. High-Level Architecture

```
┌──────────────────────────────────────────────────────────────────────┐
│                           main.cpp                                   │
│  SDL2 init → Config → FileManager → Resources → MainMenu → Game     │
└──────────────┬───────────────────────────────────────────────────────┘
               │
               ▼
┌──────────────────────────────────────────────────────────────────────┐
│                         Game (core loop)                             │
│  ┌──────────┐  ┌──────────────┐  ┌───────────┐  ┌───────────────┐  │
│  │processIn │→ │processObjects│→ │ drawScreen │→ │ network sync  │  │
│  │  put()   │  │  (per tick)  │  │ (per frame)│  │ (multiplayer) │  │
│  └──────────┘  └──────────────┘  └───────────┘  └───────────────┘  │
│                        │                                             │
│              ┌─────────┼──────────────┐                              │
│              ▼         ▼              ▼                               │
│        ObjectManager  Map      CommandManager                        │
│        (all objects)  (tiles)  (deterministic                        │
│                                 command queue)                       │
└──────────────────────────────────────────────────────────────────────┘
```

The architecture follows a **game loop with fixed-timestep simulation**:

1. **Input processing** (`doInput`) — keyboard, mouse, UI events
2. **Simulation tick** (`processObjects`) — updates every unit, structure, bullet, trigger at a fixed rate (default 16ms/cycle = ~62.5 Hz)
3. **Rendering** (`drawScreen`) — draws the full frame (decoupled from sim rate, limited by vsync)
4. **Network sync** — in multiplayer, commands are exchanged via deterministic lockstep

## 3. Core Systems & Source Layout

### 3.1 Game Object Hierarchy

All game entities derive from a single root class:

```
ObjectBase (abstract)
├── StructureBase
│   ├── BuilderBase (factories, starport)
│   │   ├── ConstructionYard, HeavyFactory, LightFactory
│   │   ├── HighTechFactory, Barracks, WOR
│   │   └── StarPort
│   ├── TurretBase
│   │   ├── GunTurret, RocketTurret
│   ├── Refinery, RepairYard, Radar, Silo
│   ├── Palace, WindTrap, IX, Wall
│   └── ...
└── UnitBase
    ├── GroundUnit
    │   ├── TrackedUnit → TankBase
    │   │   ├── Tank, SiegeTank, SonicTank, Devastator
    │   │   └── Launcher, Deviator
    │   ├── InfantryBase → Soldier, Trooper, Saboteur
    │   ├── Trike, RaiderTrike, Quad
    │   ├── Harvester, MCV
    │   └── SandWorm
    └── AirUnit
        ├── Carryall, Ornithopter, Frigate
```

Objects are managed by `ObjectManager` which assigns unique IDs and provides O(1) lookup. A `SpatialGrid` provides fast proximity queries for targeting and collision.

### 3.2 Source Directory Map

| Directory | Purpose |
|---|---|
| `src/` (root files) | Core engine: `Game.cpp`, `Map.cpp`, `House.cpp`, `ObjectBase.cpp`, `AStarSearch.cpp`, `Bullet.cpp`, `Explosion.cpp`, `SpatialGrid.cpp` |
| `src/FileClasses/` | Resource loading: PAK/SHP/ICN/CPS/VOC file parsers, `GFXManager`, `SFXManager`, `FontManager`, `TextManager`, `Animation` |
| `src/FileClasses/music/` | Music playback: ADL (SoundBlaster OPL emulation), XMI (MIDI), directory-based MP3/OGG |
| `src/GUI/` | Widget-based UI system: `Widget`, `Window`, `Button`, `ListBox`, `ScrollBar`, `DropDownBox` |
| `src/GUI/dune/` | Game-specific UI: `BuilderList`, `InGameMenu`, `ChatManager`, `NewsTicker`, object interface panels |
| `src/Menu/` | Menu screens: `MainMenu`, `SinglePlayerMenu`, `MultiPlayerMenu`, `OptionsMenu`, `MapChoice`, `BriefingMenu`, `ModMenu` |
| `src/Network/` | Multiplayer: `NetworkManager` (ENet), `MetaServerClient`, `LANGameFinderAndAnnouncer`, `UPnPManager`, `StunClient` |
| `src/players/` | AI & player control: `HumanPlayer`, `AIPlayer`, `CampaignAIPlayer`, `QuantBot`, `SmartBot`, `PlayerFactory` |
| `src/structures/` | All building types (see hierarchy above) |
| `src/units/` | All unit types (see hierarchy above) |
| `src/CutScenes/` | Scripted sequences: `Intro`, `Meanwhile`, `Finale`, video/text event system |
| `src/MapEditor/` | Map editor: `MapEditor`, `MapGenerator`, editor windows for settings/reinforcements/teams |
| `src/INIMap/` | Map file format: loading, saving, preview generation for INI-based map definitions |
| `src/Trigger/` | Scenario triggers: `TriggerManager`, `ReinforcementTrigger`, `TimeoutTrigger` |
| `src/misc/` | Utilities: `FileSystem`, `Random`, `Scaler`, `md5`, `format`, `DiscordManager`, stream I/O |
| `src/mod/` | Mod system: `ModManager` for loading and applying gameplay mods |
| `src/enet/` | Embedded ENet networking library (C) |
| `src/fixmath/` | Fixed-point math library (C) for deterministic cross-platform arithmetic |

The `include/` directory mirrors `src/` exactly with corresponding header files.

### 3.3 The Game Loop in Detail

**Entry point** (`main.cpp`):
1. Initialize SDL2 (video, audio, TTF)
2. Load configuration from INI files
3. Initialize `FileManager` (locates and opens PAK files from original Dune II)
4. Load mods via `ModManager`
5. Load all resources: graphics (`GFXManager`), sounds (`SFXManager`), fonts (`FontManager`), text/locale (`TextManager`)
6. Set up video mode and scaling
7. Enter `MainMenu` → user selects game mode → `Game::initGame()` → `Game::runMainLoop()`

**Game::runMainLoop()**:
- Per frame: process input → run N simulation cycles (catch-up if behind) → render
- Each simulation cycle: process triggers → process all objects (units update targeting/movement/combat, structures update production) → process pathfinding queue → update fog of war
- In multiplayer: exchange commands via `CommandManager` in lockstep before advancing the simulation

### 3.4 Deterministic Simulation

The game uses a **deterministic lockstep** model critical for multiplayer:

- **Fixed-point math** (`fixmath/`): All game logic uses `FixPoint` (fix32) types instead of floating-point to guarantee identical results across platforms and compilers.
- **Seeded RNG** (`misc/Random`): A single deterministic random generator shared across all game logic.
- **Command pattern** (`CommandManager`): All player actions (move, attack, build) are serialized as commands and exchanged between peers. Commands are executed at specific game cycles, ensuring all clients simulate identically.
- **Pathfinding budget**: A negotiated token budget limits A* pathfinding per cycle to prevent desync from different execution speeds. The budget is synchronized across all multiplayer clients via a host-managed negotiation protocol.

### 3.5 Pathfinding

- **Algorithm**: A* search (`AStarSearch.cpp`) over the tile grid
- **Budgeting**: A per-cycle token budget (5,000–25,000 tokens, adaptive) limits computational cost. Requests are queued and processed in order. In multiplayer, the host negotiates a budget all clients use.
- **Spatial grid**: `SpatialGrid` partitions the map for fast unit proximity lookups (targeting, collision avoidance)
- **Map revision tracking**: `Map::pathingRevision` increments when terrain changes, allowing paths to be invalidated efficiently

### 3.6 Networking

- **Transport**: ENet (reliable UDP) — embedded in `src/enet/`
- **Lockstep sync**: Commands are buffered and exchanged every N cycles. All clients must confirm before the simulation advances.
- **Game discovery**: LAN announcer/finder + internet meta-server (`MetaServerClient`)
- **NAT traversal**: UPnP port forwarding (`UPnPManager`) and STUN (`StunClient`) for peer-to-peer connections
- **Budget sync**: Host collects FPS/performance stats from clients and adjusts the shared pathfinding budget to accommodate the slowest client

### 3.7 AI System

Three AI implementations exist, all deriving from `Player`:

| AI | Purpose | Complexity |
|---|---|---|
| `CampaignAIPlayer` | Campaign missions — follows scripted reinforcement/attack patterns | Low-medium |
| `QuantBot` | Skirmish/multiplayer — configurable via INI with difficulty levels, economic/military priorities | High |
| `SmartBot` | Alternative skirmish AI with different strategic approach | Medium-high |

AIs receive notifications for game events (unit destroyed, structure built, damage taken) and make decisions during their `update()` call each cycle.

### 3.8 Houses (Factions)

The `House` class represents a faction. Up to `NUM_HOUSES` houses can exist simultaneously. Each house tracks:
- Credits, spice storage capacity
- Power production/consumption
- Unit and structure counts (by type)
- CHOAM marketplace state
- Team allegiance
- Associated players (human and/or AI)

The three primary houses are **Atreides**, **Harkonnen**, and **Ordos**, each with unique special units/abilities faithful to the original Dune II.

### 3.9 Resource & File System

The game requires the original Dune II PAK files. The `FileClasses/` subsystem handles:

- **PAK archives**: Container format holding multiple named files
- **SHP files**: Sprite sheets for units/structures
- **ICN files**: Terrain tile graphics
- **CPS files**: Full-screen images (menus, backgrounds)
- **VOC files**: Sound effects (Creative Voice format)
- **WSA files**: Animation sequences (cutscenes)
- **Palette files**: Color palettes for the 256-color original art

The `GFXManager` pre-renders sprites at multiple zoom levels. The `TextManager` handles localization via gettext PO files. Music can come from ADL (emulated OPL synthesizer for the original MIDI), XMI (MIDI), or directory-based audio files.

### 3.10 GUI System

A custom widget-based GUI framework in `src/GUI/`:

- Base classes: `Widget`, `Window`, `Container`
- Components: `Button`, `TextButton`, `PictureButton`, `Checkbox`, `RadioButton`, `ListBox`, `DropDownBox`, `ScrollBar`, `Slider`, `TextView`
- Styling: `GUIStyle` base with `DuneStyle` implementation (Dune-themed rendering)
- In-game HUD: sidebar (build menus, minimap/radar), top bar (credits, options), news ticker

### 3.11 Map System

- **Tile-based**: Grid of `Tile` objects, each with terrain type, fog-of-war state, spice amount, and references to occupying objects
- **Terrain types**: Sand, rock, dunes, spice, thick spice, mountains
- **Max size**: 512×512 tiles (constant `MAX_XSIZE`, `MAX_YSIZE`)
- **Tile rendering**: 64×64 pixels at zoom level 0 (original Dune II was 16×16)
- **Map format**: INI-based map files, loaded by `INIMapLoader`
- **Procedural generation**: `MapGenerator` in the map editor can create random maps from seeds

## 4. Build System & Dependencies

### Dependencies (via vcpkg)

| Library | Version | Purpose |
|---|---|---|
| SDL2 | 2.32.10 | Windowing, input, rendering |
| SDL2_mixer | 2.8.1 | Audio mixing, music playback |
| SDL2_ttf | 2.24.0 | TrueType font rendering |
| curl | latest | HTTP for meta-server communication |
| miniupnpc | latest | UPnP NAT port forwarding |
| discord-rpc | latest | Discord Rich Presence integration |

### Build targets

| Platform | Target | Output |
|---|---|---|
| macOS | `cmake --build build --target dmg` | `DuneLegacy-0.99.5-macOS.dmg` |
| Windows | `cmake --build build --target installer` | NSIS installer + portable ZIP |
| Linux | `cmake --build build --target package` | DEB, RPM, TGZ packages |

Tests use the **Catch2** framework, enabled with `-DDUNELEGACY_BUILD_TESTS=ON`.

## 5. Data Flow Summary

```
Original Dune II PAK files
        │
        ▼
  FileManager (PAK parser)
        │
  ┌─────┴──────┬──────────┬──────────┐
  ▼            ▼          ▼          ▼
GFXManager  SFXManager  TextManager  INIMapLoader
(sprites)   (sounds)    (locale)     (maps + scenarios)
  │            │          │           │
  └─────┬──────┴──────────┴───────────┘
        ▼
  Game (runtime)
  ├── Map (tile grid)
  ├── House[] (factions)
  │   └── Player[] (human/AI)
  ├── ObjectManager
  │   ├── UnitBase[] (all units)
  │   └── StructureBase[] (all buildings)
  ├── CommandManager (input → deterministic commands)
  ├── TriggerManager (scenario events)
  └── NetworkManager (multiplayer sync)
```

## 6. Key Architectural Patterns

| Pattern | Where | Why |
|---|---|---|
| **Game loop (fixed timestep)** | `Game::runMainLoop()` | Decouples simulation rate from frame rate; enables multiplayer sync |
| **Command pattern** | `CommandManager` | All player actions serialized; enables replays, multiplayer lockstep |
| **Object hierarchy** | `ObjectBase` → `StructureBase` / `UnitBase` → concrete types | Classic OOP inheritance for game entities with polymorphic behavior |
| **Factory pattern** | `PlayerFactory` | Creates AI/Human players by type string |
| **Observer/callback** | `Player::onObjectWasBuilt()`, `onDamage()`, etc. | AI receives game events without coupling to game loop |
| **Spatial partitioning** | `SpatialGrid` | O(1) proximity queries instead of O(n²) pairwise checks |
| **Deterministic simulation** | Fixed-point math, seeded RNG, lockstep | Multiplayer correctness without state synchronization |
| **Resource management** | `FileManager` + `GFXManager/SFXManager` | Centralized loading and caching of game assets |
| **Widget-based GUI** | `GUI/` system | Composable UI with consistent styling |

## 7. Codebase Statistics (Approximate)

- **~150+ source files** (`.cpp`) and matching headers
- **~60,000+ lines** of C++ code (excluding embedded libraries)
- **15 unit types**, **17 structure types**, **3 AI implementations**
- **3 music systems** (ADL, XMI, directory)
- **Full campaign**: 9 missions × 3 houses = 27 campaign missions
- **Multiplayer**: Up to 6 players, LAN or internet
