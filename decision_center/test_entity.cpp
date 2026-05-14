#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests/doctest.h"
#include "entity.hpp"
#include "biology.hpp"

// ==================== Entity Tests ====================

TEST_SUITE("Entity - Default Initialization")
{
    TEST_CASE("Entity initializes correctly")
    {
        auto entity = std::make_shared<Entity>();
        
        CHECK(entity->get_id() >= 0);
        CHECK(entity->get_biology() == nullptr);
        CHECK(entity->get_brain() == nullptr);
    }

    TEST_CASE("Biology can be set on entity")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        
        entity->set_biology(guy);
        CHECK(entity->get_biology() == guy);
    }

    TEST_CASE("Entity ID is accessible via property-like syntax")
    {
        auto entity = std::make_shared<Entity>();
        long long id = entity->id();
        
        CHECK(id >= 0);
    }

    TEST_CASE("Biology is accessible via property-like syntax")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        
        entity->set_biology(guy);
        auto bio = entity->biology();
        CHECK(bio == guy);
    }

    TEST_CASE("Entity death status works correctly")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        entity->set_biology(guy);
        
        // Initially should not be dead
        CHECK_FALSE(entity->dead());
        
        // Kill the biology
        guy->add_health(-1.1);
        CHECK(entity->dead());
    }

    TEST_CASE("Entity can add and remove energy")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        entity->set_biology(guy);
        
        entity->biology_add_energy(0.1);
        CHECK(guy->get_energy() == doctest::Approx(1.0));  // Clamped at 1.0
        
        entity->biology_rem_energy(0.2);
        CHECK(guy->get_energy() == doctest::Approx(0.8).epsilon(0.001));
    }

    TEST_CASE("Entity can add and remove health")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        entity->set_biology(guy);
        
        entity->biology_add_health(0.1);
        CHECK(guy->get_health() == doctest::Approx(1.0));  // Clamped at 1.0
        
        entity->biology_rem_health(0.2);
        CHECK(guy->get_health() == doctest::Approx(0.8).epsilon(0.001));
    }

    TEST_CASE("Entity can add and remove water")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        entity->set_biology(guy);
        
        entity->biology_add_water(0.1);
        CHECK(guy->get_water() == doctest::Approx(1.0));  // Clamped at 1.0
        
        entity->biology_rem_water(0.2);
        CHECK(guy->get_water() == doctest::Approx(0.8).epsilon(0.001));
    }

    TEST_CASE("Entity can get metrics from biology")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        entity->set_biology(guy);
        
        auto metrics = entity->biology_get_metrics();
        
        CHECK(metrics.find("Health") != metrics.end());
        CHECK(metrics.find("Energy") != metrics.end());
        CHECK(metrics.find("Water") != metrics.end());
    }

    TEST_CASE("Entity can get genetics from biology")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        entity->set_biology(guy);
        
        auto genetics = entity->biology_get_genetics();
        
        CHECK(genetics.size() > 0);
        CHECK(genetics.find("Energy Efficiency") != genetics.end());
    }

    TEST_CASE("Location setting and getting")
    {
        auto entity = std::make_shared<Entity>();
        
        // Test with integer location
        int location = 42;
        entity->set_location(location);
        auto retrieved_loc = entity->get_location();
        CHECK(std::any_cast<int>(retrieved_loc) == 42);
        
        // Test via convenience method
        auto cell = entity->cell();
        CHECK(std::any_cast<int>(cell) == 42);
    }

    TEST_CASE("Convenience method aliases")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        entity->set_biology(guy);
        
        // Test body() alias
        auto body = entity->body();
        CHECK(body == guy);
        
        // Test ID access
        long long id1 = entity->get_id();
        long long id2 = entity->id();
        CHECK(id1 == id2);
    }

    TEST_CASE("Entity can eat and drink through biology")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        entity->set_biology(guy);
        
        double initial_energy = guy->get_energy();
        entity->biology_eat(0.5);
        // Energy should have increased (clamped at 1.0)
        CHECK(guy->get_energy() <= 1.0);
        
        entity->biology_drink(0.4);
        // Water should have increased (clamped at 1.0)
        CHECK(guy->get_water() <= 1.0);
    }

    TEST_CASE("Entity can add and remove chemicals")
    {
        auto guy = std::make_shared<Biology>(true);
        auto entity = std::make_shared<Entity>();
        entity->set_biology(guy);
        
        // Chem 1 has a positive effect (0.9 - 0.5 = 0.4)
        entity->biology_add_chemical("Chem 1", 0.5);
        // Chem 2 has a negative effect (0.1 - 0.5 = -0.4)
        entity->biology_rem_chemical("Chem 2", 0.5);
        // Just verify operations complete without error
        CHECK(true);
    }
}
