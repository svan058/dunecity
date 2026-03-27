# Task: Road network affecting zone growth rates

## Feature
Zones adjacent to roads should grow faster. Add road connectivity bonus to zone growth calculations.

## Current State
- TrafficSimulation already uses `isCityConductive()` (roads) for traffic routing
- ZoneSimulation calculates growth based on power, land value, pollution, traffic
- Need to add road proximity bonus

## Implementation Steps

### 1. Add road proximity helper
In ZoneSimulation.cpp, add method to check if zone is near roads:
```cpp
bool hasNearbyRoad(int x, int y) const;
```
Check 3x3 area around zone tile for conductive tiles.

### 2. Modify zone evaluation functions
Update evalRes, evalCom, evalInd to add road bonus:
- Residential near roads: +500 to score (faster growth)
- Commercial near roads: +300 to score  
- Industrial near roads: +400 to score (bonus for road access to silos)

### 3. Add road multiplier to growth
In doResIn, doComIn, doIndIn, add bonus growth rate when near roads:
- Normal: +1 population
- Near road: +2 population (25% bonus effect)

## Test Plan
1. Place zone with no roads nearby → slow growth
2. Place zone with road within 2 tiles → faster growth visible
3. Verify traffic density still affects zones (existing behavior)
4. Check that Industrial gets bonus when road leads to Refinery

## Dependencies
- Requires: Road placement tool (Phase 6, just completed)
