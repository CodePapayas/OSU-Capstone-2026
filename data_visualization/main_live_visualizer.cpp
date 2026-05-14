/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462
*/

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "LiveVisualizer.hpp"
#include "db_connector.h"

int main(int argc, char* argv[]) {
    // Optional CLI args: poll_interval_seconds  history_limit
    int pollInterval  = (argc > 1) ? std::stoi(argv[1]) : 2;
    int historyLimit  = (argc > 2) ? std::stoi(argv[2]) : 60;

    try {
        auto params = DBConnectionParams::fromEnv();
        auto db = std::make_shared<DBConnector>(params);

        if (!db->isConnected()) {
            std::cerr << "Could not connect to the database.\n";
            return 1;
        }

        LiveVisualizer live(db, pollInterval, historyLimit);
        live.run();  // Blocks until Ctrl+C

    } catch (const std::exception& e) {
        std::cerr << "Live visualizer error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
