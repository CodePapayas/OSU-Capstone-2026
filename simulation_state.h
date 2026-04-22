/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462 
*/
#pragma once

#include <cstdint>
#include <vector>

/**
 * SimulationState - Snapshot of simulation at a specific tick
 * 
 * This structure is what gets stored in the circular buffer.
 * Contains minimal data needed to reconstruct or inspect a simulation moment.
 */
struct SimulationState {
    // Metadata
    uint64_t tick;                      // Simulation tick number
    double timestamp;                   // Real-world timestamp
    
    // Environment state summary
    double totalEnergy;                 // Total energy in environment
    uint32_t totalResources;            // Number of resource nodes
    
    // Agent state summary
    uint32_t agentCount;                // Number of alive agents
    double averageAgentEnergy;          // Mean energy across all agents
    double averageFitness;              // Mean fitness score
    
    // Optional: You can expand this with more detailed snapshots
    // For now, keeping it minimal for performance
    
    SimulationState()
        : tick(0)
        , timestamp(0.0)
        , totalEnergy(0.0)
        , totalResources(0)
        , agentCount(0)
        , averageAgentEnergy(0.0)
        , averageFitness(0.0)
    {}
};
