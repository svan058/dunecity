/*
 *  CityScoreTestCase.cpp - Tests for CityScore system
 */

#include <catch2/catch2.hpp>
#include <CityScore.h>

TEST_CASE("CityScore basic functionality", "[CityScore]") {
    SECTION("Empty score has zero values") {
        CityScore score;
        REQUIRE(score.totalScore == 0);
        REQUIRE(score.populationScore == 0);
        REQUIRE(score.economyScore == 0);
    }

    SECTION("Score calculation works") {
        CityScore score;
        score.playerName = "TestPlayer";
        score.houseID = 1;
        score.totalPopulation = 1000;
        score.residentialCapacity = 500;
        score.commercialCapacity = 300;
        score.industrialCapacity = 200;
        score.totalHarvested = 50000;
        score.taxRevenue = 10000;
        score.currentCredits = 5000;
        score.powerCoverage = 95;
        score.roadTiles = 100;
        score.totalBuildings = 50;
        score.crimeRate = 10;
        score.pollutionLevel = 20;
        score.landValue = 75;

        score.calculateScores();

        // Verify scores are positive
        REQUIRE(score.populationScore > 0);
        REQUIRE(score.economyScore > 0);
        REQUIRE(score.infrastructureScore > 0);
        REQUIRE(score.qualityOfLifeScore > 0);
        REQUIRE(score.totalScore > 0);

        // Verify total is sum of components
        REQUIRE(score.totalScore == score.populationScore + score.economyScore +
                                      score.infrastructureScore + score.qualityOfLifeScore);
    }
}

TEST_CASE("CityScoreManager rankings", "[CityScore]") {
    CityScoreManager manager;

    SECTION("Empty manager returns empty rankings") {
        auto rankings = manager.getRankings();
        REQUIRE(rankings.empty());
    }

    SECTION("Single player gets rank 1") {
        auto& score = manager.getScore(1);
        score.playerName = "Player1";
        score.totalScore = 1000;
        score.calculateScores();

        REQUIRE(manager.getRank(1) == 1);
    }

    SECTION("Players sorted by score descending") {
        auto& score1 = manager.getScore(1);
        score1.playerName = "Player1";
        score1.totalScore = 1000;
        score1.calculateScores();

        auto& score2 = manager.getScore(2);
        score2.playerName = "Player2";
        score2.totalScore = 2000;
        score2.calculateScores();

        auto& score3 = manager.getScore(3);
        score3.playerName = "Player3";
        score3.totalScore = 500;
        score3.calculateScores();

        auto rankings = manager.getRankings();
        REQUIRE(rankings.size() == 3);
        REQUIRE(rankings[0].houseID == 2);  // Player2 has highest score
        REQUIRE(rankings[1].houseID == 1);  // Player1 in middle
        REQUIRE(rankings[2].houseID == 3);  // Player3 has lowest

        REQUIRE(manager.getRank(2) == 1);
        REQUIRE(manager.getRank(1) == 2);
        REQUIRE(manager.getRank(3) == 3);
    }

    SECTION("Reset clears all scores") {
        auto& score = manager.getScore(1);
        score.totalScore = 1000;
        REQUIRE(manager.size() == 1);

        manager.reset();
        REQUIRE(manager.size() == 0);
    }
}

TEST_CASE("CityScore quality of life bounds", "[CityScore]") {
    SECTION("Negative factors don't cause negative scores") {
        CityScore score;
        score.crimeRate = 100;  // Max crime
        score.pollutionLevel = 100;  // Max pollution
        score.landValue = 0;  // Minimum land value

        score.calculateScores();

        // Quality of life should be 0 (100-100 + 100-100 + 0)
        REQUIRE(score.qualityOfLifeScore >= 0);
        REQUIRE(score.totalScore >= 0);
    }

    SECTION("Best conditions maximize quality of life") {
        CityScore score;
        score.crimeRate = 0;  // No crime
        score.pollutionLevel = 0;  // No pollution
        score.landValue = 100;  // Max land value

        score.calculateScores();

        // Quality of life = 100*5 + 100*5 + 100*2 = 500 + 500 + 200 = 1200
        REQUIRE(score.qualityOfLifeScore == 1200);
    }
}
