/*
 *  MilestoneTestCase.cpp - Unit tests for city milestone notification system
 *
 *  Validates:
 *  - Milestones start unachieved on a fresh CitySimulation
 *  - Population milestones trigger at correct thresholds (100, 500, 1000, 5000)
 *  - Economy milestones trigger at correct fund thresholds (1000, 5000)
 *  - All-zones milestone triggers when R+C+I zones exist and roads present
 *  - Already-achieved milestones do not re-trigger on subsequent calls
 *  - hasMilestoneBeenAchieved() reflects correct state
 *  - firstZoneMilestoneTriggered_ is correctly set and reflected
 */

#include <catch2/catch_all.hpp>
#include <data.h>
#include <dunecity/CitySimulation.h>

// =============================================================================
// Fresh city: all milestones start unachieved
// =============================================================================

TEST_CASE("Milestones: fresh city has no milestones achieved", "[milestone][init]") {
    DuneCity::CitySimulation sim;

    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_100) == false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_500) == false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_1000) == false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_5000) == false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_FIRST_ZONE) == false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ECONOMY_1000) == false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ECONOMY_5000) == false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ALL_ZONES) == false);
}

TEST_CASE("Milestones: firstZoneMilestoneTriggered starts false", "[milestone][init]") {
    DuneCity::CitySimulation sim;
    CHECK(sim.wasFirstZoneMilestoneTriggered() == false);
}

// =============================================================================
// Population milestones
// =============================================================================

TEST_CASE("Milestones: population 100 triggers correctly", "[milestone][population]") {
    DuneCity::CitySimulation sim;

    sim.checkAndTriggerMilestones(/*currentPop*/ 99, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_100) == false);

    sim.checkAndTriggerMilestones(/*currentPop*/ 100, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_100) == true);
}

TEST_CASE("Milestones: population 500 triggers correctly", "[milestone][population]") {
    DuneCity::CitySimulation sim;

    sim.checkAndTriggerMilestones(/*currentPop*/ 499, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_500) == false);

    sim.checkAndTriggerMilestones(/*currentPop*/ 500, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_500) == true);
}

TEST_CASE("Milestones: population 1000 triggers correctly", "[milestone][population]") {
    DuneCity::CitySimulation sim;

    sim.checkAndTriggerMilestones(/*currentPop*/ 999, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_1000) == false);

    sim.checkAndTriggerMilestones(/*currentPop*/ 1000, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_1000) == true);
}

TEST_CASE("Milestones: population 5000 triggers correctly", "[milestone][population]") {
    DuneCity::CitySimulation sim;

    sim.checkAndTriggerMilestones(/*currentPop*/ 4999, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_5000) == false);

    sim.checkAndTriggerMilestones(/*currentPop*/ 5000, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_5000) == true);
}

// =============================================================================
// Population milestones: no duplicate triggers
// =============================================================================

TEST_CASE("Milestones: population milestone does not re-trigger on repeated calls", "[milestone][population][idempotent]") {
    DuneCity::CitySimulation sim;

    // Cross threshold once
    sim.checkAndTriggerMilestones(/*currentPop*/ 100, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_100) == true);

    // Call again at same or higher population — must stay achieved (no re-trigger)
    sim.checkAndTriggerMilestones(/*currentPop*/ 200, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_100) == true);

    sim.checkAndTriggerMilestones(/*currentPop*/ 9999, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_100) == true);
}

// =============================================================================
// Economy milestones
// =============================================================================

TEST_CASE("Milestones: economy 1000 credits triggers correctly", "[milestone][economy]") {
    DuneCity::CitySimulation sim;

    sim.checkAndTriggerMilestones(/*currentPop*/ 0, /*currentFunds*/ 999, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ECONOMY_1000) == false);

    sim.checkAndTriggerMilestones(/*currentPop*/ 0, /*currentFunds*/ 1000, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ECONOMY_1000) == true);
}

TEST_CASE("Milestones: economy 5000 credits triggers correctly", "[milestone][economy]") {
    DuneCity::CitySimulation sim;

    sim.checkAndTriggerMilestones(/*currentPop*/ 0, /*currentFunds*/ 4999, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ECONOMY_5000) == false);

    sim.checkAndTriggerMilestones(/*currentPop*/ 0, /*currentFunds*/ 5000, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ECONOMY_5000) == true);
}

TEST_CASE("Milestones: economy milestone does not re-trigger on repeated calls", "[milestone][economy][idempotent]") {
    DuneCity::CitySimulation sim;

    sim.checkAndTriggerMilestones(/*currentPop*/ 0, /*currentFunds*/ 1000, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ECONOMY_1000) == true);

    sim.checkAndTriggerMilestones(/*currentPop*/ 0, /*currentFunds*/ 9999, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ECONOMY_1000) == true);
}

