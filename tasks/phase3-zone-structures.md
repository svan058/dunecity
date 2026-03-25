# DuneCity Phase 3: Zone Structures via Construction Yard

## Objective

R/C/I zones should be buildable structures that integrate with the existing Construction Yard build pipeline — not just tile flags. Each zone type costs 100 spice credits, can be placed on sand (not just rock/slab), and behaves like any other Dune structure.

## Background

Current state:
- Zones are placed by pressing `Z` + `1/2/3` + clicking tiles directly (freeform placement, no credit cost, no build queue)
- Zone data lives on `Tile` via `cityZoneType_` / `cityZoneDensity_` fields
- `CMD_CITY_PLACE_ZONE` command exists in `Command.h` and is dispatched via `handleCityZonePlacementClick()` in `Game.cpp`
- `CitySimulation::advancePhase()` ticks city sim and reads zone data from tiles
- Structure IDs currently max at `Structure_LastID = 19` (Structure_WOR), Unit IDs start at 20

## Task

### 1. Add Three New Structure IDs

In `include/data.h`, add after `Structure_WOR = 19`:

```cpp
Structure_ZoneResidential = 20,  ///< DuneCity: Residential zone building (100 credits)
Structure_ZoneCommercial  = 21,  ///< DuneCity: Commercial zone building (100 credits)
Structure_ZoneIndustrial  = 22,  ///< DuneCity: Industrial zone building (100 credits)
Structure_LastID = 22,           // update this
```

Then shift Unit IDs up by 3 (Unit_FirstID = 23, etc.) and update `ItemID_LastID` accordingly. Search all usages of the old numeric values to catch any hardcoded constants.

### 2. Create Structure Classes

Create `include/structures/ZoneStructure.h` and `src/structures/ZoneStructure.cpp`:

- Base class `ZoneStructure : public StructureBase`
- Three subclasses: `ResidentialZone`, `CommercialZone`, `IndustrialZone`
- `structureSize`: 2×2 (like ConstructionYard)
- No special graphics yet — reuse an existing structure graphic as placeholder (e.g. `ObjPic_Slab1` or `ObjPic_Wall`). Add a TODO comment.
- `canBeBuiltOnSand()` → return `true` (override whatever base class restriction applies)
- On placement (override `ObjectBase::handleActionClick` or similar) → call `CMD_CITY_PLACE_ZONE` to register the tile zone with CitySimulation
- On destruction → clear the zone from tiles it occupied

Placement validation: zones can be placed on sand OR slab/rock tiles. Check `House::placeStructure` flow and `StructureBase::canBePlacedAt` to understand how to allow sand placement.

### 3. Wire into ObjectData / ObjectManager

Look at how other structures register themselves (e.g. `src/ObjectData.cpp`, `src/ObjectManager.cpp`). Register the three zone structures with:
- `price = 100` (spice credits)
- `hitpoints`: match WindTrap or Silo level (sturdy but not a military structure)
- No special weapons or capabilities

### 4. Add to Construction Yard Build List

In `src/structures/ConstructionYard.cpp` (or wherever the build list is populated — check `BuilderBase::updateBuildList`), add the three zone structures to the available items. They should be available from tech level 1.

### 5. Visual Placeholder

For now, render zone structures using an existing structure sprite as placeholder. Pick something visually distinct per zone type if possible. Add `// TODO: DuneCity zone-specific sprites` comments.

When the structure is placed, the tile overlay (colored R/C/I overlay from Phase 2) should still render on top of the structure graphic.

### 6. Remove Old Freeform Zone Placement

The `Z` hotkey zone placement mode (`CursorMode_CityZone`, `handleCityZonePlacementClick`) was a temporary hack. Remove it:
- Remove `CursorMode_CityZone` from the `CursorMode` enum in `Game.h`
- Remove the `Z` key handler and zone placement click handler from `Game.cpp`
- Remove `handleCityZonePlacementClick` declaration from `Game.h` and implementation from `Game.cpp`
- Remove `selectedZoneType_` member
- Keep `CMD_CITY_PLACE_ZONE` — it's still used to notify CitySimulation when a zone structure is placed

Update `FEATURES.md` keybind table to remove the Z/1/2/3 entries.

### 7. Update FEATURES.md

Mark Phase 3 (City Building Menu) as DONE once these structures appear in the Construction Yard. Add a note about sand placement support.

---

## Build Verification

```bash
cd ~/development/dunecity
cmake -B build \
  -DCMAKE_PREFIX_PATH=~/dune/dunelegacy/build/vcpkg_installed/arm64-osx \
  -DCMAKE_BUILD_TYPE=Release \
  -DVCPKG_MANIFEST_INSTALL=OFF
cmake --build build -j$(sysctl -n hw.ncpu) 2>&1 | tail -20
```

Must compile clean (macOS arm64 SDK version warnings from vcpkg are acceptable, actual errors are not).

---

## Git

Branch: `feature/phase3-zone-structures`  
Branch from: `main` (currently at `71dd499`)  
Commit messages: conventional commits (`feat:`, `fix:`, `chore:`)  
Push to origin when done.  
Open a PR to `svan058/dunecity` targeting `main`.

## Done Signal

When completely finished (build passes, PR opened), run:
```
openclaw system event --text "Done: Phase 3 zone structures complete — R/C/I buildable via Construction Yard, 100 credits each, sand placement enabled, PR opened" --mode now
```
