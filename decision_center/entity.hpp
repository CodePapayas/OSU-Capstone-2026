#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <any>
#include <iostream>
#include <vector>
#include "../source/MathVector.hpp"

// Forward declarations
class Biology;
class Brain;

/**
 * @class Entity
 * @brief Represents an individual in the simulation
 * 
 * Coordinates function between biology and decision centers, as well as with 
 * the actual simulation.
 */
class Entity
{
private:
    std::shared_ptr<Biology> _biology;
    std::shared_ptr<Brain> _brain;
    std::any _location;  // Can hold any location type
    long long _id;

public:
    /**
     * @brief Default constructor for Entity
     */
    Entity();

    int x;
    int y;

    /**
     * @brief Destructor for Entity
     */
    ~Entity() = default;

    // ==================== Setters ====================

    /**
     * @brief Sets the organism's biology container
     * @param biology Shared pointer to the Biology object
     */
    void set_biology(const std::shared_ptr<Biology>& biology);

    /**
     * @brief Sets the organism's decision center
     * @param brain Shared pointer to the Brain object
     */
    void set_brain(const std::shared_ptr<Brain>& brain);

    /**
     * @brief Sets the organism's present location
     * @param location The location identifier (can be any type)
     */
    void set_location(const std::any& location);

    // ==================== Getters ====================

    /**
     * @brief Returns the entity's ID
     * @return The ID of this entity
     */
    long long get_id() const;

    /**
     * @brief Returns the organism's biology
     * @return Shared pointer to the Biology object
     */
    std::shared_ptr<Biology> get_biology() const;

    /**
     * @brief Returns the organism's brain
     * @return Shared pointer to the Brain object
     */
    std::shared_ptr<Brain> get_brain() const;

    /**
     * @brief Returns the organism's location
     * @return The location in std::any format
     */
    std::any get_location() const;

    // ==================== Brain Related Methods ====================

    /**
     * @brief Calls upon the brain to make a decision
     * @param inputs Input data vector for the brain decision
     * @return Decision made by the brain
     */
    int brain_get_decision(const std::vector<double>& inputs);


    // ==================== Biology Related Methods ====================

    /**
     * @brief Adds a quantity of a given chemical to the biology
     * @param chem The ID of the chemical
     * @param amount The amount to be added
     */
    void biology_add_chemical(const std::string& chem, double amount);

    /**
     * @brief Removes a quantity of a given chemical from the biology
     * @param chem The ID of the chemical
     * @param amount The amount to be removed
     */
    void biology_rem_chemical(const std::string& chem, double amount);

    /**
     * @brief Passes an amount of energy to be consumed by the creature
     * @param amount The amount to be consumed
     */
    void biology_eat(double amount);

    /**
     * @brief Adds a quantity of energy to the biology
     * @param amount The amount to be added
     */
    void biology_add_energy(double amount);

    /**
     * @brief Removes a quantity of energy from the biology
     * @param amount The amount to be removed
     */
    void biology_rem_energy(double amount);

    /**
     * @brief Passes an amount of water to be consumed by the creature
     * @param amount The amount to be consumed
     */
    void biology_drink(double amount);

    /**
     * @brief Adds a quantity of water to the biology
     * @param amount The amount to be added
     */
    void biology_add_water(double amount);

    /**
     * @brief Removes a quantity of water from the biology
     * @param amount The amount to be removed
     */
    void biology_rem_water(double amount);

    /**
     * @brief Adds a quantity of health to the biology
     * @param amount The amount to be added
     */
    void biology_add_health(double amount);

    /**
     * @brief Removes a quantity of health from the biology
     * @param amount The amount to be removed
     */
    void biology_rem_health(double amount);

    /**
     * @brief Checks if the individual should have died
     * @return True if the organism should die, false otherwise
     */
    bool biology_check_death() const;
    
    /**
     * @brief sets the coordinates on the biology
     * @param coords The coordinates to set as a Vector2d
     */
    void biology_set_coordinates(const Vector2d& coords);
    
    /**
     * @brief Gets the coordinates from the biology
     * @return The coordinates as a Vector2d
     */
    Vector2d biology_get_coordinates() const;

    /**
     * @brief Returns the coordinates of the organism as a Vector2d
     */
    Vector2d get_coordinates() const;
    
    void set_coordinates(const Vector2d& coords);
    
    /**
     * @brief Requests the biology to update itself for a tick
     */
    void update_biology();

    /**
     * @brief Gets the metrics of the organism's biology
     * @return A map containing Health, Energy, and Water values
     */
    std::unordered_map<std::string, double> biology_get_metrics();

    /**
     * @brief Returns the genetic values of the organism
     * @return A map containing the genetic values
     */
    std::unordered_map<std::string, double> biology_get_genetics();

    /**
     * @brief Polls the biology for a particular genetic value
     * @param gene The gene identifier
     * @return The genetic value, or -1 if not found
     */
    double biology_get_genetic_value(const std::string& gene);

    /**
     * @brief Tells the biology to drain energy and water based on terrain type
     * @param terrain The terrain type identifier
     * @return A pair of (energy_drain, water_drain)
     */
    std::pair<double, double> biology_movement(const std::string& terrain);

    // ==================== Convenience Methods and Aliases ====================

    /**
     * @brief Convenience method to get ID via property-like syntax
     * @return The ID of this entity
     */
    long long id() const { return get_id(); }

    /**
     * @brief Convenience method to get brain via property-like syntax
     * @return Shared pointer to the Brain object
     */
    std::shared_ptr<Brain> brain() const { return get_brain(); }

    /**
     * @brief Convenience method to get decision_center via property-like syntax
     * @return Shared pointer to the Brain object
     */
    std::shared_ptr<Brain> decision_center() const { return get_brain(); }

    /**
     * @brief Convenience method to get biology via property-like syntax
     * @return Shared pointer to the Biology object
     */
    std::shared_ptr<Biology> body() const { return get_biology(); }

    /**
     * @brief Convenience method to get biology via property-like syntax
     * @return Shared pointer to the Biology object
     */
    std::shared_ptr<Biology> biology() const { return get_biology(); }

    /**
     * @brief Convenience method to check death status via property-like syntax
     * @return True if the organism is dead, false otherwise
     */
    bool dead() const { return biology_check_death(); }

    /**
     * @brief Convenience method to check death status via property-like syntax
     * @return True if the organism is dead, false otherwise
     */
    bool death() const { return biology_check_death(); }

    /**
     * @brief Convenience method to get location via property-like syntax
     * @return The location in std::any format
     */
    std::any cell() const { return get_coordinates(); }

    /**
     * @brief Convenience method to get location via property-like syntax
     * @return The location in std::any format
     */
    std::any location() const { return get_coordinates(); }
};

#endif // ENTITY_HPP