// =============================================================================
// First-zone milestone
// =============================================================================

TEST_CASE("Milestones: first zone milestone triggers when flag is set", "[milestone][first-zone]") {
    DuneCity::CitySimulation sim;

    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_FIRST_ZONE) == false);

    sim.onFirstZoneBuilt();
    sim.checkAndTriggerMilestones(/*currentPop*/ 0, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_FIRST_ZONE) == true);
}

TEST_CASE("Milestones: first zone milestone does not trigger without flag", "[milestone][first-zone]") {
    DuneCity::CitySimulation sim;

    sim.checkAndTriggerMilestones(/*currentPop*/ 0, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_FIRST_ZONE) == false);
}

TEST_CASE("Milestones: first zone milestone does not re-trigger", "[milestone][first-zone][idempotent]") {
    DuneCity::CitySimulation sim;

    sim.onFirstZoneBuilt();
    sim.checkAndTriggerMilestones(/*currentPop*/ 0, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_FIRST_ZONE) == true);

    // Call again — stays achieved
    sim.checkAndTriggerMilestones(/*currentPop*/ 999, /*currentFunds*/ 9999, /*res*/ 99, /*com*/ 50, /*ind*/ 50, /*hasRoads*/ true);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_FIRST_ZONE) == true);
}

// =============================================================================
// All-zones milestone
// =============================================================================

TEST_CASE("Milestones: all-zones requires all three zone types plus roads", "[milestone][all-zones]") {
    DuneCity::CitySimulation sim;

    // Only R zones — no milestone
    sim.checkAndTriggerMilestones(/*currentPop*/ 100, /*currentFunds*/ 0, /*res*/ 10, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ true);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ALL_ZONES) == false);

    // R + C zones — no milestone
    sim.checkAndTriggerMilestones(/*currentPop*/ 100, /*currentFunds*/ 0, /*res*/ 10, /*com*/ 5, /*ind*/ 0, /*hasRoads*/ true);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ALL_ZONES) == false);

    // R + C + I zones, no roads — no milestone
    sim.checkAndTriggerMilestones(/*currentPop*/ 100, /*currentFunds*/ 0, /*res*/ 10, /*com*/ 5, /*ind*/ 3, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ALL_ZONES) == false);

    // R + C + I zones + roads — milestone achieved!
    sim.checkAndTriggerMilestones(/*currentPop*/ 100, /*currentFunds*/ 0, /*res*/ 10, /*com*/ 5, /*ind*/ 3, /*hasRoads*/ true);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ALL_ZONES) == true);
}

TEST_CASE("Milestones: all-zones milestone does not re-trigger", "[milestone][all-zones][idempotent]") {
    DuneCity::CitySimulation sim;

    sim.checkAndTriggerMilestones(/*currentPop*/ 100, /*currentFunds*/ 0, /*res*/ 10, /*com*/ 5, /*ind*/ 3, /*hasRoads*/ true);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ALL_ZONES) == true);

    sim.checkAndTriggerMilestones(/*currentPop*/ 9999, /*currentFunds*/ 9999, /*res*/ 99, /*com*/ 99, /*ind*/ 99, /*hasRoads*/ true);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ALL_ZONES) == true);
}

// =============================================================================
// Multiple milestones: order independence
// =============================================================================

TEST_CASE("Milestones: multiple milestones can be achieved independently", "[milestone][combo]") {
    DuneCity::CitySimulation sim;

    // Pop 100
    sim.checkAndTriggerMilestones(/*currentPop*/ 100, /*currentFunds*/ 0, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_100) == true);

    // Pop 100 + Economy 1000
    sim.checkAndTriggerMilestones(/*currentPop*/ 100, /*currentFunds*/ 1000, /*res*/ 0, /*com*/ 0, /*ind*/ 0, /*hasRoads*/ false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_100) == true);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ECONOMY_1000) == true);

    // All remaining
    sim.checkAndTriggerMilestones(/*currentPop*/ 5000, /*currentFunds*/ 5000, /*res*/ 10, /*com*/ 5, /*ind*/ 3, /*hasRoads*/ true);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_POPULATION_5000) == true);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ECONOMY_5000) == true);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::MILESTONE_ALL_ZONES) == true);
}

// =============================================================================
// Boundary: invalid milestone index
// =============================================================================

TEST_CASE("Milestones: hasMilestoneBeenAchieved returns false for invalid index", "[milestone][bounds]") {
    DuneCity::CitySimulation sim;

    CHECK(sim.hasMilestoneBeenAchieved(-1) == false);
    CHECK(sim.hasMilestoneBeenAchieved(DuneCity::CitySimulation::NUM_MILESTONES) == false);
    CHECK(sim.hasMilestoneBeenAchieved(999) == false);
}
