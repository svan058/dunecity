/*
 *  OverlayLegendTestCase.cpp - Unit tests for overlay color gradient legend
 *
 *  Tests the color interpolation formula used in the city stats HUD overlay legend.
 *  Formula: color(t) = color1 + (color2 - color1) * t,  where t ∈ [0, 1]
 */

#include <catch2/catch_all.hpp>

// Standalone color interpolation function matching GameInterface.cpp drawLegendBar
static void interpolateColor(uint8_t r1, uint8_t g1, uint8_t b1,
                             uint8_t r2, uint8_t g2, uint8_t b2,
                             float t,
                             uint8_t& r, uint8_t& g, uint8_t& b) {
    r = static_cast<uint8_t>(r1 + (r2 - r1) * t);
    g = static_cast<uint8_t>(g1 + (g2 - g1) * t);
    b = static_cast<uint8_t>(b1 + (b2 - b1) * t);
}

TEST_CASE("OverlayLegend: PowerGrid gradient Red→Green", "[overlay-legend][color-interp]") {
    uint8_t r, g, b;

    // At t=0 (low end): should be Red(200,0,0)
    interpolateColor(200, 0, 0, 0, 255, 0, 0.0f, r, g, b);
    CHECK(r == 200);
    CHECK(g == 0);
    CHECK(b == 0);

    // At t=1 (high end): should be Green(0,255,0)
    interpolateColor(200, 0, 0, 0, 255, 0, 1.0f, r, g, b);
    CHECK(r == 0);
    CHECK(g == 255);
    CHECK(b == 0);

    // At t=0.5 (midpoint): ~(100, 127, 0)
    interpolateColor(200, 0, 0, 0, 255, 0, 0.5f, r, g, b);
    CHECK(r == 100);
    CHECK(g == 127);
    CHECK(b == 0);
}

TEST_CASE("OverlayLegend: TrafficDensity gradient Green→Red", "[overlay-legend][color-interp]") {
    uint8_t r, g, b;

    // Low = Green(0,255,0)
    interpolateColor(0, 255, 0, 255, 0, 0, 0.0f, r, g, b);
    CHECK(r == 0);
    CHECK(g == 255);
    CHECK(b == 0);

    // High = Red(255,0,0)
    interpolateColor(0, 255, 0, 255, 0, 0, 1.0f, r, g, b);
    CHECK(r == 255);
    CHECK(g == 0);
    CHECK(b == 0);
}

TEST_CASE("OverlayLegend: Pollution gradient Green→Purple", "[overlay-legend][color-interp]") {
    uint8_t r, g, b;

    // Low = Green(0,255,0)
    interpolateColor(0, 255, 0, 150, 0, 200, 0.0f, r, g, b);
    CHECK(r == 0);
    CHECK(g == 255);
    CHECK(b == 0);

    // High = Purple(150,0,200)
    interpolateColor(0, 255, 0, 150, 0, 200, 1.0f, r, g, b);
    CHECK(r == 150);
    CHECK(g == 0);
    CHECK(b == 200);
}

TEST_CASE("OverlayLegend: LandValue gradient Red→Green", "[overlay-legend][color-interp]") {
    uint8_t r, g, b;

    // Low = Red(255,0,0)
    interpolateColor(255, 0, 0, 0, 255, 0, 0.0f, r, g, b);
    CHECK(r == 255);
    CHECK(g == 0);
    CHECK(b == 0);

    // High = Green(0,255,0)
    interpolateColor(255, 0, 0, 0, 255, 0, 1.0f, r, g, b);
    CHECK(r == 0);
    CHECK(g == 255);
    CHECK(b == 0);
}

TEST_CASE("OverlayLegend: CrimeRate gradient Green→Red", "[overlay-legend][color-interp]") {
    uint8_t r, g, b;

    // Safe = Green(0,255,0)
    interpolateColor(0, 255, 0, 255, 0, 0, 0.0f, r, g, b);
    CHECK(r == 0);
    CHECK(g == 255);
    CHECK(b == 0);

    // Dangerous = Red(255,0,0)
    interpolateColor(0, 255, 0, 255, 0, 0, 1.0f, r, g, b);
    CHECK(r == 255);
    CHECK(g == 0);
    CHECK(b == 0);
}

TEST_CASE("OverlayLegend: Population gradient DarkGray→White", "[overlay-legend][color-interp]") {
    uint8_t r, g, b;

    // Sparse = DarkGray(50,50,50)
    interpolateColor(50, 50, 50, 255, 255, 255, 0.0f, r, g, b);
    CHECK(r == 50);
    CHECK(g == 50);
    CHECK(b == 50);

    // Dense = White(255,255,255)
    interpolateColor(50, 50, 50, 255, 255, 255, 1.0f, r, g, b);
    CHECK(r == 255);
    CHECK(g == 255);
    CHECK(b == 255);

    // Midpoint ~(152,152,152)
    interpolateColor(50, 50, 50, 255, 255, 255, 0.5f, r, g, b);
    CHECK(r == 152);
    CHECK(g == 152);
    CHECK(b == 152);
}

TEST_CASE("OverlayLegend: Gradient intermediate stops", "[overlay-legend][color-interp]") {
    uint8_t r, g, b;

    // 10-stop PowerGrid gradient: at each stop i, t = i/9
    interpolateColor(200, 0, 0, 0, 255, 0, 0.0f/9.0f, r, g, b);
    CHECK(r == 200); CHECK(g == 0); CHECK(b == 0);

    interpolateColor(200, 0, 0, 0, 255, 0, 9.0f/9.0f, r, g, b);
    CHECK(r == 0); CHECK(g == 255); CHECK(b == 0);

    // First stop after 0: ~(177, 0, 0)
    interpolateColor(200, 0, 0, 0, 255, 0, 1.0f/9.0f, r, g, b);
    CHECK(r == 177);
    CHECK(g == 28);
    CHECK(b == 0);
}
