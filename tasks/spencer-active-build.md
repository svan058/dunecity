# Spencer Active Build — City-Specific Structures: Roads

## Objective
Make **roads** the first buildable city-specific structure in DuneCity. Keep it to one concrete deliverable: a player can place roads from the existing City build menu / construction flow, and placed roads become conductive city tiles.

## Verified starting point
- `CMD_CITY_TOOL` already exists in `include/Command.h`.
- `src/dunecity/CitySimulation.cpp` already handles `CMD_CITY_TOOL` with:
  - `toolType = 1` → road → `tile->setCityConductive(true)`
  - `toolType = 2` → power line
- `include/Tile.h` already has `cityConductive_` for roads / structures / power lines.
- `src/players/QuantBot.cpp` already places roads by dispatching `CMD_CITY_TOOL(..., 1)`.

So the missing piece is the **player-facing build flow**, not inventing a second road system.

## Task
Implement buildable **roads** only. Do **not** try to solve power lines, funding sliders, or city-budget integration in this task.

### 1. Add roads to the player build flow
Wire a Road item into the existing City build menu / construction UI so the player can select it intentionally, the same way other city features are exposed.

### 2. Placement behaviour
When the player places a road:
- dispatch the existing `CMD_CITY_TOOL` path with `toolType = 1`
- mark the tile conductive through the existing city sim command path
- reject invalid placements (occupied tiles, blocked terrain, duplicate road placement, etc.) using the engine’s normal placement rules where possible

### 3. Visuals
Use a simple placeholder visual if needed, but make the result readable on-map. This task is about functional road placement, not final art.

### 4. Tests
Add tests for the road placement path. At minimum verify:
- road placement issues the correct city tool command / effect
- placed road tiles become conductive
- invalid placement is rejected cleanly

### 5. Update tracking
Mark the relevant road feature complete in `FEATURES.md` once the player can place roads from the UI and tests pass.

## Build verification
```bash
cd ~/development/dunecity
cmake --build build -j$(sysctl -n hw.ncpu)
ctest --test-dir build --output-on-failure
```

## Git
- Commit message: conventional commits only
- Push your branch
- Open/update a PR against `svan058/dunecity` `main`

## Done signal
When done, post back to the DuneCity thread with:
- what you built
- what tests passed
- the PR number
