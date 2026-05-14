#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include "Environment.h"
#include "../decision_center/entity.hpp"
#include "../perception_movement/perception.hpp"

#ifdef ALIFE_USE_DB
class AutoSave;
struct AutoSaveStats;
struct SimulationSavePayload;
#endif
struct SimulationState;
template<typename T> class CircularBuffer;

class Brain;
class Biology;
class ResourceManager;

/**
 * @class Simulation
 * @brief Main simulation controller that initializes and manages the environment, entities, and brains
 */
class Simulation
{
private:
    std::unique_ptr<Environment> _environment;
    std::vector<std::unique_ptr<Entity>> _entities;
    std::unique_ptr<Perception> _perception;
    std::unique_ptr<ResourceManager> _resource_manager;

    uint64_t m_tick = 0;
    std::unique_ptr<CircularBuffer<SimulationState>> m_stateHistory;
    int  m_displayInterval     = 1;   // render every N ticks
    bool m_paused              = false;
    bool m_fastForwardActive   = false;
    int  m_fastForwardInterval = 10;

#ifdef ALIFE_USE_DB
    std::shared_ptr<AutoSave>                        m_autoSave;
    SimulationSavePayload buildSavePayload();
#endif

public:
    /**
     * @brief Constructor that initializes the simulation with default settings
     */
    Simulation();

    /**
     * @brief Destructor for the simulation
     */
    ~Simulation();

    /**
     * @brief Initializes the simulation with environment, entity, and brain. Currently all randos
     */
    void initialize();

    void seed_resources();

    enum DecisionCodes {MOVE_UP=0, MOVE_DOWN=1, MOVE_LEFT=2, MOVE_RIGHT=3, STAY_STILL=4, CONSUME=5};
    /**
     * @brief Returns the value of the tile located at (x,y)
     * @return the float value.
     */
    float environGetTileValue(int x, int y) const;

    /**
     * @brief Returns the perception object for this simulation
     * @return Pointer to the perception object
     */
    std::vector<double> get_perception() const;

    /**
     * @brief Sends perception plus internal state to the entity brain and returns a decision code
     * @return The decision code, or -1 on error
     */
    int pass_perception_to_brain();

    /**
     * @brief Returns the first entity (primary entity). Initial sims will only have 1, but I want to have this in place for when we expand.
     * @return Pointer to the first entity
     */
    Entity* get_primary_entity() const;

    void interpret_decision(int decision_code);

    void execute_movement(int direction);

    int tick();

    /**
     * @brief Returns number of entities in the simulation. Surpisingly helpful in diagnosing bugs.
     * @return The count of entities
     */
    size_t get_entity_count() const;

    /**
     * @brief Tests whether the sim can actually access its members
     * Does not currently test for accuracy, just that communication between modules is possible
     * Also outputs to CLI all info accessed for visual confirmation
     */
    void testAccess();

    /**
     * @brief Sets the coordinates of the primary entity. Should be renamed. Doesn't access bio 
     * @param coords The new coordinates to set
     */
    void biologySetCoordinates(Vector2d coords);

    /**
     * @brief Gets the coordinates of the primary entity. Should be renamed. Doesn't access bio
     * @return The current coordinates of the primary entity
     */
    Vector2d biologyGetCoordinates() const;

    /**
     * @brief Displays the environment to the CLI with color-coded tiles
     * Tiles are colored blue (low values) to red (high values)
     * Entities are displayed as white "X"s
     */
    void display_environment() const;

    /**
     * @brief Gets the vision value of the primary entity, which may be used to filter perception in the future
     * @return The vision value as a float
     */
    float get_vision_value() const;

    std::vector<double> filter_perception(std::vector<double> perception, int tilesToIgnore) const;

#ifdef ALIFE_USE_DB
    void enableAutoSave(std::shared_ptr<AutoSave> autoSave);
    void disableAutoSave();
    const AutoSaveStats* autoSaveStats() const;
#endif

    uint64_t currentTick() const { return m_tick; }

    const SimulationState* latestState() const;
    int getWorldSize() const;
    const ResourceManager* getResourceManager() const;
    void reset();

    // display interval: render grid every N ticks; fast-forward overrides with its own interval
    void setDisplayInterval(int interval);
    int  getDisplayInterval()     const { return m_displayInterval; }

    void setPaused(bool paused);           // tick() returns 2 while paused
    bool isPaused()               const { return m_paused; }

    void setFastForward(bool enabled, int interval = 0); // interval=0 keeps current FF interval
    bool isFastForwarding()       const { return m_fastForwardActive; }
    int  getFastForwardInterval() const { return m_fastForwardInterval; }
};
