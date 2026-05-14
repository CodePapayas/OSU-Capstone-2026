#ifndef BIOLOGY_CONSTANTS_HPP
#define BIOLOGY_CONSTANTS_HPP

#include <unordered_map>
#include <string>

// Energy and resource coefficients
constexpr double ENERGY_DRAIN_COEFFICIENT = .5;
constexpr double TERRAIN_ENERGY_COEFFICIENT = .5;
constexpr double TERRAIN_WATER_COEFFICIENT = .2;
constexpr double HEALTH_COEFFICIENT = .5;

// Terrain type identifiers
const std::string TERRAIN_1 = "Traversal Efficiency 1";
const std::string TERRAIN_2 = "Traversal Efficiency 2";
const std::string TERRAIN_3 = "Traversal Efficiency 3";

/**
 * @brief Returns the default genetic values for a creature
 * @return An unordered_map with default genetic trait values
 */
inline std::unordered_map<std::string, double> GetDefaultGeneticValues()
{
    return {
        {"Energy Efficiency", 0.5},
        {"Water Efficiency", 0.8},
        {"Mass", 0.5},
        {"Vision", 0.4},
        {"Chem 1", 0.9},
        {"Chem 2", 0.1},
        {"Chem 3", 0.3},
        {"Chem 4", 0.4},
        {"Traversal Efficiency 1", 0.6},
        {"Traversal Efficiency 2", 0.4},
        {"Traversal Efficiency 3", 0.35}
    };
}

#endif // BIOLOGY_CONSTANTS_HPP
