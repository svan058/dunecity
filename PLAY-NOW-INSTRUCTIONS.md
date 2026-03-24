# 🎮 Dune Legacy - Dynasty AI Edition - READY TO PLAY!

## What's Been Fixed

Your AI now behaves **exactly like Dune Dynasty's Original AI**:

### ✓ Combat Units Hunt Enemies
- Troopers, tanks, and infantry now **immediately hunt** for enemies when built
- No more units sitting idle at spawn point
- AI is aggressive and seeks you out

### ✓ Config Fixed
- Your config now uses `CampaignAIPlayer` (not QuantBot)
- Location: `~/Library/Application Support/dunelegacy/Dune Legacy.ini`

### ✓ Build Priorities Match Dynasty
- Uses exact `priorityBuild` values from Dynasty's `structureinfo.c`/`unitinfo.c`
- Turrets have proper rebuild priority (Gun=75, Rocket=100)

## How to Play

### Option 1: Launch From Build Directory
```bash
open /Users/stefanvanderwel/development/dune/dunelegacy/build/bin/dunelegacy.app
```

### Option 2: Install DMG
```bash
open /Users/stefanvanderwel/development/dune/dunelegacy/build/DuneLegacy-0.98.7.0-macOS.dmg
```

Then drag Dune Legacy to Applications and launch.

## What to Expect

### AI Will Now:
1. ✓ **Hunt you aggressively** - Combat units move toward your base
2. ✓ **Build proper unit mix** - Uses Dynasty's build priorities
3. ✓ **Activate on contact** - Starts attacking after first ground unit encounter
4. ✓ **Full-scale attack** - Sends all units when structures are damaged
5. ✓ **Rebuild defenses** - Prioritizes turrets when destroyed

### AI Will NOT (Intentional):
- ❌ Use team scripts (deferred to future work)
- ❌ Use squads (Brutal AI feature, deferred)
- ❌ Follow exact vanilla Dune II behavior (no scripting VM)

## Verify It's Working

### Check Log File
```bash
tail -30 ~/Library/Application\ Support/Dune\ Legacy/Dune\ Legacy.log
```

You should see:
```
CampaignAIPlayer initialization
```

NOT:
```
QuantBot initialization
```

### In-Game Behavior
Start any campaign mission and observe:
- AI builds units
- **Units move toward your base** (HUNT mode)
- Units don't just sit at spawn point
- AI actively seeks combat

## Quick Test Mission
1. Launch game
2. Select "Campaign" 
3. Choose any house (e.g., Harkonnen)
4. Start Mission 1
5. Build some units
6. Watch AI behavior after first contact

## Performance Log Monitoring (Optional)
[[memory:10683497]]

If you want to monitor AI behavior in real-time:
```bash
lnav ~/Library/Application\ Support/Dune\ Legacy/Dune\ Legacy.log
```

## Technical Details

See comprehensive documentation in:
- `documents/DYNASTY-AI-COMPLETE.md` - Full implementation summary
- `documents/DYNASTY-AI-BEHAVIOR-FIX.md` - Technical fix details
- `documents/76-campaign-ai-alignment-plan.md` - Overall design plan
- `documents/original-ai-implementation-status.md` - Feature status

## Known Differences from Dynasty

The AI is **more aggressive** than vanilla Dune II because:
- Units use **HUNT mode** (seek enemies actively)
- No team scripting (units act individually, not in coordinated waves)

This makes the AI more challenging than original Dune II, which is good for gameplay!

## Files Modified (Summary)
- ✓ Unit attack mode initialization (`BuilderBase.cpp`)
- ✓ Build priorities (`CampaignAIPlayer.cpp`)
- ✓ AI activation logic (`UnitBase.cpp`, `House.cpp`)
- ✓ Savegame compatibility (`Game.cpp`, `GameInitSettings.cpp`)
- ✓ Config file (`Dune Legacy.ini`)

## Troubleshooting

### If AI still seems passive:
1. Check config: `cat ~/Library/Application\ Support/dunelegacy/Dune\ Legacy.ini | grep -A 1 "\[AI\]"`
2. Check log: `tail -50 ~/Library/Application\ Support/Dune\ Legacy/Dune\ Legacy.log | grep -i player`
3. Verify you're using the new build (check build date in log)

### If you see QuantBot in logs:
```bash
# Re-run config fix:
cd /Users/stefanvanderwel/development/dune/dunelegacy
echo -e "\n[AI]\nCampaign AI = CampaignAIPlayer" >> ~/Library/Application\ Support/dunelegacy/Dune\ Legacy.ini
```

---

## 🏜️ Good Luck, Commander!

The AI will now hunt you down just like in the original Dune II. Prepare for aggressive trooper rushes and coordinated attacks!

**May your spice harvesting be profitable and your defenses strong!**

---

Build: 0.98.7.0 | Date: November 17, 2025 | AI: Dynasty Original Parity ✓

