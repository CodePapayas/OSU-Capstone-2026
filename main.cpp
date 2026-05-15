#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <chrono>
#include <thread>
#include <ctime>
#include <filesystem>
#include <algorithm>
#include <numeric>
#include <memory>
#include <vector>
#include "source/simulation/Simulation.hpp"
#include "source/simulation/circular_buffer.h"
#include "source/simulation/simulation_state.h"
#include "source/entity/decision_center/biology.hpp"
#include "source/entity/decision_center/brain.hpp"


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
        << "  --autosave K      Autosave every K ticks, 0=off      (default: 0)\n"
        << "  --buffer-size N   Circular buffer capacity            (default: 1000)\n"
        << "  --save-dir DIR    Directory for autosave files        (default: saves/)\n"
        << "  --help            Show this help message\n";
}

/** Clone an entity with independent Brain and Biology ownership. */
static std::unique_ptr<Entity> clone_entity(const Entity& src) {
    auto clone = std::make_unique<Entity>();
    clone->set_coordinates(src.get_coordinates());

    if (src.get_brain()) {
        clone->set_brain(std::make_shared<Brain>(*src.get_brain()));
    }

    if (src.get_biology()) {
        clone->set_biology(std::make_shared<Biology>(*src.get_biology()));
    }

    return clone;
}

/*Runs a simulation that does the following:
1. runs a simulation over 100 generations (with the same environment each time)
2. In each simulation picks the top 10% performing individuals and breeds them with each other to create the next generation
3. Outputs the average fitness of each generation to a file for later analysis and prints to console.
4. Every 10 iterations, pauses and askes the user if they want to see the current top performer
    Output genes and run a mock simulation
*/
void alphaDemonstration(){
    Simulation sim;
    sim.initialize();
    std::cout << "\nSimulation setup complete with "
              << sim.get_entity_count() << " entity!" << std::endl;

    // ---- Initialise circular buffer for state history ----
    CircularBuffer<SimulationState> stateHistory(1000);
    std::cout << "Circular buffer initialised (capacity: 1000)" << std::endl;
    
    int numGenerations = 100;
    int childrenInGenerations = 100;
    int numTicksMax = 10000;
    std::vector<std::unique_ptr<Entity>> entities(childrenInGenerations);
    std::vector<double> fitness_history(childrenInGenerations, 0.0);
    for (int i = 0; i < numGenerations; i++){
        std::cout << "\n=== Generation " << (i + 1) << " ===" << std::endl;
        if (i == 0) {
            for (int j = 0; j < childrenInGenerations; j++) {
                sim.set_primary_entity_random();
                Entity* sampled = sim.get_primary_entity();
                if (!sampled) {
                    throw std::runtime_error("Failed to sample initial entity");
                }
                entities[j] = clone_entity(*sampled);
            }
        } else {
            // zip fitness history and entities together, sort by fitness, and select parents from the top 10
            std::vector<std::pair<double, int>> fitness_entity_pairs;
            for (int j = 0; j < childrenInGenerations; j++){
                fitness_entity_pairs.push_back(std::make_pair(fitness_history[j], j));
            }
            std::sort(fitness_entity_pairs.begin(), fitness_entity_pairs.end(),
                [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
                    return a.first > b.first; // Sort in descending order of fitness
                });
            int top_10_percent = std::max(1, childrenInGenerations / 10);
            std::vector<int> parents;
            for (int j = 0; j < top_10_percent; j++){
                parents.push_back(fitness_entity_pairs[j].second);
            }
            // Breed new generation from parents
            for (int j = 0; j < childrenInGenerations; j++){
                Entity* parent1 = entities[parents[rand() % top_10_percent]].get();
                Entity* parent2 = entities[parents[rand() % top_10_percent]].get();
                Entity* child = sim.reproduce(parent1, parent2);
                entities[j] = clone_entity(*child);
                fitness_history[j] = 0.0; // reset fitness history for the new generation
            }
        }
        for (int j = 0; j < childrenInGenerations; j++){
            // Average performance across multiple sims
            int total = 0;
            for (int m=0;m<3;m++){
                sim.seed_resources(); // OOPS I had forgotten to reseed resources, so entities were just starving to death every generation later on
                int ticks = 0;
                sim.set_primary_entity(*entities[j]);
                // Reset health, energy and water to 1 for the primary entity to ensure fair fitness evaluation
                sim.get_primary_entity()->get_biology()->add_energy(1.);
                sim.get_primary_entity()->get_biology()->add_health(1.);
                sim.get_primary_entity()->get_biology()->add_water(1.);
                while(ticks < numTicksMax){
                   // cout << "Generation " << (i + 1) << ", Entity " << (j + 1) << ", Tick " << ticks << "\n";
                    stateHistory.push(capture_state(sim, static_cast<uint64_t>(i + 1)));
                    int result = sim.tick(0);
                    if(result == -1 || ticks == numTicksMax-1){
                        total += ticks;
                        cout << "Entity " << (j + 1) << " fitness: " << ticks << "\n";
                        break;
                    }
                    ticks++;
                }
            }
            fitness_history[j] = total/3;
        }
        cout << "Generation " << (i + 1) << " average fitness: " << std::accumulate(fitness_history.begin(), fitness_history.end(), 0.0) / childrenInGenerations << "\n";
        if((i+1) % 5== 0 || i == 0){
            cout << "Do you want to see the top performer of this generation? (y/n)\n";
            char input;
            cin >> input;
            if(input == 'y' || input == 'Y'){
                int max_index = static_cast<int>(
                    std::distance(fitness_history.begin(), std::max_element(fitness_history.begin(), fitness_history.end()))
                );
                Entity* top_performer = entities[max_index].get();
                cout << "Top performer fitness: " << fitness_history[max_index] << "\n";
                // Output genes and run a mock simulation
                sim.seed_resources(); // Reseed resources for fair demonstration
                int result;
                sim.set_primary_entity(*top_performer);
                sim.get_primary_entity()->get_biology()->add_energy(1.); 
                sim.get_primary_entity()->get_biology()->add_health(1.);
                sim.get_primary_entity()->get_biology()->add_water(1.);
                for (int l =0; l <100; l++){
                    cout << "Mock Simulation Tick " << l << "\n";
                    result = sim.tick(1);
                    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Slow down for visibility
                    if(result == -1){
                        cout << "Entity died at tick " << l << ".\n";
                        break;
                    }
                }
                cout << "End of mock simulation for top performer of generation " << (i + 1) << "\n";
                cout<< "Genes: \n";
                for (const auto& pair : top_performer->biology_get_genetics()) {
                    cout << pair.first << ": " << pair.second << "\n";
                }
                // Wait for user input before continuing
                cout << "Top performer fitness: " << fitness_history[max_index] << "\n";

                cout << "Press Enter to continue to the next generation...\n";
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                cin.get();
            }       
        }
    }
}

