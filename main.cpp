#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>

#include "source/Simulation.hpp"
#include "include/db_connector.h"
#include "include/save_manager.h"
#include "include/auto_save.h"

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

    auto db = std::make_shared<DBConnector>(DBConnectionParams::fromEnv());
    auto sm = std::make_shared<SaveManager>(db);
    sm->initSchema("db/schema.sql");

    AutoSaveConfig cfg;
    cfg.intervalTicks = 3;
    cfg.maxAutoSaves  = 5;
    cfg.slotPrefix    = "autosave";

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
