/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462
*/

#include <iostream>
#include <memory>
#include <stdexcept>
#include "DataVisualizer.hpp"
#include "db_connector.h"

int main() {
    std::cout << "Starting Simulation Data Visualizer...\n";
    
    try {
        // Init the Database Connection based on environment / defaults 
        auto params = DBConnectionParams::fromEnv();
        std::shared_ptr<DBConnector> db = std::make_shared<DBConnector>(params);

        if (!db->isConnected()) {
            std::cerr << "Failed to connect to the visualization database.\n";
            return 1;
        }

        DataVisualizer visualizer(db);
        
        // Grab recent standard chunk of stats
        std::vector<SnapshotData> history = visualizer.fetchHistory(60); 

        if (history.empty()) {
            std::cout << "No historical simulation data found in DB. Run the simulation and auto-save first.\n";
            return 0;
        }

        // Draw simple ASCII charts in the console
        visualizer.plotAgentCount(history);
        visualizer.plotTotalEnergy(history);

        // Dump data as a convenient CSV 
        visualizer.exportToCSV(history, "simulation_stats_out.csv");

        std::cout << "Visualization sequence complete.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error during visualizer execution: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
