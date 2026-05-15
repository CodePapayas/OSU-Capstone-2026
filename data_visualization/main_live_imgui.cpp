#include "ImGuiVisualizer.hpp"
#include "../source/Simulation.hpp"
#include "../include/simulation_state.h"
#include "../include/resource_node.h"

#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    Simulation sim;
    sim.initialize();
    cout << "Simulation ready with " << sim.get_entity_count() << " entity.\n";

    ImGuiVisualizer viz;

    bool entityDead = false;
    viz.setTickCallback([&]() {
        if (entityDead) return;

        int result = sim.tick();

        const SimulationState* st = sim.latestState();
        if (st) {
            VisSnapshot snap;
            snap.tick          = st->tick;
            snap.timestamp     = st->timestamp;
            snap.agentCount    = static_cast<int>(st->agentCount);
            snap.resourceCount = static_cast<int>(st->totalResources);
            snap.totalEnergy   = st->totalEnergy;
            snap.avgFitness    = st->averageFitness;
            viz.pushSnapshot(snap);
        }

        if (result == -1) {
            entityDead = true;
            cout << "Entity died at tick " << sim.currentTick()
                 << ". Simulation stopped.\n";
            viz.notifyEntityDied();
        }
    });

    viz.setRestartCallback([&]() {
        sim.reset();
        entityDead = false;
        viz.clearHistory();
        cout << "Simulation restarted with " << sim.get_entity_count() << " entity.\n";
    });

    viz.setWorldMapCallback([&]() -> WorldMapSnapshot {
        WorldMapSnapshot wm;
        wm.gridSize = sim.getWorldSize();

        wm.tileValues.resize(wm.gridSize * wm.gridSize);
        for (int y = 0; y < wm.gridSize; y++)
            for (int x = 0; x < wm.gridSize; x++)
                wm.tileValues[y * wm.gridSize + x] = sim.environGetTileValue(x, y);

        Entity* e = sim.get_primary_entity();
        if (e && !e->dead())
            wm.entityPositions.push_back({e->x, e->y});

        const ResourceManager* rm = sim.getResourceManager();
        if (rm) {
            auto resources = rm->getAllResources();
            for (auto* r : resources) {
                auto pos = r->getPosition();
                if (r->getType() == ResourceType::FOOD)
                    wm.foodPositions.push_back({pos.x, pos.y});
                else if (r->getType() == ResourceType::WATER)
                    wm.waterPositions.push_back({pos.x, pos.y});
            }
        }

        return wm;
    });

    viz.run();

    cout << "Dashboard closed at tick " << sim.currentTick() << ".\n";
    return 0;
}