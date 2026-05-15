#include "Simulation.hpp"
#include "Environment.h"
#include "../decision_center/entity.hpp"
#include "../decision_center/brain.hpp"
#include "../decision_center/biology.hpp"
#include "../perception_movement/perception.hpp"
#include "../perception_movement/movement.hpp"
#include "resource_node.h"

#ifdef ALIFE_USE_DB
#include "../include/auto_save.h"
#endif
#include "../include/circular_buffer.h"
#include "../include/simulation_state.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <format>

static constexpr size_t STATE_HISTORY_CAPACITY = 500;

Simulation::Simulation()
    : _environment(nullptr)
    , m_tick(0)
    , m_stateHistory(std::make_unique<CircularBuffer<SimulationState>>(STATE_HISTORY_CAPACITY))
{
}

Simulation::~Simulation() = default;

void Simulation::initialize()
{
    // Create a new environment
    _environment = std::make_unique<Environment>();
    std::cout << "Environment created successfully!" << std::endl;

    // Create the configured number of entities
    std::vector<int> layer_sizes = {28, 8, 8, 6};
    for (int i = 0; i < m_initialEntityCount; ++i) {
        auto entity = std::make_unique<Entity>();
        std::cout << "Entity created successfully with ID: " << entity->get_id() << std::endl;

        // Random starting coordinates within the world
        int w = getWorldSize();
        int sx = std::rand() % std::max(1, w);
        int sy = std::rand() % std::max(1, w);
        entity->set_coordinates(Vector2d(sx, sy));

        // Create brain and biology for each entity
        auto brain = std::make_shared<Brain>(layer_sizes);
        auto biology = std::make_shared<Biology>(false);  // randomized genetics by default
        entity->set_brain(brain);
        entity->set_biology(biology);
        std::cout << "Entity configured with brain and biology!" << std::endl;
        _entities.push_back(std::move(entity));
    }

    // Add perception to sim class
    _perception = std::make_unique<Perception>();
    std::cout << "Perception module initialized successfully!" << std::endl;

    // Add resource manager
    _resource_manager = std::make_unique<ResourceManager>();
    seed_resources();
    std::cout << "Resource manager initialized successfully!" << std::endl;
}

void Simulation::seed_resources()
{
    // Example of seeding some resources in the environment
    for (int x =0; x < _environment->getChunksInEnvironment() * _environment->getTilesPerChunk(); x += 1) {
        for (int y = 0; y < _environment->getChunksInEnvironment() * _environment->getTilesPerChunk(); y += 1) {
            float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            // spawn with configured probability
            if (randomValue < static_cast<float>(m_resourceSpawnProbability)){
                // choose type based on configured food probability
                float typeChoice = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                ResourceType type = (typeChoice < m_resourceFoodProbability) ? ResourceType::FOOD : ResourceType::WATER;
                double energyValue = static_cast<double>(rand()) / static_cast<double>(RAND_MAX); // Random energy value between 0 and 1
                bool renewable = (rand() % 2) == 0; // Randomly decide if it's renewable
                _resource_manager->createResource(Position(x, y), type, energyValue, renewable);
                std::cout << "Seeded resource at (" << x << ", " << y << ") with energy " << energyValue 
                          << " and type " << (type == ResourceType::FOOD ? "FOOD" : "WATER") 
                          << (renewable ? " (Renewable)" : " (Non-renewable)") << std::endl;
            }
        }
    }
}

float Simulation::environGetTileValue(int x, int y) const
{
    return _environment->getTileValue(Vector2d(x, y));
}

Entity* Simulation::get_primary_entity() const
{
    if (_entities.empty())
    {
        return nullptr;
    }
    return _entities[0].get();
}

Entity* Simulation::get_entity(int index) const
{
    if (index < 0) return nullptr;
    if (static_cast<size_t>(index) >= _entities.size()) return nullptr;
    return _entities[static_cast<size_t>(index)].get();
}