int runSimulation(int numTicks, int autosaveInterval, size_t bufferCapacity, const std::string& saveDir){
// ---- Initialise simulation ----
    Simulation sim;
    sim.initialize();
    std::cout << "\nSimulation setup complete with "
              << sim.get_entity_count() << " entity!" << std::endl;

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
int main(int argc, char* argv[]) {

    // ---- Default configuration ----
    int         numTicks         = 10;
    int         autosaveInterval = 0;       // 0 = autosave disabled
    size_t      bufferCapacity   = 1000;
    std::string saveDir          = "saves";

    // ---- Parse command-line arguments ----
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--ticks" && i + 1 < argc) {
            numTicks = std::stoi(argv[++i]);
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
    if (argc == 1) {
        while (1)
        {
        std::cout << "No command-line arguments were detected. Displaying user interface" << std::endl;
        std::cout << "Main Menu:\n"
                  << "1. Run Alpha Demonstration (100 generations with selection and breeding)\n"
                  << "2. Run Basic Simulation (" << numTicks << " ticks)\n"
                  << "3. Help\n"
                  << "4. Options\n"
                  << "5. Exit\n"
                  << "Please enter your choice (1-5): ";
        int choice;
        std::cin >> choice;
        switch (choice) 
        {
            case 1:
                std::cout << "Running Alpha Demonstration...\n";
                alphaDemonstration();
                break;
            case 2:
                std::cout << "Running Basic Simulation...\n";
                runSimulation(numTicks, autosaveInterval, bufferCapacity, saveDir);
                break;
            case 3:
                print_usage(argv[0]);
                break;
            case 4:
                std::cout << "Options Menu:\n"
                          << "1. Set number of ticks (current: " << numTicks << ")\n"
                          << "2. Set autosave interval (current: " << autosaveInterval << ")\n"
                          << "3. Set buffer capacity (current: " << bufferCapacity << ")\n"
                          << "4. Set save directory (current: " << saveDir << ")\n"
                          << "5. Back to Main Menu\n"
                          << "Please enter your choice (1-5): ";
                int optionChoice;
                std::cin >> optionChoice;
                switch (optionChoice) 
                {     
                    case 1:
                        std::cout << "Enter number of ticks: ";
                        std::cin >> numTicks;
                        break;
                    case 2:
                        std::cout << "Enter autosave interval (0 to disable): ";
                        std::cin >> autosaveInterval;
                        break;
                    case 3:
                        std::cout << "Enter buffer capacity: ";
                        std::cin >> bufferCapacity;
                        break;
                    case 4:
                        std::cout << "Enter save directory: ";
                        std::cin >> saveDir;
                        break;
                    case 5:
                        break;
                    default:
                        std::cout << "Invalid option. Returning to Main Menu.\n";
                        break;
                }
                break;
            
            case 5:
                std::cout << "Exiting program. Goodbye!\n";
                return 0;
            default:
                std::cout << "Invalid choice\n";
                break;
        }
    }
}
    else {
        return runSimulation(numTicks, autosaveInterval, bufferCapacity, saveDir);
    }
return 0;
}
