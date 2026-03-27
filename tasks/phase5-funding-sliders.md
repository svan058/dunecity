# Task: Funding allocation sliders (police, fire, transport)

## Feature
Add UI sliders to City Budget window to let players allocate city budget across Police, Fire, and Transport (road) categories. Show visual feedback when allocations sum >100%.

## Current State
- CityBudget already has: `roadPercent_`, `policePercent_`, `firePercent_` (0-100)
- CityBudgetWindow has tax rate controls
- Police/fire effects exist (`policeEffect_`, `fireEffect_`, `roadEffect_`) and are calculated in `collectTax()`

## Implementation Steps

### 1. Add a command to set budget allocation
In `Command.h`, add:
```cpp
CMD_CITY_SET_BUDGET,  // Already exists! (p1=road%, p2=police%, p3=fire%)
```

### 2. Add validation + slider UI to CityBudgetWindow
Modify `CityBudgetWindow.cpp`:
- Add 3 slider/button controls for Road %, Police %, Fire %
- Each has +/- buttons (like tax rate) to adjust by 5%
- Add "Total: X%" label that updates live
- If total > 100%, show warning in red and disable "Apply" button
- On valid apply, send CMD_CITY_SET_BUDGET

### 3. Hook up the sliders
- Add handler methods: `onRoadIncrease`, `onRoadDecrease`, etc.
- Track pending values before applying
- Show validation state in UI

### 4. Verify city effects
- Road % affects `roadEffect_` in budget calculation
- Police % affects `policeEffect_`
- Fire % affects `fireEffect_`
- These are already calculated in `CityBudget::collectTax()` and `updateFundEffects()`

## UI Layout
```
City Budget
Treasury: 1000
Tax Revenue: 50
Tax Rate: [+] 7% [-]

--- Allocation ---
Road:     [+] 33% [-]    (transport)
Police:   [+] 33% [-]
Fire:     [+] 34% [-]
Total:    100%

[ Apply ]  [ Cancel ]
[ Close ]
```

## Test Plan
1. Open City Budget (Shift+B) → verify sliders appear
2. Adjust sliders → verify Total % updates
3. Set total > 100% → verify warning appears
4. Apply allocation → verify city effects change (via debug or HUD)
5. Verify crime/fire/road indicators respond to allocation changes

## Dependencies
- Requires: "City Budget management screen" (Phase 5, already done)
- Next: Phase 6 (traffic overlay, road network)
