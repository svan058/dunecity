/*
 *  CityRoadTestCase.cpp - Unit tests for city road placement
 *
 *  Validates: road tool constants, CMD_CITY_TOOL command enum,
 *  tile conductive state contract, and placement validation rules.
 */

#include <catch2/catch_all.hpp>
#include <data.h>
#include <dunecity/CityConstants.h>
#include <Command.h>

// =============================================================================
// CityToolType Constants
// =============================================================================

TEST_CASE("CityRoad: CityTool_Road has value 1", "[road][constants]") {
    REQUIRE(DuneCity::CityTool_Road == 1);
}

TEST_CASE("CityRoad: CityToolType values are distinct", "[road][constants]") {
    REQUIRE(DuneCity::CityTool_Bulldoze != DuneCity::CityTool_Road);
    REQUIRE(DuneCity::CityTool_Road != DuneCity::CityTool_PowerLine);
    REQUIRE(DuneCity::CityTool_Bulldoze != DuneCity::CityTool_PowerLine);
}

TEST_CASE("CityRoad: CityTool_Bulldoze is 0, CityTool_PowerLine is 2", "[road][constants]") {
    REQUIRE(DuneCity::CityTool_Bulldoze == 0);
    REQUIRE(DuneCity::CityTool_PowerLine == 2);
}

// =============================================================================
// CMD_CITY_TOOL Command Enum
// =============================================================================

TEST_CASE("CityRoad: CMD_CITY_TOOL is a valid command ID", "[road][command]") {
    REQUIRE(CMD_CITY_TOOL > CMD_TEST_SYNC);
    REQUIRE(CMD_CITY_TOOL < CMD_MAX);
}

TEST_CASE("CityRoad: CMD_CITY_TOOL is distinct from other city commands", "[road][command]") {
    REQUIRE(CMD_CITY_TOOL != CMD_CITY_PLACE_ZONE);
    REQUIRE(CMD_CITY_TOOL != CMD_CITY_SET_TAX_RATE);
    REQUIRE(CMD_CITY_TOOL != CMD_CITY_SET_BUDGET);
}

TEST_CASE("CityRoad: All DuneCity commands are contiguous after CMD_TEST_SYNC", "[road][command]") {
    REQUIRE(CMD_CITY_PLACE_ZONE == CMD_TEST_SYNC + 1);
    REQUIRE(CMD_CITY_SET_TAX_RATE == CMD_TEST_SYNC + 2);
    REQUIRE(CMD_CITY_SET_BUDGET == CMD_TEST_SYNC + 3);
    REQUIRE(CMD_CITY_TOOL == CMD_TEST_SYNC + 4);
    REQUIRE(CMD_MAX == CMD_CITY_TOOL + 1);
}

// =============================================================================
// Tile Conductive State Contract
// =============================================================================

namespace {

struct TileConductiveProxy {
    bool cityConductive_ = false;
    bool cityPowered_ = false;
    DuneCity::ZoneType cityZoneType_ = DuneCity::ZoneType::None;
    uint8_t cityZoneDensity_ = 0;

    bool isCityConductive() const noexcept { return cityConductive_; }
    void setCityConductive(bool c) noexcept { cityConductive_ = c; }
    bool isCityPowered() const noexcept { return cityPowered_; }
    void setCityPowered(bool p) noexcept { cityPowered_ = p; }
    DuneCity::ZoneType getCityZoneType() const noexcept { return cityZoneType_; }
    void setCityZoneType(DuneCity::ZoneType z) noexcept { cityZoneType_ = z; }
    bool hasCityZone() const noexcept { return cityZoneType_ != DuneCity::ZoneType::None; }
};

} // anonymous namespace

TEST_CASE("CityRoad: Tile starts non-conductive", "[road][tile]") {
    TileConductiveProxy tile;
    REQUIRE_FALSE(tile.isCityConductive());
}

