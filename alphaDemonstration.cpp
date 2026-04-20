#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <chrono>
#include <thread>
#include <ctime>
#include <filesystem>

#include "source/simulation/Simulation.hpp"
#include "source/simulation/circular_buffer.h"
#include "source/simulation/simulation_state.h"


/** Capture a lightweight SimulationState snapshot from the live simulation. */
static SimulationState capture_state(Simulation& sim, uint64_t tick) {
    SimulationState state;
    state.tick      = tick;
    state.timestamp = static_cast<double>(std::time(nullptr));
    state.agentCount = static_cast<uint32_t>(sim.get_entity_count());

    Entity* primary = sim.get_primary_entity();
    if (primary) {
        auto metrics = primary->biology_get_metrics();
        state.averageAgentEnergy = metrics["Energy"];
        state.averageFitness     = 0.0;
    }

    state.totalResources = 0;
    state.totalEnergy    = 0.0;
    return state;
}

/** Write every entry currently in the circular buffer to a plain-text file. */
static void save_buffer_to_file(const CircularBuffer<SimulationState>& buffer,
                                const std::string& filepath) {
    std::ofstream out(filepath);
    if (!out.is_open()) {
        std::cerr << "[AUTOSAVE] Failed to open " << filepath << std::endl;
        return;
    }

    out << "=== Autosave ===\n"
        << "Buffer entries: " << buffer.size()
        << " / " << buffer.capacity() << "\n\n";

    for (size_t i = 0; i < buffer.size(); ++i) {
        const auto& s = buffer.get(i);
        out << "Tick: "          << s.tick
            << " | Agents: "    << s.agentCount
            << " | Avg Energy: " << s.averageAgentEnergy
            << " | Avg Fitness: " << s.averageFitness
            << " | Resources: " << s.totalResources
            << " | Total Energy: " << s.totalEnergy
            << "\n";
    }
}

/** Print CLI information. */
static void print_usage(const char* prog) {
    std::cout
        << "Usage: " << prog << " [options]\n"
        << "\nOptions:\n"
        << "  --ticks N         Number of simulation ticks to run  (default: 10)\n"
        << "  --agents N        Number of agents to spawn [1-50]   (default: 5)\n"
        << "  --autosave K      Autosave every K ticks, 0=off      (default: 0)\n"
        << "  --buffer-size N   Circular buffer capacity            (default: 1000)\n"
        << "  --save-dir DIR    Directory for autosave files        (default: saves/)\n"
        << "  --help            Show this help message\n";
}

int main(int argc, char* argv[]) {

    // ---- Default configuration ----
    int         numTicks         = 10;
    int         numAgents        = 5;
    int         autosaveInterval = 0;       // 0 = autosave disabled
    size_t      bufferCapacity   = 1000;
    std::string saveDir          = "saves";

    // ---- Parse command-line arguments ----
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--ticks" && i + 1 < argc) {
            numTicks = std::stoi(argv[++i]);
        } else if (arg == "--agents" && i + 1 < argc) {
            numAgents = std::max(1, std::min(std::stoi(argv[++i]), 50));
        } else if (arg == "--autosave" && i + 1 < argc) {
            autosaveInterval = std::stoi(argv[++i]);
        } else if (arg == "--buffer-size" && i + 1 < argc) {
            bufferCapacity = static_cast<size_t>(std::stoull(argv[++i]));
        } else if (arg == "--save-dir" && i + 1 < argc) {
            saveDir = argv[++i];
        } else if (arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    // ---- Initialise simulation ----
    Simulation sim;
    sim.initialize(numAgents);
    std::cout << "\nSimulation setup complete with "
              << sim.get_entity_count() << " entities." << std::endl;

    // ---- Initialise circular buffer for state history ----
    CircularBuffer<SimulationState> stateHistory(bufferCapacity);
    std::cout << "Circular buffer initialised (capacity: "
              << bufferCapacity << ")" << std::endl;

    if (autosaveInterval > 0) {
        std::filesystem::create_directories(saveDir);
        std::cout << "Autosave enabled every " << autosaveInterval
                  << " tick(s) -> " << saveDir << "/" << std::endl;
    }

    // ---- Main simulation loop ----
    int autosaveCount = 0;

    for (int i = 0; i < numTicks; ++i) {
        std::cout << "\n=== Tick " << (i + 1) << " ===" << std::endl;
        int result = sim.tick();

        stateHistory.push(capture_state(sim, static_cast<uint64_t>(i + 1)));

        // Autosave check
        if (autosaveInterval > 0 && (i + 1) % autosaveInterval == 0) {
            ++autosaveCount;
            std::string path = saveDir + "/autosave_tick_"
                             + std::to_string(i + 1) + ".txt";
            save_buffer_to_file(stateHistory, path);
            std::cout << "[AUTOSAVE] tick " << (i + 1)
                      << " -> " << path
                      << "  (buffer: " << stateHistory.size()
                      << "/" << stateHistory.capacity() << ")" << std::endl;
        }

        if (result == -1) {
            std::cout << "Entity died at tick " << (i + 1) << "." << std::endl;
            if (autosaveInterval > 0) {
                std::string path = saveDir + "/autosave_final_tick_"
                                 + std::to_string(i + 1) + ".txt";
                save_buffer_to_file(stateHistory, path);
                std::cout << "[AUTOSAVE] Final save -> " << path << std::endl;
            }
            break;
        }
    }

    // Print a summary to the console
    std::cout << "\n=== Simulation Complete ===" << std::endl;
    std::cout << "State history: " << stateHistory.size()
              << "/" << stateHistory.capacity() << " entries" << std::endl;

    if (autosaveInterval > 0)
        std::cout << "Total autosaves written: " << autosaveCount << std::endl;

    if (!stateHistory.empty()) {
        const auto& latest = stateHistory.latest();
        std::cout << "Latest state — Tick: " << latest.tick
                  << ", Agents: " << latest.agentCount
                  << ", Avg Energy: " << latest.averageAgentEnergy << std::endl;
    }

    return 0;
}
