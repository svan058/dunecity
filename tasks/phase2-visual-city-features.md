# Task: Phase 2-4 — Visual City Features (Big PR)

## Context

DuneCity has a fully working Micropolis city simulation running inside the Dune Legacy game loop. The simulation tracks population, zones, power, traffic, pollution, crime, land value, budget, and evaluation — all updating every game cycle. But there is **zero visual rendering** of any city features beyond a stats HUD overlay (Shift+C).

The game already has:
- `Tile` objects with `cityZoneType_` (None/Residential/Commercial/Industrial) and `cityZoneDensity_` (0-8)
- Zone placement command via `CitySimulation::executeCityCommand()` 
- The QuantBot AI already places zones programmatically
- All city data layers: `powerGridMap_`, `trafficDensityMap_`, `pollutionDensityMap_`, `landValueMap_`, `crimeRateMap_`, `populationDensityMap_`
- Tile rendering pipeline: `blitGround()` → `blitStructures()` → `blitSelectionRects()` in `Game.cpp` lines ~1300-1380

**Goal:** Make the city simulation VISIBLE and INTERACTIVE. After this PR, a player should be able to place zones, see them on the map, see power coverage, and manage their city budget.

## Branch

Create: `feature/phase2-city-visuals`
Base: `main` (after pulling latest)

## Deliverable 1: Zone Overlay Rendering

**Add colored semi-transparent overlays on tiles that have city zones.**

In `Tile::blitGround()` (src/Tile.cpp), after the terrain is drawn, add zone visualization:

```
if (hasCityZone()) {
    // Draw a colored semi-transparent rectangle over the tile
    // Residential = Green (R:0 G:200 B:0 A:80)
    // Commercial  = Blue  (R:0 G:0 B:200 A:80)  
    // Industrial  = Yellow (R:200 G:200 B:0 A:80)
    // Density (0-8) affects alpha: base_alpha + density * 10
    // Use SDL_SetRenderDrawBlendMode + SDL_RenderFillRect
}
```

The game uses `SDL2` for rendering. The renderer is available via `dune::globals::renderer`. Check how `blitSelectionRects()` draws colored rectangles — follow the same pattern.

Each tile is `TILESIZE` pixels (defined in the codebase, likely 64). The overlay should cover the full tile area at the screen position passed to the blit function.

**Also add a zone border/outline** — a 2px colored border around zone tiles to make them identifiable even when zoomed out. Use brighter versions of the zone colors for the border.

## Deliverable 2: Data Layer Overlay System

**Add toggleable map overlays for city simulation data layers.**

Create a new overlay rendering system that can display any `CityMapLayer` as a color-coded heatmap over the game map.

**Overlay modes** (cycled with a keyboard shortcut):
| Key | Overlay | Data Source | Color Range |
|-----|---------|-------------|-------------|
| `Shift+1` | None (default) | - | - |
| `Shift+2` | Power Grid | `powerGridMap_` | Red (unpowered) → Green (powered) |
| `Shift+3` | Traffic Density | `trafficDensityMap_` | Green (low) → Red (high) |
| `Shift+4` | Pollution | `pollutionDensityMap_` | Green (clean) → Purple (polluted) |
| `Shift+5` | Land Value | `landValueMap_` | Red (low) → Green (high) |
| `Shift+6` | Crime Rate | `crimeRateMap_` | Green (safe) → Red (dangerous) |
| `Shift+7` | Population | `populationDensityMap_` | Dark (sparse) → Bright (dense) |

**Implementation:**
- Add `enum class CityOverlayMode` to `GameInterface.h` or a new header
- Add `currentCityOverlay_` state to `GameInterface`
- In the rendering loop in `Game.cpp` (around line 1303-1310), after `blitGround()` and before/after `blitStructures()`, add a call to draw the overlay for visible tiles
- Each overlay reads the appropriate `CityMapLayer` from `currentGame->getCitySimulation()` and draws a semi-transparent colored rectangle based on the data value
- The CityMapLayer uses block sizes (1, 2, 4, or 8 tiles per data point) — the overlay should cover the entire block area
- Show the active overlay name in the city stats HUD when an overlay is active
- Add the keybinds to the keybinds table in `FEATURES.md`

**Color mapping function:** Map the uint8_t/int16_t data value to a color gradient. For uint8_t layers (0-255), normalize to 0.0-1.0 and interpolate between two colors. For int16_t layers, use min/max of the visible area for normalization.

## Deliverable 3: Zone Placement UI

