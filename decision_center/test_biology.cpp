#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests/doctest.h"
#include "biology.hpp"
#include "biology_constants.hpp"

// ==================== Biology Tests with Default Values ====================

TEST_SUITE("Biology - Default Values")
{
    TEST_CASE("Default genetics loaded correctly")
    {
        auto guy = std::make_shared<Biology>(true);
        auto values = guy->get_genetic_vals();

        CHECK(values["Energy Efficiency"] == doctest::Approx(0.5));
        CHECK(values["Water Efficiency"] == doctest::Approx(0.8));
        CHECK(values["Mass"] == doctest::Approx(0.5));
        CHECK(values["Vision"] == doctest::Approx(0.4));
        CHECK(values["Chem 1"] == doctest::Approx(0.9));
        CHECK(values["Chem 2"] == doctest::Approx(0.1));
        CHECK(values["Chem 3"] == doctest::Approx(0.3));
        CHECK(values["Chem 4"] == doctest::Approx(0.4));
        CHECK(values["Traversal Efficiency 1"] == doctest::Approx(0.6));
        CHECK(values["Traversal Efficiency 2"] == doctest::Approx(0.4));
        CHECK(values["Traversal Efficiency 3"] == doctest::Approx(0.35));
    }

    TEST_CASE("Eating energy returns expected values")
    {
        auto guy = std::make_shared<Biology>(true);
        double eaten = guy->eat_energy(0.5);
        
        CHECK(eaten == doctest::Approx(0.1767).epsilon(0.001));
        CHECK(guy->get_energy() == doctest::Approx(1.0));
    }

    TEST_CASE("Drinking water returns expected value")
    {
        auto guy = std::make_shared<Biology>(true);
        double drunk = guy->drink_water(0.4);
        
        CHECK(drunk == doctest::Approx(0.32).epsilon(0.001));
        CHECK(guy->get_water() == doctest::Approx(1.0));
    }

    TEST_CASE("Moving through terrain 1 loses expected energy amount")
    {
        auto guy = std::make_shared<Biology>(true);
        double energy_drained = guy->movement_energy_drain(TERRAIN_1);
        
        CHECK(energy_drained == doctest::Approx(0.2 * TERRAIN_ENERGY_COEFFICIENT).epsilon(0.001));
        CHECK(guy->get_energy() == doctest::Approx(0.8).epsilon(0.001));
    }

    TEST_CASE("Moving through terrain 2 reduces water appropriately")
    {
        auto guy = std::make_shared<Biology>(true);
        double water_drained = guy->movement_water_drain(TERRAIN_2);
        
        CHECK(water_drained == doctest::Approx(0.12 * TERRAIN_WATER_COEFFICIENT).epsilon(0.001));
        CHECK(guy->get_water() == doctest::Approx(0.88).epsilon(0.001));
    }

    TEST_CASE("Adding chem 1 affects health as expected")
    {
        auto guy = std::make_shared<Biology>(true);
        double health_change = guy->add_chemical("Chem 1", 0.5);
        
        CHECK(health_change == doctest::Approx(0.2).epsilon(0.001));
    }

    TEST_CASE("Adding chem 2 affects health as expected")
    {
        auto guy = std::make_shared<Biology>(true);
        double health_change = guy->add_chemical("Chem 2", 0.4);
        
        CHECK(health_change == doctest::Approx(-0.16).epsilon(0.001));
    }

    TEST_CASE("Energy is drained as expected per tick")
    {
        auto guy = std::make_shared<Biology>(true);
        double energy_drained = guy->tick_energy_drain();
        
        CHECK(energy_drained == doctest::Approx(0.1485).epsilon(0.001));
    }

    TEST_CASE("Health is drained appropriately when energy is low")
    {
        auto guy = std::make_shared<Biology>(true);
        guy->add_energy(-0.9);  // Subtract 0.9 energy
        
        double health_drained = guy->tick_health_drain();
        
        CHECK(health_drained == doctest::Approx(0.16).epsilon(0.001));
    }

    TEST_CASE("Organism reports death status appropriately")
    {
        auto guy = std::make_shared<Biology>(true);
        
        CHECK_FALSE(guy->check_death());
        
        guy->add_health(-1.1);  // Kill the organism
        CHECK(guy->check_death());
        
        guy->add_health(1.1);   // Bring back to 1.0
        guy->add_health(-0.9999);  // Set to ~0.0001
        CHECK_FALSE(guy->check_death());
    }
}

// ==================== Biology Tests with Random Values ====================

TEST_SUITE("Biology - Random Values")
{
    TEST_CASE("Random creature behavior with various operations")
    {
        auto guy = std::make_shared<Biology>(false);  // Random genetics
        
        SUBCASE("Display and consume resources")
        {
            guy->display_genetic_vals();
            
            double eaten = guy->eat_energy(0.3);
            double drunk = guy->drink_water(0.7);
            double energy_drained_t1 = guy->movement_energy_drain(TERRAIN_1);
            double energy_drained_t3 = guy->movement_energy_drain(TERRAIN_3);
            double chem_effect = guy->add_chemical("Chem 4", 0.5);
            
            // Verify all values are valid
            CHECK(eaten >= 0.0);
            CHECK(drunk >= 0.0);
            CHECK(energy_drained_t1 >= 0.01);  // Minimum is 0.01
            CHECK(energy_drained_t3 >= 0.01);
            CHECK(chem_effect <= 1.0);  // Max impact on health
            
            guy->update();
            guy->print_vals();
        }
    }
}