std::vector<double> Simulation::get_perception() const
{
    Perception::SensoryInput val = _perception->perceive_local_tiles(
        get_primary_entity()->get_coordinates().x,
        get_primary_entity()->get_coordinates().y,
        *_environment,
        2 //(4 * get_primary_entity()->biology_get_genetic_value("Vision")) // Default perception radius
    );
    return val.tile_values;
}

int Simulation::pass_perception_to_brain()
{
    auto entity = get_primary_entity();
    if (!entity)
    {
        std::cerr << "No primary entity found for perception to brain!" << std::endl;
        return -1; // Indicate an error
    }
    // Get the value of all tiles
    std::vector<double> perception = get_perception();

    // Get the strength of the entities vision and determine how many tiles to ignore
    float vision_value = entity->biology_get_genetic_value("Vision");
    int tilesToIgnore = std::max(static_cast<int>(25.0 - (25 * vision_value)), 1); // at max vision (1.0), ignore 0 tiles, at min vision (0.0) ignore 24 tiles (only sees own tile) 
    
    // Get the filtered values based on the vision strength and add intetnal metrics
    std::vector<double> filteredPerception = filter_perception(perception, tilesToIgnore); // Start with just the tile values
    filteredPerception.push_back(entity->biology_get_metrics()["Energy"]);
    filteredPerception.push_back(entity->biology_get_metrics()["Health"]);
    filteredPerception.push_back(entity->biology_get_metrics()["Water"]);

    // Get the decision from the brain
    int decision = entity->brain_get_decision(filteredPerception);
    return decision;
}

void Simulation::interpret_decision(int decision_code)
{
    switch (static_cast<DecisionCodes>(decision_code))
    {
        case DecisionCodes::MOVE_UP:
            std::cout << "Entity moves up." << std::endl;
            Simulation::execute_movement(decision_code);
            break;
        case DecisionCodes::MOVE_DOWN:
            std::cout << "Entity moves down." << std::endl;
            Simulation::execute_movement(decision_code);
            break;
        case DecisionCodes::MOVE_LEFT:
            std::cout << "Entity moves left." << std::endl;
            Simulation::execute_movement(decision_code);    
            break;
        case DecisionCodes::MOVE_RIGHT:
            std::cout << "Entity moves right." << std::endl;
            Simulation::execute_movement(decision_code);
            break;
        case DecisionCodes::STAY_STILL:
            std::cout << "Entity stays still." << std::endl;
            // Logic for the entity staying still would go here (probably nothing)
            break;
        case DecisionCodes::CONSUME:
            std::cout << "Entity consumes resources." << std::endl;
            // Logic for consuming resources in current tile would go here
            break;
        default:
            std::cerr << "Unknown decision code: " << decision_code << std::endl;
    }
}

void Simulation::execute_movement(int direction){
    // Placeholder for movement execution logic based on the direction decided by the brain
    // This would involve updating the entity's coordinates and applying any relevant energy costs or terrain effects

    // First create a movement struct
    Movement::Action action = Movement::direction_to_action(static_cast<Movement::Direction>(direction), 0); // For now, keeping base energy at 0
    auto entity = get_primary_entity();
    int prev_x = entity->x;
    int prev_y = entity->y;
    // Fetch the new coordinates and update the entity's position
    std::vector<int> new_coords = Movement::execute_movement_wraparound(entity->x, entity->y, action, chunk_amt * tile_amt, chunk_amt * tile_amt, entity->biology_get_metrics()["Energy"]);
    entity->set_coordinates(Vector2d(new_coords[0], new_coords[1]));
    std::cout << "Entity moved from (" << prev_x << ", " << prev_y << ") to (" << entity->x << ", " << entity->y << ")" << std::endl;
    // Need to drain energy based on the terrain type of the new tile and the entity's biology
    //entity->biology_movement(_environment->getTileType(Vector2d(entity->x, entity->y))); // Something like this in practice
    entity->biology_movement("Terrain 1"); // Placeholder until we have actual terrain types implemented
}   

