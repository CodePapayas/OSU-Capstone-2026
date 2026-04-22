#include "biology.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>
#include <algorithm>
#include "../source/Environment.h"

// idk, this was a suggestion for implementing randomness.
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> dis(0.0, 1.0);

Biology::Biology(bool debug)
    : _energy(1.0), _health(1.0), _water(1.0)
{
    _genetic_values = GetDefaultGeneticValues();
    if (!debug)
    {
        set_random_attributes();
    }
}

// ==================== Initialization & Setup ====================

void Biology::set_random_attributes()
{
    /**
     * Sets the genetic values to random values between 0 and 1
     * Squares the result to bias towards lower values
     */
    for (auto& pair : _genetic_values)
    {
        double random_val = dis(gen);
        pair.second = random_val * random_val; 
    }
}

// ==================== Getters ====================

double Biology::get_health() const
{
    return _health;
}

double Biology::get_energy() const
{
    return _energy;
}

double Biology::get_water() const
{
    return _water;
}

// Creates a map of the values, can be accessed like a python dicttionary
std::unordered_map<std::string, double> Biology::get_efficiencies() const
{
    return _genetic_values;
}

// Returns a specific genetic efficiency value, throws an error if not found
double Biology::get_efficiency(const std::string& efficiency) const
{
    auto it = _genetic_values.find(efficiency);
    if (it == _genetic_values.end())
    {
        throw std::out_of_range("Genetic value not found: " + efficiency);
    }
    return it->second;
}

std::unordered_map<std::string, double> Biology::get_genetic_vals() const
{
    return _genetic_values;
}

// ==================== Setters ====================

void Biology::set_coordinates(const Vector2d& coords)
{
    x = static_cast<int>(coords.x);
    y = static_cast<int>(coords.y);
}

void Biology::set_efficiencies(const std::unordered_map<std::string, double>& vals)
{
    _genetic_values = vals;
}

// Sets specific genetic values
void Biology::set_efficiency(const std::string& type, double value)
{
    if (value < 0.0 || value > 1.0)
    {
        throw std::invalid_argument("Efficiency value must be between 0 and 1");
    }

    auto it = _genetic_values.find(type);
    if (it == _genetic_values.end())
    {
        throw std::out_of_range("Genetic trait not found: " + type);
    }

    it->second = value;
}

void Biology::add_health(double val)
{
    _health = std::min(_health + val, 1.0);
}

void Biology::add_energy(double val)
{
    _energy = std::min(_energy + val, 1.0);
}

void Biology::add_water(double val)
{
    _water = std::min(_water + val, 1.0);
}

// ==================== Resource Consumption ====================

double Biology::eat_energy(double quantity)
{
    /**
     * Adjusts the energy stores obtained from eating based on the creature's
     * various efficiencies. Does some bs using mass to make bigger creatures more costly to maintain
     */
    double amount = quantity * _genetic_values["Energy Efficiency"];
    amount *= std::pow(1.0 - _genetic_values["Mass"], 0.5);
    add_energy(amount);
    return amount;
}

double Biology::drink_water(double quantity)
{
    /**
     * Adjusts the water reserves a creature obtains from drinking based on
     * efficiency.
     */
    double amount = quantity * _genetic_values["Water Efficiency"];
    add_water(amount);
    return amount;
}

double Biology::add_chemical(const std::string& chemical_type, double quantity)
{
    /**
     * Adjusts health based on the chemical type passed.
     */
    auto it = _genetic_values.find(chemical_type);
    if (it == _genetic_values.end())
    {
        throw std::out_of_range("Chemical type not found: " + chemical_type);
    }

    double efficiency = it->second - 0.5;
    double amount = efficiency * quantity * HEALTH_COEFFICIENT;
    add_health(amount);
    return amount;
}

// ==================== Terrain & Movement ====================

double Biology::movement_energy_drain(const std::string& terrain_type)
{
    /**
     * Drains energy based on the type of terrain the creature moved through.
     * Falls back to default traversal efficiency if terrain type not found.
     */
    double efficiency;
    auto it = _genetic_values.find(terrain_type);

    if (it == _genetic_values.end())
    {
        // Use default traversal efficiency
        efficiency = 1.0 - _genetic_values.at("Traversal Efficiency 1");
    }
    else
    {
        efficiency = 1.0 - it->second;
    }

    double amount = std::max(
        efficiency * TERRAIN_ENERGY_COEFFICIENT * _genetic_values["Mass"],
        0.01
    );

    add_energy(amount * -1);
    return amount;
}

double Biology::movement_water_drain(const std::string& terrain_type)
{
    /**
     * Drains water based on the type of terrain navigated.
     */
    double efficiency;
    auto it = _genetic_values.find(terrain_type);

    if (it == _genetic_values.end())
    {
        // Use default traversal efficiency
        efficiency = 1.0 - _genetic_values.at("Traversal Efficiency 1");
    }
    else
    {
        efficiency = 1.0 - it->second;
    }

    double water_efficiency = 1.0 - _genetic_values["Water Efficiency"];
    double amount = std::max(
        water_efficiency * efficiency * TERRAIN_WATER_COEFFICIENT,
        0.01
    );

    add_water(amount * -1);
    return amount;
}

// ==================== Life Cycle ====================

double Biology::tick_energy_drain()
{
    /**
     * Determines how much energy the creature loses every tick.
     * Based on the sum of genetic traits (excluding Mass).
     */
    double total = 0.0;

    for (const auto& pair : _genetic_values)
    {
        if (pair.first != "Mass")
        {
            total += pair.second;
        }
    }

    // Calculate the drain: sqrt of sum, divided by number of traits, adjusted for mass
    total = std::pow(total, 0.5) / static_cast<double>(_genetic_values.size());
    total = std::max(
        total * (1.0 - std::pow(_genetic_values["Mass"], 2.0)),
        0.02
    );

    double drain = total * ENERGY_DRAIN_COEFFICIENT;
    add_energy(drain * -1);
    return total;
}

double Biology::tick_health_drain()
{
    /**
     * Determines how much health the creature loses each tick.
     * Health is drained if energy falls below a mass-dependent threshold.
     */
    if (_energy < 1.0 - _genetic_values["Mass"])
    {
        double difference = _genetic_values["Mass"] - _energy;
        double drain = std::pow(difference, 2.0);
        add_health(drain * -1);
        return drain;
    }
    return 0.0;
}

void Biology::update()
{
    /**
     * Updates the organism for a single game tick.
     * Clamps resources and applies per-tick drains.
     */
    _energy = std::max(_energy, 0.0);
    _water = std::max(_water, 0.0);

    std::cout << "Tick energy loss: " << tick_energy_drain() << std::endl;
    std::cout << "Tick Health loss: " << tick_health_drain() << std::endl;
}

bool Biology::check_death() const
{
    /**
     * Checks if the organism should be considered dead.
     */
    return _health <= 0.0;
}

// ==================== Display & Debugging ====================

void Biology::display_genetic_vals() const
{
    /**
     * Outputs all of the genetic values for the biology.
     */
    for (const auto& pair : _genetic_values)
    {
        std::cout << std::setw(25) << std::left << (pair.first + ":") 
                  << pair.second << std::endl;
    }
}

void Biology::print_vals() const
{
    /**
     * Displays current resource levels.
     */
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Current Health: " << _health << std::endl;
    std::cout << "Current Energy: " << _energy << std::endl;
    std::cout << "Current Water: " << _water << std::endl;
}
