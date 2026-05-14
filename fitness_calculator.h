/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462 
*/
#pragma once

#include <cmath>
#include <algorithm>

using namespace std;

/**
 * FitnessCalculator - Evaluates agent viability via weighted metrics
 * (energy, survival, efficiency, reproduction) for evolutionary selection
 */
class FitnessCalculator {
public:
    struct FitnessWeights {
        double energyWeight, survivalWeight, efficiencyWeight, reproductionWeight;
        
        FitnessWeights() 
            : energyWeight(0.4)
            , survivalWeight(0.3)
            , efficiencyWeight(0.2)
            , reproductionWeight(0.1)
        {}
    };
    
    // Calculate weighted fitness score (0.0-1.0) from agent metrics
    static double calculateFitness(double currentEnergy, double maxEnergy, uint64_t age,
                                   double energyGained, double energySpent, uint32_t offspring,
                                   const FitnessWeights& weights = FitnessWeights());
    
    // Individual fitness components (all return 0.0-1.0)
    static double energyScore(double currentEnergy, double maxEnergy);
    static double survivalScore(uint64_t age);  // Logarithmic scaling
    static double efficiencyScore(double energyGained, double energySpent);
    static double reproductionScore(uint32_t offspring);
    
    // Survival thresholds
    static bool meetsMinimumSurvival(double currentEnergy, double minSurvivalEnergy);
    static bool canReproduce(double currentEnergy, double reproductionThreshold);

private:
    static constexpr double SURVIVAL_LOG_BASE = 100.0;
};

// Implementation

inline double FitnessCalculator::calculateFitness(double currentEnergy, double maxEnergy, uint64_t age,
                                                   double energyGained, double energySpent, uint32_t offspring,
                                                   const FitnessWeights& weights) {
    // Weighted sum of normalized components, clamped to [0,1]
    // Higher fitness = more likely to survive and reproduce
    return std::clamp(energyScore(currentEnergy, maxEnergy) * weights.energyWeight +
                      survivalScore(age) * weights.survivalWeight +
                      efficiencyScore(energyGained, energySpent) * weights.efficiencyWeight +
                      reproductionScore(offspring) * weights.reproductionWeight, 0.0, 1.0);
}

inline double FitnessCalculator::energyScore(double currentEnergy, double maxEnergy) {
    // Linear ratio: full energy = 1.0, empty = 0.0
    return maxEnergy <= 0.0 ? 0.0 : std::clamp(currentEnergy / maxEnergy, 0.0, 1.0);
}

inline double FitnessCalculator::survivalScore(uint64_t age) {
    // Logarithmic: surviving longer is better, but with diminishing returns
    // Prevents score from growing unbounded over time
    return age == 0 ? 0.0 : std::clamp(log(double(age) + 1.0) / log(SURVIVAL_LOG_BASE), 0.0, 1.0);
}

inline double FitnessCalculator::efficiencyScore(double energyGained, double energySpent) {
    // If no spending, perfect efficiency if gained anything
    if (energySpent <= 0.0) return energyGained > 0.0 ? 1.0 : 0.0;
    
    // Sigmoid-like normalization: ratio/(ratio+1) keeps it in [0,1]
    // Agents that gain more than they spend get high scores
    double ratio = energyGained / energySpent;
    return std::clamp(ratio / (ratio + 1.0), 0.0, 1.0);
}

inline double FitnessCalculator::reproductionScore(uint32_t offspring) {
    // Log scale: having kids helps fitness, but diminishing returns
    return offspring == 0 ? 0.0 : std::clamp(log(double(offspring) + 1.0) / log(10.0), 0.0, 1.0);
}

inline bool FitnessCalculator::meetsMinimumSurvival(double currentEnergy, double minSurvivalEnergy) {
    return currentEnergy >= minSurvivalEnergy;
}

inline bool FitnessCalculator::canReproduce(double currentEnergy, double reproductionThreshold) {
    return currentEnergy >= reproductionThreshold;
}