int Simulation::tick(){
    if (m_paused) return 2; // Paused — don't advance; caller should wait and retry

    ++m_tick;

    // Get the perception for the primary entity and pass it to the brain to get a decision
    std::vector<double> perception = get_perception();
    int decision = pass_perception_to_brain();
    interpret_decision(decision);
    get_primary_entity()->update_biology(); // Handle biology updates like energy drain, health regen, etc.

    // Only render the environment grid at the configured display interval.
    // In fast-forward mode use the (larger) fast-forward interval.
    int activeInterval = m_fastForwardActive ? m_fastForwardInterval : m_displayInterval;
    if (m_tick % activeInterval == 0) {
        display_environment();
    }

    bool entity_dead = get_primary_entity()->biology_check_death();
    if (entity_dead) {
        std::cout << "Entity has died. Ending simulation." << std::endl;
        // In a more complex simulation, we might want to remove the entity and continue
        return -1;
    }

    // Snapshot this tick for the rolling state history used by auto-save and rewind.
    SimulationState state;
    state.tick              = m_tick;
    state.timestamp         = static_cast<double>(std::time(nullptr));
    state.agentCount        = static_cast<uint32_t>(_entities.size());
    state.totalResources    = static_cast<uint32_t>(_resource_manager->getResourceCount());

    double totalEnergy = 0.0;
    for (const auto& ent : _entities) {
        auto metrics = ent->biology_get_metrics();
        if (metrics.count("Energy"))
            totalEnergy += metrics.at("Energy");
    }
    state.totalEnergy        = totalEnergy;
    state.averageAgentEnergy = _entities.empty() ? 0.0 : totalEnergy / _entities.size();
    m_stateHistory->push(state);

    // Let the auto-save system decide whether this tick warrants a save.
#ifdef ALIFE_USE_DB
    if (m_autoSave) {
        m_autoSave->tick(m_tick, [this]() { return buildSavePayload(); });
    }
#endif

    return 0; // Return 0 to indicate the tick completed successfully
}

size_t Simulation::get_entity_count() const
{
    return _entities.size();
}

#ifdef ALIFE_USE_DB
void Simulation::enableAutoSave(std::shared_ptr<AutoSave> autoSave)
{
    m_autoSave = std::move(autoSave);
}

void Simulation::disableAutoSave()
{
    m_autoSave.reset();
}

const AutoSaveStats* Simulation::autoSaveStats() const
{
    if (!m_autoSave) return nullptr;
    return &m_autoSave->stats();
}
#endif

const SimulationState* Simulation::latestState() const
{
    if (m_stateHistory->empty()) return nullptr;
    return &m_stateHistory->latest();
}

int Simulation::getWorldSize() const
{
    return chunk_amt * tile_amt;
}

const ResourceManager* Simulation::getResourceManager() const
{
    return _resource_manager.get();
}

void Simulation::reset()
{
    _entities.clear();
    _environment.reset();
    _resource_manager.reset();
    _perception.reset();
    m_tick = 0;
    m_stateHistory = std::make_unique<CircularBuffer<SimulationState>>(STATE_HISTORY_CAPACITY);
    initialize();
}

void Simulation::setDisplayInterval(int interval)
{
    m_displayInterval = std::max(1, interval);
}

void Simulation::setPaused(bool paused)
{
    m_paused = paused;
}

void Simulation::setFastForward(bool enabled, int interval)
{
    m_fastForwardActive = enabled;
    if (interval > 0)
        m_fastForwardInterval = interval;
}

void Simulation::setInitialEntityCount(int count)
{
    m_initialEntityCount = std::max(1, count);
}

void Simulation::setResourceSpawnProbability(double prob)
{
    if (prob < 0.0) prob = 0.0;
    if (prob > 1.0) prob = 1.0;
    m_resourceSpawnProbability = prob;
}

void Simulation::setResourceFoodProbability(double prob)
{
    if (prob < 0.0) prob = 0.0;
    if (prob > 1.0) prob = 1.0;
    m_resourceFoodProbability = prob;
}

