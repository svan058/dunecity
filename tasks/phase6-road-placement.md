# Task: Road placement tool

## Feature
Add a dedicated road placement tool accessible from the city building toolbar. Click to place road tiles, click-and-drag for continuous placement, right-click to cancel. Cost deducted from city budget.

## Current State
- Roads (25 credits) can be built via Construction Yard (Phase 4)
- CMD_CITY_TOOL type 1 = road placement (already exists in backend)
- Zone placement uses 'Z' hotkey and CursorMode_CityZone

## Implementation Steps

### 1. Add road cursor mode
In `Game.h`, add to CursorMode enum:
```cpp
CursorMode_CityRoad,  // For direct road placement
```

### 2. Add road placement handler
In `Game.cpp`:
- Add handler method `handleCityRoadPlacementClick(int x, int y)`
- Route clicks in CursorMode_CityRoad to this handler
- Handle right-click to cancel (return to normal mode)

### 3. Add hotkey for road mode
In Game.cpp key handling, add:
- 'R' key to toggle CursorMode_CityRoad
- Escape to exit road mode

### 4. Add continuous drag placement
In mouse handling for CursorMode_CityRoad:
- Track mouse position while button held
- Place road on each tile change if different from last
- Check city budget before each placement

### 5. Hook up budget check
In handleCityRoadPlacementClick:
- Check citySim->spendCityFunds(25) before placing
- If fails, play error sound and return

### 6. Visual road rendering (if needed)
- Check if Tile.cpp has road rendering
- Roads should show connectivity to adjacent roads
- May use existing graphics or simple overlay

## Hotkey
- 'R' to toggle road placement mode
- Right-click to cancel
- While in road mode, left-click/drag places roads

## Test Plan
1. Press 'R' → cursor changes to road mode
2. Click on map → road placed, 25 credits deducted from city budget
3. Click and drag → continuous road placement
4. Right-click → exit road mode
5. Try to place road with 0 city funds → should fail with error sound
6. Verify roads show in power grid overlay (they're conductive)

## Dependencies
- Requires: "City-specific structures (roads)" (Phase 4, done)
- Next: "Road network affecting zone growth rates"
