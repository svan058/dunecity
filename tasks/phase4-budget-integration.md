# Task: Build cost integration with city budget

## Feature
When building roads/power lines via Construction Yard, costs are deducted from city budget (not player credits). If insufficient, block construction and notify player.

## Current State
- Roads (25 credits) and Power Lines (15 credits) can be built via Construction Yard
- Cost currently deducted from player credits (via standard building flow)
- City budget (`totalFunds_`) exists but not used for construction

## Implementation Steps

### 1. Add funds deduction method to CitySimulation
In `CitySimulation.h`, add:
```cpp
bool spendCityFunds(int amount);  // returns true if sufficient funds
```
Implementation checks `totalFunds_ >= amount`, deducts if true.

### 2. Modify ConstructionYard::doPlaceStructure
Update the road/powerline handling to:
1. Get price from objectData
2. Check city simulation has sufficient funds via `spendCityFunds(price)`
3. If insufficient: show message "Insufficient city funds" and return false
4. If sufficient: deduct from city funds, then proceed with CMD_CITY_TOOL

### 3. Add notification for insufficient funds
Use existing NewsTicker/MessageTicker system to show "Insufficient city funds for [Road/Power Line]"

### 4. Update HUD (optional, if needed)
City stats HUD already shows Treasury — verify it updates after construction

## Test Plan
1. Start game with 0 city funds → try to build road → should fail with message
2. Add city funds (via console/cheat) → build road → verify treasury decreases
3. Build power line → verify treasury decreases by 15
4. Verify road/power line still places correctly on map

## Dependencies
- Requires: "City-specific structures (roads, power lines)" (just completed)