#ifdef ALIFE_USE_DB
SimulationSavePayload Simulation::buildSavePayload()
{
    SimulationSavePayload payload;
    payload.tick           = m_tick;
    payload.realTimestamp  = static_cast<double>(std::time(nullptr));
    payload.worldWidth     = _environment->getChunksInEnvironment()
                           * _environment->getTilesPerChunk();
    payload.worldHeight    = payload.worldWidth;
    payload.stateHistory   = m_stateHistory.get();

    double totalEnergy = 0.0;
    for (const auto& ent : _entities) {
        AgentSaveData agent;
        agent.agentId = static_cast<uint64_t>(ent->get_id());
        agent.posX    = static_cast<int32_t>(ent->x);
        agent.posY    = static_cast<int32_t>(ent->y);

        auto metrics = ent->biology_get_metrics();
        if (metrics.count("Energy"))    agent.energy    = metrics.at("Energy");
        if (metrics.count("Max Energy")) agent.maxEnergy = metrics.at("Max Energy");
        if (metrics.count("Health"))    agent.fitness   = metrics.at("Health");

        totalEnergy += agent.energy;
        payload.agents.push_back(std::move(agent));
    }

    payload.totalEnergy = totalEnergy;
    payload.resources   = _resource_manager->getAllResources();

    return payload;
}
#endif


std::vector<double> Simulation::filter_perception(std::vector<double> perception, int tilesToIgnore) const
{
    if (perception.empty() || tilesToIgnore <= 0)
    {
        return perception;
    }

    int tile_count = static_cast<int>(perception.size());
    int tail_count = 0;
    int grid_size = static_cast<int>(std::sqrt(tile_count));

    if (tile_count >= 3)
    {
        tail_count = 3;
        tile_count -= 3;
    }
    
    if (grid_size % 2 == 0)
    {
        return perception;
    }

    int radius = grid_size / 2;
    int ignore_count = std::min(tilesToIgnore, tile_count > 0 ? tile_count - 1 : 0);
    std::vector<bool> ignore(tile_count, false);
    int ignored = 0;

    auto mark_tile = [&](int rx, int ry) {
        if (ignored >= ignore_count)
        {
            return;
        }
        int x = rx + radius;
        int y = ry + radius;
        if (x < 0 || y < 0 || x >= grid_size || y >= grid_size)
        {
            return;
        }
        int idx = x * grid_size + (grid_size - 1 - y);
        if (idx < tile_count && !ignore[idx])
        {
            ignore[idx] = true;
            ++ignored;
        }
    };

    for (int r = radius; r >= 1 && ignored < ignore_count; --r)
    {
        // Bottom edge: (0,-r), (-1,-r), (1,-r), ..., (-r,-r), (r,-r)
        mark_tile(0, -r);
        for (int i = 1; i <= r && ignored < ignore_count; ++i)
        {
            mark_tile(-i, -r);
            mark_tile(i, -r);
        }

        // Vertical edges: (-r, y), (r, y) for y = -r+1..r-1
        for (int y = -r + 1; y <= r - 1 && ignored < ignore_count; ++y)
        {
            mark_tile(-r, y);
            mark_tile(r, y);
        }

        // Top edge: (-r,r), (r,r), (-(r-1),r), ((r-1),r), ..., (-1,r), (1,r), (0,r)
        mark_tile(-r, r);
        mark_tile(r, r);
        for (int i = r - 1; i >= 1 && ignored < ignore_count; --i)
        {
            mark_tile(-i, r);
            mark_tile(i, r);
        }
        mark_tile(0, r);
    }

    if (ignored < ignore_count)
    {
        mark_tile(0, 0);
    }

    std::vector<double> filtered;
    filtered.reserve((tile_count - ignored + tail_count));
    for (int i = 0; i < tile_count; ++i)
    {
        if (!ignore[i])
        {
            filtered.push_back(perception[i]);
        }
    }

    if (tail_count > 0)
    {
        filtered.insert(filtered.end(), perception.end() - tail_count, perception.end());
    }

    return filtered;
}

void Simulation::biologySetCoordinates(Vector2d coords)
{
    auto entity = get_primary_entity();
    if (entity)
    {
        entity->set_coordinates(coords);
        std::cout << "Entity location set to (" << coords.x << ", " << coords.y << ")" << std::endl;
    }
    else
    {
        std::cerr << "No primary entity found to set coordinates!" << std::endl;
    }
}