**Allow the player to place R/C/I zones using keyboard + click.**

**Zone placement mode:**
- Press `Z` to enter zone placement mode (show a zone type selector)
- In zone mode: `1` = Residential, `2` = Commercial, `3` = Industrial
- Click on a tile to place the selected zone type
- Right-click or `Escape` to exit zone placement mode
- Show the current zone type as a cursor indicator or status text

**Implementation:**
- This needs to issue a city command. Look at how `CitySimulation::executeCityCommand()` works — it takes `(playerID, cmdType, x, y, zoneType)`. The command type for zone placement should already exist.
- Follow the pattern of how other cursor modes work (e.g., `CursorMode_Attack`, `CursorMode_Move`, `CursorMode_Capture` in `Game.cpp`). Add `CursorMode_CityZone` or similar.
- When clicking in zone mode, create a `Command` that calls `executeCityCommand` with the tile coords and zone type.
- Only allow zone placement on rock tiles (`isRock()`) or slab tiles — not on sand (sand is unstable in Dune lore).
- Show invalid placement feedback (e.g., flash red) when trying to place on sand.
- Add the keybind to `FEATURES.md`

## Deliverable 4: City Budget Screen

**Add a simple budget/economy overlay accessible via `Shift+B`.**

Show a centered modal-style panel (like the existing `InGameMenu` or `GameOptionsWindow`) with:
- **Treasury:** Current city funds
- **Income:** Tax revenue per cycle
- **Expenses:** Road maintenance, Police funding, Fire funding
- **Tax Rate:** Current rate with +/- buttons to adjust (1-20%)
- **Funding sliders** (or simple +/- buttons): Road, Police, Fire funding levels (0-100%)
- **Population breakdown:** Residential / Commercial / Industrial

Follow the pattern of `GameOptionsWindow` or `InGameSettingsMenu` for the Window subclass. Use existing GUI widgets (`Label`, `TextButton`, `ProgressBar`).

The budget data is all available from `CitySimulation`:
- `getTotalFunds()`, `getCityTax()`, `getRoadFund()`, `getPoliceFund()`, `getFireFund()`
- `getResPop()`, `getComPop()`, `getIndPop()`, `getTotalPop()`

To change tax rate, issue a city command (add a new command type if needed, or use `executeCityCommand` with appropriate cmdType).

## Deliverable 5: Enhanced City Stats HUD

**Upgrade the existing Shift+C overlay to be more useful.**

Current HUD just shows text. Upgrade to include:
- Color-coded population bars (R/G/I with proportional widths)
- Growth indicators: ↑↓ arrows next to R/C/I showing valve direction
- Power status: green checkmark if >80% coverage, yellow warning if 50-80%, red alert if <50%
- Active overlay indicator (which Shift+1-7 overlay is on)
- City year display (from `getCityYear()` — calculate from `cityTime_`)
- Mini budget summary: income vs expenses, profit/loss indicator

## Deliverable 6: Update FEATURES.md

Mark all completed items in FEATURES.md. Update the keybinds table with new shortcuts.

## Deliverable 7: Verify Build

```bash
cd ~/development/dunecity
cmake -B build \
  -DCMAKE_PREFIX_PATH=/Users/stefanclaw/dune/dunelegacy/build/vcpkg_installed/arm64-osx \
  -DCMAKE_BUILD_TYPE=Release \
  -DVCPKG_MANIFEST_INSTALL=OFF
cmake --build build -j$(sysctl -n hw.ncpu)
```

Must compile clean with no warnings in new code.

## Technical Notes

- The game renderer is SDL2-based. The global renderer is at `dune::globals::renderer` (check `globals.h`)
- `TILESIZE` is the pixel size of each tile — check `data.h` or similar
- `screenborder->world2screenX/Y()` converts tile coords to screen coords
- `currentGame` is the global game pointer, `currentGameMap` is the global map
- The game uses a custom `DuneStyle` for UI rendering — check `src/GUI/dune/DuneStyle.cpp`
- For semi-transparent rendering: `SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND)` then `SDL_SetRenderDrawColor(renderer, r, g, b, a)` then `SDL_RenderFillRect()`
- Be careful with render state — restore blend mode after overlay drawing

## Constraints

- Must compile on macOS arm64
- Do NOT break save/load compatibility (no changes to serialization format)
- Do NOT modify simulation logic — this is purely visual/UI
- Keep SDL2 render state clean (restore blend modes after drawing)
- Commit with conventional commit messages on the feature branch
- Push the branch and do NOT merge — we'll review via PR
