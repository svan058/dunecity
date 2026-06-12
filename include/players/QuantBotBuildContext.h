/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QUANTBOT_BUILD_CONTEXT_H
#define QUANTBOT_BUILD_CONTEXT_H

#include <set>
#include <data.h>

class House;
class BuilderBase;

/// Snapshot of AI state for one build tick. Replaces scattered ad-hoc checks.
struct QuantBotBuildContext {
    // Item counts (snapshot, already includes queued items)
    int itemCount[Num_ItemID];

    // Economy
    int money;
    int militaryValue;
    int militaryValueLimit;
    int harvesterLimit;
    int activeHeavyFactoryCount;
    int activeHighTechFactoryCount;
    int activeRepairYardCount;

    // Power
    int powerProduced;
    int powerRequired;

    // Mode flags
    bool isCitySim;
    bool isCampaign;
    bool isCustom;
    bool isBrutal;
    bool supportMode;
    int  techLevel;
    int  difficulty;

    // City simulation stats
    int ownResPop = 0, ownComPop = 0, ownIndPop = 0, ownTotalPop = 0;
    int ownAvgLandValue = 0;
    int16_t ownResValve = 0, ownComValve = 0, ownIndValve = 0;
    bool ownHasStadium = false, ownHasAirport = false;

    // Dedup tracking — items ordered this tick across multiple CYs
    std::set<Uint32> orderedThisTick;

    // Back-references (non-owning)
    const House* house = nullptr;
    int houseID = -1;

    /// Power delta: positive = surplus, negative = deficit
    int powerDelta() const { return powerProduced - powerRequired; }

    /// Has at least `surplus` excess power?
    bool hasPowerSurplus(int surplus = 0) const { return powerDelta() >= surplus; }

    /// Can the house afford this amount?
    bool canAfford(int amount) const { return money >= amount; }

    /// Has another CY already ordered this unique item this tick?
    bool alreadyOrdered(Uint32 itemID) const { return orderedThisTick.count(itemID) > 0; }

    /// Mark item as ordered (for dedup).
    void markOrdered(Uint32 itemID) { orderedThisTick.insert(itemID); }

    /// Is this item allowed to be built by multiple CYs in one tick?
    bool isMultiBuild(Uint32 itemID) const {
        return itemID == Structure_ZoneResidential
            || itemID == Structure_ZoneCommercial
            || itemID == Structure_ZoneIndustrial
            || itemID == Structure_RocketTurret
            || itemID == Structure_GunTurret
            || itemID == Structure_Wall
            || itemID == Structure_Slab1;
    }

    /// Combined city zone count
    int cityZoneCount() const {
        return itemCount[Structure_ZoneResidential]
             + itemCount[Structure_ZoneCommercial]
             + itemCount[Structure_ZoneIndustrial];
    }
};

#endif // QUANTBOT_BUILD_CONTEXT_H
