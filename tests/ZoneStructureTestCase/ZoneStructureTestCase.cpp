/*
 *  ZoneStructureTestCase.cpp - Unit tests for Phase 3 zone structure integration
 *
 *  Validates: structure IDs, enum helpers, zone type constants, and build list ordering.
 */

#include <catch2/catch_all.hpp>
#include <data.h>
#include <dunecity/CityConstants.h>

// =============================================================================
// Structure ID Enum Tests
// =============================================================================

TEST_CASE("ZoneStructure: Structure IDs are correctly assigned", "[zone][ids]") {
    REQUIRE(Structure_ZoneResidential == 20);
    REQUIRE(Structure_ZoneCommercial == 21);
    REQUIRE(Structure_ZoneIndustrial == 22);
    REQUIRE(Structure_Road == 23);
    REQUIRE(Structure_PowerLine == 24);
}

TEST_CASE("ZoneStructure: Structure_LastID includes city structures", "[zone][ids]") {
    REQUIRE(Structure_LastID == 24);
    REQUIRE(Structure_PowerLine == Structure_LastID);
}

TEST_CASE("ZoneStructure: Unit IDs shifted above city structures", "[zone][ids]") {
    REQUIRE(Unit_FirstID == 25);
    REQUIRE(Unit_FirstID == Structure_LastID + 1);
    REQUIRE(Unit_Carryall == 25);
}

TEST_CASE("ZoneStructure: ItemID bounds are consistent", "[zone][ids]") {
    REQUIRE(ItemID_FirstID == 1);
    REQUIRE(ItemID_LastID == Unit_LastID);
    REQUIRE(Num_ItemID == ItemID_LastID + 1);
}

// =============================================================================
// isStructure / isUnit / isZoneStructure Helper Tests
// =============================================================================

TEST_CASE("ZoneStructure: isStructure returns true for zone structures", "[zone][helpers]") {
    REQUIRE(isStructure(Structure_ZoneResidential));
    REQUIRE(isStructure(Structure_ZoneCommercial));
    REQUIRE(isStructure(Structure_ZoneIndustrial));
}

TEST_CASE("ZoneStructure: isStructure returns true for all structures", "[zone][helpers]") {
    for (int id = Structure_FirstID; id <= Structure_LastID; id++) {
        REQUIRE(isStructure(id));
    }
}

TEST_CASE("ZoneStructure: isUnit returns false for zone structures", "[zone][helpers]") {
    REQUIRE_FALSE(isUnit(Structure_ZoneResidential));
    REQUIRE_FALSE(isUnit(Structure_ZoneCommercial));
    REQUIRE_FALSE(isUnit(Structure_ZoneIndustrial));
}

TEST_CASE("ZoneStructure: isUnit returns true for all units", "[zone][helpers]") {
    for (int id = Unit_FirstID; id <= Unit_LastID; id++) {
        REQUIRE(isUnit(id));
    }
}

TEST_CASE("ZoneStructure: isZoneStructure returns true only for zone types", "[zone][helpers]") {
    REQUIRE(isZoneStructure(Structure_ZoneResidential));
    REQUIRE(isZoneStructure(Structure_ZoneCommercial));
    REQUIRE(isZoneStructure(Structure_ZoneIndustrial));

    // City infrastructure (roads/power lines) are structures but not zones
    REQUIRE(isStructure(Structure_Road));
    REQUIRE(isStructure(Structure_PowerLine));
    REQUIRE_FALSE(isZoneStructure(Structure_Road));
    REQUIRE_FALSE(isZoneStructure(Structure_PowerLine));

    REQUIRE_FALSE(isZoneStructure(Structure_Barracks));
    REQUIRE_FALSE(isZoneStructure(Structure_ConstructionYard));
    REQUIRE_FALSE(isZoneStructure(Structure_WindTrap));
    REQUIRE_FALSE(isZoneStructure(Structure_Wall));
    REQUIRE_FALSE(isZoneStructure(Structure_WOR));
    REQUIRE_FALSE(isZoneStructure(Unit_Harvester));
    REQUIRE_FALSE(isZoneStructure(ItemID_Invalid));
}

TEST_CASE("ZoneStructure: isStructure and isUnit are mutually exclusive", "[zone][helpers]") {
    for (int id = ItemID_FirstID; id <= ItemID_LastID; id++) {
        bool both = isStructure(id) && isUnit(id);
        REQUIRE_FALSE(both);
    }
}

TEST_CASE("ZoneStructure: every valid item is either structure or unit", "[zone][helpers]") {
    for (int id = ItemID_FirstID; id <= ItemID_LastID; id++) {
        bool either = isStructure(id) || isUnit(id);
        REQUIRE(either);
    }
}

// =============================================================================
// CityConstants ZoneType Tests
// =============================================================================

TEST_CASE("ZoneStructure: ZoneType enum values match expected", "[zone][constants]") {
    REQUIRE(static_cast<uint8_t>(DuneCity::ZoneType::None) == 0);
    REQUIRE(static_cast<uint8_t>(DuneCity::ZoneType::Residential) == 1);
    REQUIRE(static_cast<uint8_t>(DuneCity::ZoneType::Commercial) == 2);
    REQUIRE(static_cast<uint8_t>(DuneCity::ZoneType::Industrial) == 3);
}

TEST_CASE("ZoneStructure: ZoneType values are distinct", "[zone][constants]") {
    REQUIRE(DuneCity::ZoneType::None != DuneCity::ZoneType::Residential);
    REQUIRE(DuneCity::ZoneType::Residential != DuneCity::ZoneType::Commercial);
    REQUIRE(DuneCity::ZoneType::Commercial != DuneCity::ZoneType::Industrial);
    REQUIRE(DuneCity::ZoneType::None != DuneCity::ZoneType::Industrial);
}

// =============================================================================
// ID Continuity Tests (no gaps between structures and units)
// =============================================================================

TEST_CASE("ZoneStructure: No ID gap between structures and units", "[zone][ids]") {
    REQUIRE(Unit_FirstID == Structure_LastID + 1);
}

TEST_CASE("ZoneStructure: Structure range is contiguous", "[zone][ids]") {
    int count = Structure_LastID - Structure_FirstID + 1;
    REQUIRE(count == 24);
}

TEST_CASE("ZoneStructure: Zone structures are at end of structure range", "[zone][ids]") {
    REQUIRE(Structure_ZoneResidential == Structure_WOR + 1);
    REQUIRE(Structure_ZoneCommercial == Structure_WOR + 2);
    REQUIRE(Structure_ZoneIndustrial == Structure_WOR + 3);
}

// =============================================================================
// isFlyingUnit / isInfantryUnit don't match zone structures
// =============================================================================

TEST_CASE("ZoneStructure: isFlyingUnit returns false for zone structures", "[zone][helpers]") {
    REQUIRE_FALSE(isFlyingUnit(Structure_ZoneResidential));
    REQUIRE_FALSE(isFlyingUnit(Structure_ZoneCommercial));
    REQUIRE_FALSE(isFlyingUnit(Structure_ZoneIndustrial));
}

TEST_CASE("ZoneStructure: isInfantryUnit returns false for zone structures", "[zone][helpers]") {
    REQUIRE_FALSE(isInfantryUnit(Structure_ZoneResidential));
    REQUIRE_FALSE(isInfantryUnit(Structure_ZoneCommercial));
    REQUIRE_FALSE(isInfantryUnit(Structure_ZoneIndustrial));
}
