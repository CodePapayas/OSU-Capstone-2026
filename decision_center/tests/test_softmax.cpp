#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../brain.hpp"
#include <numeric>
#include <set>
#include <cmath>

TEST_CASE("soft_max - probability distribution properties") {
    SUBCASE("empty input returns empty") {
        CHECK(soft_max({}).empty());
    }

    SUBCASE("output size matches input size") {
        std::vector<double> input = {1.0, 2.0, 3.0, 4.0};
        CHECK(soft_max(input).size() == 4);
    }

    SUBCASE("outputs sum to 1.0") {
        std::vector<double> input = {1.0, 2.0, 3.0, 4.0};
        auto out = soft_max(input);
        double sum = std::accumulate(out.begin(), out.end(), 0.0);
        CHECK(sum == doctest::Approx(1.0).epsilon(1e-9));
    }

    SUBCASE("all outputs in [0, 1]") {
        std::vector<double> input = {-5.0, 0.0, 5.0, 10.0, -10.0};
        for (double v : soft_max(input)) {
            CHECK(v >= 0.0);
            CHECK(v <= 1.0);
        }
    }

    SUBCASE("equal inputs produce equal probabilities") {
        auto out = soft_max({2.0, 2.0, 2.0, 2.0});
        for (double v : out)
            CHECK(v == doctest::Approx(0.25).epsilon(1e-9));
    }

    SUBCASE("dominant input captures most probability mass") {
        auto out = soft_max({100.0, 0.0, 0.0, 0.0});
        CHECK(out[0] > 0.99);
        CHECK(out[1] < 0.01);
    }
}

TEST_CASE("soft_max - numerical stability (large values)") {
    SUBCASE("large positive values: no nan/inf, sum = 1") {
        std::vector<double> input = {1000.0, 999.0, 998.0};
        auto out = soft_max(input);
        for (double v : out) {
            CHECK(!std::isnan(v));
            CHECK(!std::isinf(v));
        }
        double sum = std::accumulate(out.begin(), out.end(), 0.0);
        CHECK(sum == doctest::Approx(1.0).epsilon(1e-9));
    }

    SUBCASE("large negative values: no nan/inf, sum = 1") {
        std::vector<double> input = {-1000.0, -999.0, -998.0};
        auto out = soft_max(input);
        for (double v : out) {
            CHECK(!std::isnan(v));
            CHECK(!std::isinf(v));
        }
        double sum = std::accumulate(out.begin(), out.end(), 0.0);
        CHECK(sum == doctest::Approx(1.0).epsilon(1e-9));
    }

    SUBCASE("mixed extreme values: stable") {
        auto out = soft_max({500.0, -500.0, 0.0});
        for (double v : out) {
            CHECK(!std::isnan(v));
            CHECK(!std::isinf(v));
            CHECK(v >= 0.0);
            CHECK(v <= 1.0);
        }
    }
}

TEST_CASE("softDecide - valid probabilistic decisions") {
    SUBCASE("returns index in valid range") {
        Brain brain({5, 8, 7}, 42);
        std::vector<double> input(5, 0.5);
        int d = brain.softDecide(input);
        CHECK(d >= 0);
        CHECK(d < 7);
    }

    SUBCASE("repeated calls on same brain produce multiple distinct outputs") {
        // With equal-weight inputs, all 4 outputs should appear given enough trials
        Brain brain({3, 4}, 0);
        std::vector<double> input = {1.0, 1.0, 1.0};
        std::set<int> seen;
        for (int i = 0; i < 200; ++i)
            seen.insert(brain.softDecide(input));
        CHECK(seen.size() > 1);
    }

    SUBCASE("different seeds produce different decisions") {
        // Across 100 different-seeded brains, we should observe more than one output
        std::set<int> seen;
        std::vector<double> input = {0.5, 0.5, 0.5};
        for (unsigned int seed = 0; seed < 100; ++seed) {
            Brain brain({3, 4}, seed);
            seen.insert(brain.softDecide(input));
        }
        CHECK(seen.size() > 1);
    }

    SUBCASE("deterministic within a single seeded brain given same rng state is NOT guaranteed") {
        // softDecide advances the rng, so two calls may differ - just verify both valid
        Brain brain({4, 6}, 99);
        std::vector<double> input(4, 1.0);
        int d1 = brain.softDecide(input);
        int d2 = brain.softDecide(input);
        CHECK(d1 >= 0); CHECK(d1 < 6);
        CHECK(d2 >= 0); CHECK(d2 < 6);
    }
}
