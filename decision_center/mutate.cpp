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

std::vector<double> mutate_vector_inbred(const std::vector<double>& original, uint32_t inbreeding_gen) {
    std::vector<double> mutated = original;
    std::mt19937 gen(std::time(nullptr));
    std::uniform_real_distribution<double> chance_dist(0.0, 1.0);

    double g = static_cast<double>(inbreeding_gen);
    // Mutation rate scales linearly; dead-neuron chance scales quadratically → compounding lethality
    double mut_rate      = std::min(MUTATION_CHANCE * (1.0 + g),        0.50);
    double neg_bias      = std::min(0.50 + 0.15 * g,                    0.95); // prob delta is negative
    double dead_chance   = std::min(0.04 * g * g,                       0.60); // prob weight zeroed entirely
    double delta_mag     = std::min(0.10 + 0.05 * g,                    0.40); // mutation magnitude

    for (double& val : mutated) {
        if (chance_dist(gen) < dead_chance) {
            val = 0.0;
            continue;
        }
        if (chance_dist(gen) < mut_rate) {
            if (chance_dist(gen) < 0.2) {
                // Randomize but bias toward low values — inbred genomes trend toward dysfunction
                val = chance_dist(gen) * std::max(0.0, 1.0 - 0.15 * g);
            } else {
                double sign = (chance_dist(gen) < neg_bias) ? -1.0 : 1.0;
                val += sign * delta_mag * chance_dist(gen);
                val = std::clamp(val, 0.0, 1.0);
            }
        }
    }
    return mutated;
}

std::unordered_map<std::string, double> mutate_genetics(const std::unordered_map<std::string, double>& original) {
    std::unordered_map<std::string, double> mutatedMap;
    std::vector<std::string> keys;
    std::vector<double> values;
    for (const auto& pair : original) {
        keys.push_back(pair.first);
        values.push_back(pair.second);
    }
    std::vector<double> mutated_values = mutate_vector(values);
    for (size_t i = 0; i < keys.size(); ++i) {
        mutatedMap[keys[i]] = mutated_values[i];
    }
    return mutatedMap;
}

std::unordered_map<std::string, double> mutate_genetics_inbred(const std::unordered_map<std::string, double>& original, uint32_t inbreeding_gen) {
    std::unordered_map<std::string, double> mutatedMap;
    std::vector<std::string> keys;
    std::vector<double> values;
    for (const auto& pair : original) {
        keys.push_back(pair.first);
        values.push_back(pair.second);
    }
    std::vector<double> mutated_values = mutate_vector_inbred(values, inbreeding_gen);
    for (size_t i = 0; i < keys.size(); ++i) {
        mutatedMap[keys[i]] = mutated_values[i];
    }
    return mutatedMap;
}