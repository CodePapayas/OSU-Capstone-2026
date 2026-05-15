#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>

#include "source/Simulation.hpp"
#include "include/db_connector.h"
#include "include/save_manager.h"
#include "include/auto_save.h"
#include "decision_center/biology.hpp"

// Raw terminal mode for non-blocking single-keypress input (no Enter required)
static struct termios s_origTermios;
static bool           s_rawModeActive = false;

static void restoreTerminal()
{
    if (s_rawModeActive) {
        tcsetattr(STDIN_FILENO, TCSANOW, &s_origTermios);
        s_rawModeActive = false;
    }
}

static void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &s_origTermios);
    std::atexit(restoreTerminal);

    struct termios raw = s_origTermios;
    raw.c_lflag &= ~(static_cast<tcflag_t>(ICANON) | static_cast<tcflag_t>(ECHO));
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    s_rawModeActive = true;
}

static int pollKey()
{
    unsigned char c = 0;
    int n = read(STDIN_FILENO, &c, 1);
    return (n > 0) ? static_cast<int>(c) : -1;
}

int main() {
    Simulation sim;
    sim.initialize();
    std::cout << "\nSimulation setup complete with "
              << sim.get_entity_count() << " entity!\n";

    // Startup parameter prompts (seed presets, entity count, resource density/type, coords, genetics)
    int envSeed = -1;
    int startX = 0;
    int startY = 0;
    bool useDeterministicGenetics = false;
    int initialEntityCount = 1;
    double resourceSpawnProb = 0.1;
    double resourceFoodProb  = 0.5;

    {
        std::string line;

        std::cout << "Resource spawn preset — [1] Sparse (2%), [2] Balanced (10%), [3] Dense (25%), [4] Custom [default 2]: ";
        std::getline(std::cin, line);
        int preset = 2;
        if (!line.empty()) {
            try { preset = std::stoi(line); } catch (...) { preset = 2; }
        }
        if (preset == 1) { resourceSpawnProb = 0.02; resourceFoodProb = 0.5; }
        else if (preset == 2) { resourceSpawnProb = 0.10; resourceFoodProb = 0.5; }
        else if (preset == 3) { resourceSpawnProb = 0.25; resourceFoodProb = 0.5; }
        else {
            std::cout << "Custom spawn probability (0.0-1.0) [0.10]: ";
            std::getline(std::cin, line);
            if (!line.empty()) { try { resourceSpawnProb = std::stod(line); } catch (...) { resourceSpawnProb = 0.10; } }
            std::cout << "Probability resource is FOOD (0.0-1.0) [0.5]: ";
            std::getline(std::cin, line);
            if (!line.empty()) { try { resourceFoodProb = std::stod(line); } catch (...) { resourceFoodProb = 0.5; } }
        }

        // Random seed for reproducibility
        std::cout << "Random seed for environment (blank=generate): ";
        std::getline(std::cin, line);
        if (!line.empty()) {
            try {
                envSeed = static_cast<int>(std::stol(line));
            } catch (...) {
                envSeed = -1;
                std::cout << "Invalid seed entered; a seed will be generated.\n";
            }
        }

        Vector2d cur = sim.biologyGetCoordinates();
        int defX = static_cast<int>(cur.x);
        int defY = static_cast<int>(cur.y);
        std::cout << "Starting X coordinate [" << defX << "]: ";
        std::getline(std::cin, line);
        if (!line.empty()) {
            try { startX = std::stoi(line); } catch (...) { startX = defX; }
        } else startX = defX;
        std::cout << "Starting Y coordinate [" << defY << "]: ";
        std::getline(std::cin, line);
        if (!line.empty()) {
            try { startY = std::stoi(line); } catch (...) { startY = defY; }
        } else startY = defY;

        std::cout << "Number of starting entities [1]: ";
        std::getline(std::cin, line);
        if (!line.empty()) {
            try { initialEntityCount = std::max(1, std::stoi(line)); } catch (...) { initialEntityCount = 1; }
        }

        std::cout << "Use deterministic genetics (no randomness)? [y/N]: ";
        std::getline(std::cin, line);
        if (!line.empty() && (line[0] == 'y' || line[0] == 'Y')) {
            useDeterministicGenetics = true;
        }
    }

    // Apply seed (generate if blank) and set global RNGs for reproducibility
    if (envSeed == -1) {
        unsigned long long now = static_cast<unsigned long long>(std::chrono::system_clock::now().time_since_epoch().count());
        unsigned int genSeed = static_cast<unsigned int>((now ^ std::random_device{}()) & 0xFFFFFFFF);
        envSeed = static_cast<int>(genSeed);
        std::cout << "Generated seed: " << envSeed << " (use this to reproduce)\n";
    } else {
        std::cout << "Using provided seed: " << envSeed << "\n";
    }
    std::srand(static_cast<unsigned int>(envSeed));
    Biology::set_global_seed(static_cast<unsigned int>(envSeed));

    sim.setInitialEntityCount(initialEntityCount);
    sim.setResourceSpawnProbability(resourceSpawnProb);
    sim.setResourceFoodProbability(resourceFoodProb);

    {
        int displayInterval = 1;
        std::cout << "Display frame every X ticks [default 1]: ";
        std::string line;
        std::getline(std::cin, line);
        if (!line.empty()) {
            try {
                int v = std::stoi(line);
                if (v >= 1) displayInterval = v;
            } catch (...) {}
        }
        sim.setDisplayInterval(displayInterval);
        std::cout << "Display interval: every " << displayInterval << " tick(s).\n";
    }

    // Reinitialize simulation so that seed, entities and resource parameters take effect
    sim.reset();
    if (useDeterministicGenetics) {
        for (int i = 0; i < sim.get_entity_count(); ++i) {
            auto e = sim.get_entity(i);
            if (e) {
                e->set_biology(std::make_shared<Biology>(true));
            }
        }
        std::cout << "Deterministic genetics applied to entities.\n";
    }
    // Set requested starting coords on the primary entity if present
    auto primary = sim.get_primary_entity();
    if (primary) {
        sim.biologySetCoordinates(Vector2d(startX, startY));
    }
    std::cout << "Simulation reinitialized with startup parameters.\n";

    AutoSaveConfig cfg;
    cfg.intervalTicks = 3;
    cfg.maxAutoSaves  = 5;
    cfg.slotPrefix    = "autosave";

    auto db = std::make_shared<DBConnector>(DBConnectionParams::fromEnv());
    auto sm = std::make_shared<SaveManager>(db);
    sm->initSchema("db/schema.sql");

    auto autoSave = std::make_shared<AutoSave>(sm, /*loadConfigFromDB=*/false);
    autoSave->configure(cfg);
    sim.enableAutoSave(autoSave);
    std::cout << "Auto-save enabled (every " << cfg.intervalTicks
              << " ticks, max " << cfg.maxAutoSaves << " slots)\n";

    std::cout << "\nRuntime controls:\n"
              << "  [P] — Pause / Resume\n"
              << "  [F] — Toggle Fast Forward  (display every "
              << sim.getFastForwardInterval() << " ticks)\n"
              << "  [Q] — Quit\n\n";

    enableRawMode();

    bool running = true;
    while (running) {
        int key = pollKey();
        switch (key) {
            case 'p': case 'P': {
                bool nowPaused = !sim.isPaused();
                sim.setPaused(nowPaused);
                std::cout << (nowPaused ? "\n[PAUSED — press P to resume]\n"
                                        : "\n[RESUMED]\n");
                break;
            }
            case 'f': case 'F': {
                bool nowFF = !sim.isFastForwarding();
                sim.setFastForward(nowFF);
                if (nowFF)
                    std::cout << "\n[FAST FORWARD ON — display every "
                              << sim.getFastForwardInterval() << " ticks]\n";
                else
                    std::cout << "\n[FAST FORWARD OFF]\n";
                break;
            }
            case 'q': case 'Q':
                running = false;
                continue;
            default:
                break;
        }

        int result = sim.tick();

        if (result == -1) {
            std::cout << "Entity has died — ending simulation.\n";
            running = false;
        } else if (result == 2) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } else {
            const auto* stats = sim.autoSaveStats();
            if (stats && stats->lastAutoSaveTick == sim.currentTick()) {
                std::cout << "[auto-save] slot written at tick "
                          << stats->lastAutoSaveTick
                          << " (total saves: " << stats->totalAutoSavesDone << ")\n";
            }
        }
    }

    restoreTerminal();
    std::cout << "\nFinal tick: " << sim.currentTick() << "\n";
    return 0;
}
