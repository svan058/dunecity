# Task: City-specific structures (roads, power lines)

## Feature
Add roads and power lines as buildable city infrastructure via the Construction Yard menu. These are 1x1 tile "structures" that, when placed, enable city connectivity:
- **Roads** (tool type 1): Set tile as conductive for power grid + affect traffic
- **Power Lines** (tool type 2): Set tile as conductive for power grid (no traffic effect)

## Current State
- Backend EXISTS: `CMD_CITY_TOOL` already handles tool types 1 (road) and 2 (power line)
- Backend sets `tile->setCityConductive(true)` for both
- MISSING: No way to trigger these from the UI — not in Construction Yard build list

## Implementation Steps

### 1. Add item IDs (include/data.h)
Add after Structure_ZoneIndustrial (22):
```cpp
Structure_Road = 23,        ///< DuneCity: Road tile (25 credits)
Structure_PowerLine = 24,   ///< DuneCity: Power line tile (15 credits)
Structure_LastID = 24,
```
Update ItemID_LastID accordingly.

### 2. Add to BuilderBase itemOrder (structures/BuilderBase.cpp)
Add to the list (after zones):
```cpp
Structure_Road, Structure_PowerLine,
```
These go to Construction Yard (the city builder).

### 3. Add object data entries
In appropriate INI/JSON config, add:
- Structure_Road: price=25, size=1x1
- Structure_PowerLine: price=15, size=1x1

### 4. Create road/powerline structure classes
Create new files or add to ZoneStructure.cpp:
- `RoadStructure`: Places with CMD_CITY_TOOL(x, y, 1), sets cityConductive
- `PowerLineStructure`: Places with CMD_CITY_TOOL(x, y, 2), sets cityConductive

OR modify the existing placement flow to intercept these itemIDs and call CMD_CITY_TOOL instead of placing a real structure object.

### 5. Placement handling
When player clicks "Road" or "PowerLine" in builder list:
- Enter placement mode (like zones)
- On map click → send CMD_CITY_TOOL(x, y, 1 or 2)
- Don't create a permanent structure object (roads/lines are terrain modifiers)

### 6. UI/Visuals
- Use existing graphics as placeholders (e.g., Wall sprite for road, maybe a different one for power line)
- Future: custom sprites for road grid / power line grid

## Test Plan
1. Open Construction Yard → verify Road and PowerLine appear in build list with prices
2. Build Road → place on map → verify tile shows as connected in power grid overlay (Shift+2)
3. Build PowerLine → verify same behavior
4. Place zones near roads → verify zones get power when connected to Wind Trap

## Dependencies
- None (backend already exists)
- Blocks: "Build cost integration with city budget" (Phase 4, next item)
