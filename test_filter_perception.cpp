#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Simulation.hpp"
#include <vector>

// Regression tests: filter_perception must terminate without stack overflow
// for any reasonable input size/tilesToIgnore combination.
//
// Simulation() default constructor is lightweight (no initialize() needed);
// filter_perception only reads its arguments, not any simulation state.

TEST_CASE("filter_perception - edge cases terminate cleanly") {
    Simulation sim;

    SUBCASE("empty perception returns empty") {
        auto result = sim.filter_perception({}, 5);
        CHECK(result.empty());
    }

    SUBCASE("single-element input survives") {
        auto result = sim.filter_perception({0.5}, 1);
        CHECK(result.size() == 1);
        CHECK(result[0] == doctest::Approx(0.5));
    }

    SUBCASE("tilesToIgnore = 0 makes no changes") {
        std::vector<double> perc(25, 1.0);
        auto result = sim.filter_perception(perc, 0);
        CHECK(result == perc);
    }

    SUBCASE("negative tilesToIgnore treated as zero, no changes") {
        std::vector<double> perc(25, 1.0);
        auto result = sim.filter_perception(perc, -3);
        CHECK(result == perc);
    }

    SUBCASE("even grid_size triggers early return (no modifications)") {
        // size=16 -> grid_size=4 (even) -> returns unchanged
        std::vector<double> perc(16, 1.0);
        auto result = sim.filter_perception(perc, 10);
        CHECK(result == perc);
    }
}

TEST_CASE("filter_perception - typical 5x5 perception inputs terminate") {
    Simulation sim;

    SUBCASE("5x5 grid (size 25), small tilesToIgnore: outer tiles zeroed") {
        std::vector<double> perc(25, 1.0);
        auto result = sim.filter_perception(perc, 5);
        CHECK(result.size() == 25);
        int zeroed = 0;
        for (double v : result) if (v == 0.0) ++zeroed;
        CHECK(zeroed > 0);
        CHECK(zeroed <= 5);
    }

    SUBCASE("5x5 grid, tilesToIgnore clamped above tile_count - 1") {
        // tilesToIgnore=25 gets clamped; should not loop forever or overflow stack
        std::vector<double> perc(25, 1.0);
        auto result = sim.filter_perception(perc, 25);
        CHECK(result.size() == 25);
    }

    SUBCASE("5x5 grid: center tile (idx 12) preserved with small tilesToIgnore") {
        std::vector<double> perc(25, 1.0);
        // tilesToIgnore=1: only one outermost tile zeroed; center (radius=2) untouched
        auto result = sim.filter_perception(perc, 1);
        // center at (rx=0,ry=0): x=2, y=2, idx = 2*5 + (5-1-2) = 12
        CHECK(result[12] == doctest::Approx(1.0));
    }
}

TEST_CASE("filter_perception - large grids terminate without stack overflow") {
    Simulation sim;

    SUBCASE("25x25 grid (size 625), high tilesToIgnore") {
        // grid_size = sqrt(625) = 25 (odd) -- exercises many loop iterations
        std::vector<double> perc(625, 1.0);
        auto result = sim.filter_perception(perc, 600);
        CHECK(result.size() == 625);
    }

    SUBCASE("9x9 grid (size 81), tilesToIgnore = 40") {
        std::vector<double> perc(81, 1.0);
        auto result = sim.filter_perception(perc, 40);
        CHECK(result.size() == 81);
        int zeroed = 0;
        for (double v : result) if (v == 0.0) ++zeroed;
        CHECK(zeroed > 0);
    }

    SUBCASE("all-zero perception stays all-zero regardless of tilesToIgnore") {
        std::vector<double> perc(25, 0.0);
        auto result = sim.filter_perception(perc, 10);
        CHECK(result.size() == 25);
        for (double v : result) CHECK(v == 0.0);
    }
}
