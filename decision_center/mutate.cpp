#include <cmath>
#include <vector>
#include <numeric>
#include <random>
#include <ctime>
#include <algorithm>
#include "mutate.hpp"
#include <unordered_map>

std::vector<double> mutate_vector(const std::vector<double>& original) {
    std::vector<double> mutated = original;
    std::mt19937 gen(std::time(nullptr));
    std::uniform_real_distribution<double> mutation_dist(-0.1, 0.1); // Mutation changes value by up to ±0.05
    std::uniform_real_distribution<double> chance_dist(0.0, 1.0); // For mutation chance

    for (double& val : mutated) {
        double roll = chance_dist(gen);
        if (roll < MUTATION_CHANCE) {
            if (chance_dist(gen) < .2) {
                val = chance_dist(gen); // 20% chance to completely randomize the value instead of just mutating it slightly
            }
            else{
                val += mutation_dist(gen);
                if (val > 1.0) val = 1.0;
                if (val < 0.0) val = 0.0;
            }
        }
    }
    return mutated;
}

std::unordered_map<std::string, double> mutate_genetics(const std::unordered_map<std::string, double>& original) {
    std::unordered_map<std::string, double> mutatedMap;
    // get the keys and values as vectors
    std::vector<std::string> keys;
    std::vector<double> values;
    for (const auto& pair : original) {
        keys.push_back(pair.first);
        values.push_back(pair.second);
    }
    // mutate the values as a vector
    std::vector<double> mutated_values = mutate_vector(values);
    // reconstruct the map
    for (size_t i = 0; i < keys.size(); ++i) {
        mutatedMap[keys[i]] = mutated_values[i];
    }
    return mutatedMap;
}