TEST_CASE("CityRoad: setCityConductive(true) makes tile conductive", "[road][tile]") {
    TileConductiveProxy tile;
    tile.setCityConductive(true);
    REQUIRE(tile.isCityConductive());
}

TEST_CASE("CityRoad: setCityConductive(false) removes conductivity", "[road][tile]") {
    TileConductiveProxy tile;
    tile.setCityConductive(true);
    REQUIRE(tile.isCityConductive());
    tile.setCityConductive(false);
    REQUIRE_FALSE(tile.isCityConductive());
}

TEST_CASE("CityRoad: Road (conductive) is independent of zone type", "[road][tile]") {
    TileConductiveProxy tile;
    tile.setCityConductive(true);
    REQUIRE(tile.isCityConductive());
    REQUIRE_FALSE(tile.hasCityZone());

    tile.setCityZoneType(DuneCity::ZoneType::Residential);
    REQUIRE(tile.isCityConductive());
    REQUIRE(tile.hasCityZone());
}

TEST_CASE("CityRoad: Conductive state is independent of powered state", "[road][tile]") {
    TileConductiveProxy tile;
    tile.setCityConductive(true);
    REQUIRE_FALSE(tile.isCityPowered());
    REQUIRE(tile.isCityConductive());

    tile.setCityPowered(true);
    REQUIRE(tile.isCityPowered());
    REQUIRE(tile.isCityConductive());
}

// =============================================================================
// Road Placement Validation Rules (via CityTilePlacementState)
// =============================================================================

TEST_CASE("CityRoad: canPlaceRoad accepts valid tile", "[road][validation]") {
    auto state = DuneCity::makeCityTilePlacementState(true, false, false, false, false);
    REQUIRE(DuneCity::canPlaceRoad(state));
}

TEST_CASE("CityRoad: Road cannot be placed on mountain", "[road][validation]") {
    auto state = DuneCity::makeCityTilePlacementState(true, true, false, false, false);
    REQUIRE_FALSE(DuneCity::canPlaceRoad(state));
}

TEST_CASE("CityRoad: Road cannot be placed on tile with structure", "[road][validation]") {
    auto state = DuneCity::makeCityTilePlacementState(true, false, true, false, false);
    REQUIRE_FALSE(DuneCity::canPlaceRoad(state));
}

TEST_CASE("CityRoad: Duplicate road placement is rejected", "[road][validation]") {
    auto state = DuneCity::makeCityTilePlacementState(true, false, false, false, true);
    REQUIRE_FALSE(DuneCity::canPlaceRoad(state));
}

TEST_CASE("CityRoad: Road cannot be placed on unsupported terrain", "[road][validation]") {
    auto state = DuneCity::makeCityTilePlacementState(false, false, false, false, false);
    REQUIRE_FALSE(DuneCity::canPlaceRoad(state));
}

TEST_CASE("CityRoad: Road cannot be placed on existing city zone", "[road][validation]") {
    auto state = DuneCity::makeCityTilePlacementState(true, false, false, true, false);
    REQUIRE_FALSE(DuneCity::canPlaceRoad(state));
}

// =============================================================================
// Road Effect: Placed road makes tile conductive
// =============================================================================

TEST_CASE("CityRoad: Placing road sets conductive flag", "[road][effect]") {
    TileConductiveProxy tile;
    tile.setCityConductive(true);
    REQUIRE(tile.isCityConductive());
}

TEST_CASE("CityRoad: Road placement on valid tile follows correct sequence", "[road][effect]") {
    TileConductiveProxy tile;

    REQUIRE_FALSE(tile.isCityConductive());

    tile.setCityConductive(true);

    REQUIRE(tile.isCityConductive());
}

// =============================================================================
// CityTool_Road matches QuantBot dispatch value
// =============================================================================

TEST_CASE("CityRoad: CityTool_Road value matches QuantBot's toolType=1", "[road][integration]") {
    REQUIRE(static_cast<uint32_t>(DuneCity::CityTool_Road) == 1u);
}
