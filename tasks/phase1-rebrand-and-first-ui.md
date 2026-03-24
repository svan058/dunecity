# Task: Phase 1 — Rebrand to DuneCity + First Visible City Feature

## Context

DuneCity is a fork of Dune Legacy (open-source Dune 2 RTS) with Micropolis-style city simulation integrated into the game loop. The city simulation code is complete and running — CitySimulation, ZoneSimulation, PowerGrid, TrafficSimulation, CityBudget, CityEvaluation — all in `src/dunecity/` and `include/dunecity/`. It's wired into `Game.cpp` via `citySimulation_->advancePhase()` every cycle, saves/loads, and has an economic victory condition.

**Problem:** The compiled binary is still `dunelegacy.app`, the window title says "Dune Legacy", the CMake project is `DuneLegacy`, and there is zero UI for any city features. The game looks and feels exactly like Dune Legacy.

## Deliverables

### 1. Rebrand: Dune Legacy → DuneCity

**Scope:** User-visible strings and build artifacts only. Do NOT rename internal C++ namespaces, class names, or refactor code structure.

Change these:

- **CMakeLists.txt:**
  - `project(DuneLegacy ...)` → `project(DuneCity ...)`
  - Binary/target name: `dunelegacy` → `dunecity` (the executable/app bundle)
  - All `DUNELEGACY_*` CMake options/variables → `DUNECITY_*`
  - The macOS app bundle should produce `dunecity.app` not `dunelegacy.app`

- **Window title** (`src/main.cpp` line ~181):
  - `SDL_CreateWindow("Dune Legacy", ...)` → `SDL_CreateWindow("DuneCity", ...)`

- **All user-facing strings in `src/main.cpp`:**
  - Usage text: `dunelegacy` → `dunecity`
  - Log messages: `"Starting Dune Legacy %s"` → `"Starting DuneCity %s"`
  - Error dialogs: `"Dune Legacy"` → `"DuneCity"`
  - Performance log: `"Dune Legacy-Performance.log"` → `"DuneCity-Performance.log"`

- **configure.ac:** Update `AC_INIT` from dunelegacy → dunecity

- **File renames** (if they affect the build):
  - `dunelegacy.desktop` → `dunecity.desktop` (update contents too)
  - `dunelegacy.6` man page → `dunecity.6`
  - `resource.rc` — update any version info strings

- **DO NOT** rename:
  - Asset files (`dunelegacy-128x128.png`, `dunelegacy.icns` etc.) — those are just source assets
  - The `DUNE-LEGACY-README` file — that's historical
  - License headers in source files — those are legally meaningful
  - Internal function names like `getDuneLegacyDataDir()` — that's a refactor, not a rebrand

### 2. City Stats HUD Overlay

Add a small, toggleable HUD panel that displays live city simulation data during gameplay. This is the first visible proof that DuneCity is not just Dune Legacy.

**Requirements:**

- **Toggle:** Keyboard shortcut `C` to show/hide the city overlay
- **Position:** Top-right corner of the game screen, semi-transparent background
- **Content (live from CitySimulation):**
  - Population (from `getTotalPop()`)
  - City Budget / Treasury (from `cityBudget_`)
  - Tax Rate
  - Power coverage % (powered tiles / total tiles that need power)
  - City Score / Evaluation rating (from `cityEval_`)
  - Game year (from `cityTime_`)
- **Style:** Use the existing GUI text rendering system (SDL2 text / `GUIStyle`). Keep it simple — white text on dark semi-transparent rectangle. Match the existing Dune Legacy aesthetic (the game uses a specific palette/font system).
- **Integration point:** `GameInterface::draw()` in `src/GameInterface.cpp` — draw after the main interface so it overlays on top. Access city data via `currentGame->getCitySimulation()`.

**Implementation hints:**
- Look at how the existing game speed indicator or credits display works — follow the same pattern
- The game uses `DuneStyle` for rendering with `SDL2` surfaces/textures
- `GUIStyle::getInstance()` has text drawing methods
- Keep the overlay small — maybe 200x150 pixels

### 3. Feature Tracking — FEATURES.md

Create `FEATURES.md` in the repo root as a living task list for the DuneCity project. Use GitHub-flavored markdown checkboxes.

```markdown
# DuneCity Feature Roadmap

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

## Phase 1: Identity & First UI
- [ ] Rebrand: binary, window title, project name → DuneCity
- [ ] City Stats HUD overlay (population, budget, power, score)
- [ ] Feature tracking (this file)

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
```

### 4. Verify Build

After all changes, ensure the project still compiles:
```bash
cd ~/development/dunecity
cmake -B build \
  -DCMAKE_PREFIX_PATH=/Users/stefanclaw/dune/dunelegacy/build/vcpkg_installed/arm64-osx \
  -DCMAKE_BUILD_TYPE=Release \
  -DVCPKG_MANIFEST_INSTALL=OFF
cmake --build build -j$(sysctl -n hw.ncpu)
```

The binary should now be `build/bin/dunecity.app` (not `dunelegacy.app`).

## Constraints

- Must compile on macOS arm64 with the existing vcpkg dependencies
- Do NOT break save/load compatibility
- Do NOT modify game logic or simulation code — this is UI + branding only
- Keep changes minimal and focused — don't refactor unrelated code
- Commit with conventional commit messages: `feat:`, `chore:`, etc.
- Create a feature branch `feature/phase1-rebrand-hud` and commit there
