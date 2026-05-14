#ifndef BIOLOGY_HPP
#define BIOLOGY_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include "biology_constants.hpp"
#include "../source/MathVector.hpp"

/**
 * @class Biology
 * @brief Represents the biological systems of a creature in the simulation
 * 
 * Manages health, energy, water, and genetic traits of a creature. Handles
 * all calculations related to resource consumption, terrain traversal, and
 * organism viability.
 */
class Biology
{
private:
    double _energy;
    double _health;
    double _water;
    std::unordered_map<std::string, double> _genetic_values;

public:
    /**
     * @brief Constructor for Biology
     * @param debug If true, uses default genetic values; if false, randomizes them
     */
    explicit Biology(bool debug = false);

    int x;
    int y;
    /**
     * @brief Destructor for Biology
     */
    ~Biology() = default;

    // ==================== Initialization & Setup ====================

    /**
     * @brief Sets genetic values to random values between 0 and 1
     */
    void set_random_attributes();

    // ==================== Getters ====================


    Vector2d get_coordinates() const;
    void set_coordinates(const Vector2d& coords);
    /**
     * @brief Returns the current health of the organism
     * @return The current health value (0.0 to 1.0)
     */
    double get_health() const;

    /**
     * @brief Returns the current energy of the organism
     * @return The current energy value (0.0 to 1.0)
     */
    double get_energy() const;

    /**
     * @brief Returns the current water level of the organism
     * @return The current water value (0.0 to 1.0)
     */
    double get_water() const;

    /**
     * @brief Returns all genetic values (efficiencies)
     * @return A map containing all genetic trait values
     */
    std::unordered_map<std::string, double> get_efficiencies() const;

    /**
     * @brief Returns a specific genetic efficiency value
     * @param efficiency The trait name to query
     * @return The efficiency value (0.0 to 1.0)
     * @throws std::out_of_range if the efficiency is not found
     */
    double get_efficiency(const std::string& efficiency) const;

    /**
     * @brief Returns all genetic values
     * @return A map containing the genetic values
     */
    std::unordered_map<std::string, double> get_genetic_vals() const;

    // ==================== Setters ====================

    /**
     * @brief Sets all genetic values from a map
     * @param vals An unordered_map of genetic values
     */
    void set_efficiencies(const std::unordered_map<std::string, double>& vals);

    /**
     * @brief Sets a specific genetic efficiency value
     * @param type The trait name to modify
     * @param value The new value (must be between 0 and 1)
     * @throws std::invalid_argument if value is not in range [0, 1]
     * @throws std::out_of_range if the trait type doesn't exist
     */
    void set_efficiency(const std::string& type, double value);

    /**
     * @brief Adds a value to health (clamped at 1.0)
     * @param val The amount to add
     */
    void add_health(double val);

    /**
     * @brief Adds a value to energy (clamped at 1.0)
     * @param val The amount to add
     */
    void add_energy(double val);

    /**
     * @brief Adds a value to water (clamped at 1.0)
     * @param val The amount to add
     */
    void add_water(double val);

    // ==================== Resource Consumption ====================

    /**
     * @brief Adjusts energy obtained from eating based on creature efficiencies
     * @param quantity The original amount of energy
     * @return The net amount of energy actually obtained
     */
    double eat_energy(double quantity);

    /**
     * @brief Adjusts water reserves from drinking based on creature efficiency
     * @param quantity The original amount of water
     * @return The net adjusted amount of water
     */
    double drink_water(double quantity);

    /**
     * @brief Adds a chemical with adjusted health impact based on efficiency
     * @param chemical_type The chemical type identifier
     * @param quantity The amount of the chemical absorbed
     * @return The amount health was adjusted
     * @throws std::out_of_range if the chemical type is not found
     */
    double add_chemical(const std::string& chemical_type, double quantity);

    // ==================== Terrain & Movement ====================

    /**
     * @brief Calculates energy drain from moving through a terrain type
     * @param terrain_type The terrain type identifier
     * @return The amount of energy drained
     */
    double movement_energy_drain(const std::string& terrain_type);

    /**
     * @brief Calculates water drain from moving through a terrain type
     * @param terrain_type The terrain type identifier
     * @return The amount of water drained
     */
    double movement_water_drain(const std::string& terrain_type);

    // ==================== Life Cycle ====================

    /**
     * @brief Calculates energy loss per tick based on creature complexity
     * @return The amount of energy drained this tick
     */
    double tick_energy_drain();

    /**
     * @brief Calculates health loss per tick based on energy deficiency
     * @return The amount of health drained this tick
     */
    double tick_health_drain();

    /**
     * @brief Updates the organism for a single game tick
     * Recalculates resource drains and applies them
     */
    void update();

    /**
     * @brief Checks if the organism should be considered dead
     * @return True if health <= 0.0, false otherwise
     */
    bool check_death() const;

    // ==================== Display & Debugging ====================

    /**
     * @brief Displays all genetic values to standard output
     */
    void display_genetic_vals() const;

    /**
     * @brief Displays current health, energy, and water levels
     */
    void print_vals() const;
};

#endif
