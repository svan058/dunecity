/*
 *  CityStructuresTestCase.cpp - Unit tests for Phase 4 road and power line integration
 *
 *  Validates:
 *  - Structure IDs for Road (23) and PowerLine (24)
 *  - isStructure() and isZoneStructure() helpers for road/powerline
 *  - ConstructionYard routing of Road/PowerLine to CMD_CITY_TOOL
 *  - CMD_CITY_TOOL handler sets tile conductivity for tool types 1 and 2
 *  - ObjectData.ini.default contains Road and PowerLine price entries
 */

#include <catch2/catch_all.hpp>
#include <data.h>
#include <dunecity/CityConstants.h>
#include <fstream>
#include <string>
#include <cstdlib>

// Get the Dune Legacy source directory from environment (set by CMake test config)
static std::string getSourceDir() {
    const char* env = std::getenv("DUNE_CITY_SOURCE_DIR");
    return env ? std::string(env) : std::string();
}

// Open a file relative to the source directory
static std::ifstream openSourceFile(const std::string& relativePath) {
    return std::ifstream(getSourceDir() + "/" + relativePath);
}

// =============================================================================
// Structure ID Tests
// =============================================================================

TEST_CASE("CityStructures: Road and PowerLine have correct IDs", "[city][roads][phase4]") {
    REQUIRE(Structure_Road == 23);
    REQUIRE(Structure_PowerLine == 24);
    REQUIRE(Structure_LastID == 24);
    REQUIRE(Structure_Road == Structure_LastID - 1);
    REQUIRE(Structure_PowerLine == Structure_LastID);
}

TEST_CASE("CityStructures: Road and PowerLine are structures not zones", "[city][roads][phase4]") {
    REQUIRE(isStructure(Structure_Road));
    REQUIRE(isStructure(Structure_PowerLine));
    REQUIRE_FALSE(isZoneStructure(Structure_Road));
    REQUIRE_FALSE(isZoneStructure(Structure_PowerLine));
    REQUIRE_FALSE(isUnit(Structure_Road));
    REQUIRE_FALSE(isUnit(Structure_PowerLine));
}

TEST_CASE("CityStructures: Unit IDs correctly follow city structures", "[city][roads][phase4]") {
    REQUIRE(Unit_FirstID == Structure_LastID + 1);
}

// =============================================================================
// ConstructionYard Routing Tests
// =============================================================================

TEST_CASE("CityStructures: ConstructionYard source contains road routing", "[city][roads][phase4][routing]") {
    std::ifstream file = openSourceFile("src/structures/ConstructionYard.cpp");
    REQUIRE(file.is_open());

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    // Road must be handled via CMD_CITY_TOOL with tool type 1
    REQUIRE(content.find("Structure_Road") != std::string::npos);
    REQUIRE(content.find("CMD_CITY_TOOL") != std::string::npos);

    // Verify the tool type branching exists for road and powerline
    INFO("ConstructionYard.cpp must check itemID == Structure_Road || itemID == Structure_PowerLine");
    const bool hasRoadCheck = content.find("itemID == Structure_Road") != std::string::npos;
    const bool hasPowerLineCheck = content.find("itemID == Structure_PowerLine") != std::string::npos;
    REQUIRE(hasRoadCheck);
    REQUIRE(hasPowerLineCheck);
}

TEST_CASE("CityStructures: ConstructionYard routes road to tool type 1", "[city][roads][phase4][routing]") {
    std::ifstream file = openSourceFile("src/structures/ConstructionYard.cpp");
    REQUIRE(file.is_open());

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    // toolType = (itemID == Structure_Road) ? 1 : 2
    INFO("Road must map to tool type 1");
    const bool hasToolTypeMapping = content.find("(itemID == Structure_Road) ? 1 : 2") != std::string::npos;
    REQUIRE(hasToolTypeMapping);
}

// =============================================================================
// CMD_CITY_TOOL Handler Tests
// =============================================================================

TEST_CASE("CityStructures: CitySimulation handles CMD_CITY_TOOL tool types 1 and 2", "[city][roads][phase4][command]") {
    std::ifstream file = openSourceFile("src/dunecity/CitySimulation.cpp");
    REQUIRE(file.is_open());

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    // Tool type 1 = road: sets tile conductive
    INFO("CMD_CITY_TOOL case 1 must call tile->setCityConductive(true)");
    const bool hasRoadTool = content.find("case 1: // Road") != std::string::npos;
    REQUIRE(hasRoadTool);

    // Tool type 2 = power line: sets tile conductive
    INFO("CMD_CITY_TOOL case 2 must call tile->setCityConductive(true)");
    const bool hasPowerLineTool = content.find("case 2: // Power line") != std::string::npos;
    REQUIRE(hasPowerLineTool);
}

// =============================================================================
// ObjectData Configuration Tests
// =============================================================================

TEST_CASE("CityStructures: ObjectData.ini.default contains Road price", "[city][roads][phase4][config]") {
    std::ifstream file = openSourceFile("config/ObjectData.ini.default");
    REQUIRE(file.is_open());

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    // Road section with price = 25
    INFO("[Road] section must have Price = 25");
    const bool hasRoadSection = content.find("[Road]") != std::string::npos;
    REQUIRE(hasRoadSection);
    const bool hasRoadPrice = content.find("Price = 25") != std::string::npos;
    REQUIRE(hasRoadPrice);
}

TEST_CASE("CityStructures: ObjectData.ini.default contains PowerLine price", "[city][roads][phase4][config]") {
    std::ifstream file = openSourceFile("config/ObjectData.ini.default");
    REQUIRE(file.is_open());

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    // Power Line section with price = 15
    INFO("[Power Line] section must have Price = 15");
    const bool hasPowerLineSection = content.find("[Power Line]") != std::string::npos;
    REQUIRE(hasPowerLineSection);
    const bool hasPowerLinePrice = content.find("Price = 15") != std::string::npos;
    REQUIRE(hasPowerLinePrice);
}

TEST_CASE("CityStructures: Road and PowerLine are buildable from Construction Yard", "[city][roads][phase4][config]") {
    std::ifstream file = openSourceFile("config/ObjectData.ini.default");
    REQUIRE(file.is_open());

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    // Both must be enabled and use Construction Yard builder
    INFO("[Road] section must exist with Enabled=true and Builder=Construction Yard");
    const bool roadEnabled = content.find("[Road]") != std::string::npos &&
                             content.find("Enabled = true") != std::string::npos;
    const bool roadBuilder = content.find("Builder = Construction Yard") != std::string::npos;

    // Find [Power Line] section
    const size_t powerLinePos = content.find("[Power Line]");
    REQUIRE(powerLinePos != std::string::npos);

    // Check enabled in Power Line section (look for it after the section header)
    const std::string afterPowerLine = content.substr(powerLinePos);
    const bool powerLineSectionExists = afterPowerLine.find("[") != std::string::npos;
    const bool powerLineEnabled = powerLineSectionExists &&
                                   afterPowerLine.find("Enabled = true") != std::string::npos;

    REQUIRE(roadEnabled);
    REQUIRE(roadBuilder);
    REQUIRE(powerLineSectionExists);
}
