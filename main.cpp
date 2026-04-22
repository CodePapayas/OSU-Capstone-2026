#include <iostream>
#include <memory>

#include "source/Simulation.hpp"
#include "include/db_connector.h"
#include "include/save_manager.h"
#include "include/auto_save.h"

int main() {
    Simulation sim;
    sim.initialize();
    std::cout << "\nSimulation setup complete with "
              << sim.get_entity_count() << " entity!\n";

    // Connect to the database and initialise the schema (idempotent).
    auto db = std::make_shared<DBConnector>(DBConnectionParams::fromEnv());
    auto sm = std::make_shared<SaveManager>(db);
    sm->initSchema("db/schema.sql");

    // Configure auto-save: every 3 ticks, keep the last 5 slots.
    AutoSaveConfig cfg;
    cfg.intervalTicks = 3;
    cfg.maxAutoSaves  = 5;
    cfg.slotPrefix    = "autosave";

    auto autoSave = std::make_shared<AutoSave>(sm, /*loadConfigFromDB=*/false);
    autoSave->configure(cfg);
    sim.enableAutoSave(autoSave);

    std::cout << "Auto-save enabled (every " << cfg.intervalTicks
              << " ticks, max " << cfg.maxAutoSaves << " slots)\n";

    const int numTicks = 10;
    for (int i = 0; i < numTicks; ++i) {
        std::cout << "\n=== Tick " << sim.currentTick() + 1 << " ===\n";
        int result = sim.tick();

        const auto* stats = sim.autoSaveStats();
        if (stats && stats->lastAutoSaveTick == sim.currentTick()) {
            std::cout << "[auto-save] slot written at tick "
                      << stats->lastAutoSaveTick
                      << " (total saves: " << stats->totalAutoSavesDone << ")\n";
        }

        if (result == -1) {
            std::cout << "Entity has died — ending simulation.\n";
            break;
        }
    }

    std::cout << "\nFinal tick: " << sim.currentTick() << "\n";
    return 0;
}