Vector2d Simulation::biologyGetCoordinates() const
{
    auto entity = get_primary_entity();
    if (entity)
    {
        return entity->get_coordinates();
    }
    else
    {
        return Vector2d(0, 0); // Return default coordinates if no entity is found
    }
}

void Simulation::display_environment() const
{
    if (!_environment)
    {
        std::cerr << "Environment not initialized!" << std::endl;
        return;
    }

    int grid_size = chunk_amt * tile_amt;
    Vector2d entity_pos = get_primary_entity() ? get_primary_entity()->get_coordinates() : Vector2d(-1, -1);

    std::cout << "\n=== Environment Display ===\n" << std::endl;
    // Normal 2d traversal of the environment grid
    for (int y = 0; y < grid_size; ++y)
    {
        for (int x = 0; x < grid_size; ++x)
        {
            float tile_value = _environment->getTileValue(Vector2d(x, y));
            // Check if an entity is at this location, probably a better way down the line
            if (entity_pos.x == x && entity_pos.y == y)
            {
                // Display entity as white x
                std::cout << "\033[97m"  // White color
                          << "  X "
                          << "\033[0m"; // Reset color
            }
            else if(!_resource_manager->findResourcesInRange(Position(x, y), 0).empty()) // Check if there's a resource at this location
            {
                // Display resource as green R
                std::cout << "\033[92m"  // Green color
                          << "  R "
                          << "\033[0m"; // Reset color

            }
            else
            {
                // Some hacky color coding adopted from another project.
                int r, g, b;
                if (tile_value < 0.5)
                {
                    // Blue to Cyan (0.0 to 0.5)
                    float normalized = tile_value * 2; // 0.0 to 1.0
                    b = 255;
                    g = (int)(normalized * 255);
                    r = 0;
                }
                else
                {
                    // Cyan to Red (0.5 to 1.0 Gets kinda dicey around.5, but it is what it is)
                    float normalized = (tile_value - 0.5) * 2; // 0.0 to 1.0
                    r = (int)(normalized * 255);
                    g = (int)((1 - normalized) * 255);
                    b = (int)((1 - normalized) * 255);
                }
                //std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m"
                //          << "O "
                //          << "\033[0m";
                // Alternative print method, prints the value instead of 0
                char buffer[20];
                std::sprintf(buffer,"%.1f", tile_value); 
                std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m"
                          << buffer
                          << " "
                          << "\033[0m"; 
            }
        }
        std::cout << std::endl;
    }
    std::cout << "\n=== End Environment Display ===\n" << std::endl;
}

float Simulation::get_vision_value() const
{
    auto entity = get_primary_entity();
    if (entity)
    {
        return entity->biology_get_genetic_value("Vision");
    }
    else
    {
        std::cerr << "No primary entity found to get vision value!" << std::endl;
        return 0.0f; // Default vision value if no entity is found
    }
  } 

void Simulation::testAccess()
{
    std::cout << "Testing access to environment and entity..." << std::endl;
    float val = environGetTileValue(0, 0);
    if (val)
    {
        std::cout << "Tile at 0,0 has value: " << val << std::endl;
    }
    else
    {
        std::cerr << "Access failed to the environment!" << std::endl;
    }
    auto entity = get_primary_entity();
    if (entity)
    {
        std::cout << "Entity accessed successfully with ID: " << entity->get_id() << std::endl;
        auto metrics = entity->biology_get_metrics();
        for (const auto& pair : metrics){
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
        auto genetics = entity->biology_get_genetics();
        for (const auto& pair : genetics){
            std::cout << pair.first << ": " << pair.second << std::endl;   
        }
        auto brain = entity->brain();
        std::vector<double> inputs = {0.5, 0.2, 0.1, 0.9};
        auto decision = entity->brain_get_decision(inputs);
        std::cout << "Brain decision made successfully: " << std::any_cast<int>(decision) << std::endl;
    }
    else
    {
        std::cerr << "Couldn't find a boy, did you initialize the simulation properly?" << std::endl;
    }
}