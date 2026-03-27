# Task: Disaster notification system

## Feature
Add a notification system for city disasters that displays event banners at the top of the screen with event type, description, duration, and affected zone count. Notifications stack and auto-dismiss.

## Implementation Steps

### 1. Create DisasterNotification widget
Create `include/GUI/dune/DisasterNotification.h` and `src/GUI/dune/DisasterNotification.cpp`:
- Widget that displays disaster notifications
- Supports: Fire, Sandstorm, Sandworm attack
- Fields: event type, message, duration, affected count
- Auto-dismiss timer
- Click to dismiss

### 2. Add to GameInterface
- Add DisasterNotification widget to GameInterface
- Position above City Stats HUD
- Support multiple stacked notifications

### 3. Add disaster trigger methods
In Game.h/Game.cpp, add:
```cpp
void triggerFire(int x, int y, int radius);
void triggerSandstorm(int x, int y, int radius);
void triggerSandworm(int x, int y);
```
Each triggers notification + actual disaster effect (placeholder for now).

### 4. Add test triggers
For testing, bind keys:
- F7: Trigger fire notification
- F8: Trigger sandstorm notification
- F9: Trigger sandworm notification

### 5. Notification display
- Icon: emoji-style or from game graphics
- Message: "Fire! X zones affected" / "Sandstorm approaching!" / "Sandworm spotted!"
- Duration: show countdown in seconds
- Position: top center, stack multiple notifications vertically

## Test Plan
1. Press F7/F8/F9 → notification appears
2. Multiple disasters → notifications stack
3. Click notification → dismisses
4. Duration expires → auto-dismiss
5. Verify notification appears above City Stats HUD

## Dependencies
- Future: Actual disaster effects (fire spread, sandstorm damage, sandworm attacks)
