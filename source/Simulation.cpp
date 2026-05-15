#include "Simulation.hpp"
#include "Environment.h"
#include "../decision_center/entity.hpp"
#include "../decision_center/brain.hpp"
#include "../decision_center/biology.hpp"
#include "../decision_center/mutate.hpp"
#include "../decision_center/biology_constants.hpp"
#include "../perception_movement/perception.hpp"
#include "../perception_movement/movement.hpp"
#include "resource_node.h"

#ifdef ALIFE_USE_DB
#include "../include/auto_save.h"
#endif
#include "../include/circular_buffer.h"
#include "../include/simulation_state.h"

#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <thread>
#include <unordered_set>
#include <vector>

static constexpr size_t STATE_HISTORY_CAPACITY = 500;
static constexpr int GRID_SIZE = chunk_amt * tile_amt;

Simulation::Simulation()
    : _environment(nullptr)
    , m_tick(0)
    , m_stateHistory(std::make_unique<CircularBuffer<SimulationState>>(STATE_HISTORY_CAPACITY))
{
}

Simulation::~Simulation() = default;

void Simulation::initialize(int num_entities)
{
    num_entities = std::max(1, num_entities);

    _environment = std::make_unique<Environment>();
    std::cout << "Environment created successfully!" << std::endl;

    std::vector<int> layer_sizes = {128, 200, 200, 8};
    for (int i = 0; i < num_entities; ++i) {
        auto entity = std::make_unique<Entity>();
        entity->set_coordinates(Vector2d(rand() % GRID_SIZE, rand() % GRID_SIZE));
        entity->set_brain(std::make_shared<Brain>(layer_sizes));
        entity->set_biology(std::make_shared<Biology>(false));
        _entities.push_back(std::move(entity));
    }
    std::cout << "Spawned " << num_entities << " entities." << std::endl;

    _perception = std::make_unique<Perception>();
    std::cout << "Perception module initialized successfully!" << std::endl;

    _resource_manager = std::make_unique<ResourceManager>();
    seed_resources();
    std::cout << "Resource manager initialized successfully!" << std::endl;
}

void Simulation::seed_resources()
{
    _resource_manager->clear();
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            if (randomValue > 0.9f) {
                ResourceType type = static_cast<ResourceType>(rand() % 2);
                double energyValue = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
                bool renewable = (rand() % 2) == 0;
                _resource_manager->createResource(Position(x, y), type, energyValue, renewable);
            }
        }
    }
}

float Simulation::environGetTileValue(int x, int y) const
{
    return _environment->getTileValue(Vector2d(x, y));
}

static constexpr uint64_t REPRO_COOLDOWN = 25;
static constexpr uint64_t MIN_REPRO_AGE  = REPRO_COOLDOWN;

Entity* Simulation::reproduce(Entity* p1, Entity* p2)
{
    if (p1 == nullptr || p2 == nullptr) return nullptr;
    if ((m_tick - p1->birth_tick) < MIN_REPRO_AGE)  return nullptr;
    if ((m_tick - p2->birth_tick) < MIN_REPRO_AGE)  return nullptr;
    if ((m_tick - p1->last_repro_tick) < REPRO_COOLDOWN) return nullptr;
    if ((m_tick - p2->last_repro_tick) < REPRO_COOLDOWN) return nullptr;

    // Hard block: parent-child pairings
    if (p1->child_ids.count(p2->get_id()) || p2->child_ids.count(p1->get_id())) return nullptr;

    // Sibling check: shared parent → inbreeding path
    bool is_inbred = false;
    for (long long pid : p1->parent_ids) {
        if (p2->parent_ids.count(pid)) { is_inbred = true; break; }
    }
    uint32_t child_inbreeding_gen = is_inbred
        ? std::max(p1->inbreeding_gen, p2->inbreeding_gen) + 1
        : 0;

    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(nullptr);

    Entity* brainParent = (rand() % 2 == 0) ? p1 : p2;
    std::unordered_map<std::string, double> p1_genetics = p1->biology_get_genetics();
    std::unordered_map<std::string, double> p2_genetics = p2->biology_get_genetics();
    std::unordered_map<std::string, double> child_genetics;
    for (const auto& pair : p1_genetics) {
        double chosen_val = (rand() % 2 == 0) ? pair.second : p2_genetics[pair.first];
        child_genetics[pair.first] = chosen_val;
    }
    child_genetics = is_inbred
        ? mutate_genetics_inbred(child_genetics, child_inbreeding_gen)
        : mutate_genetics(child_genetics);

    Entity* child = new Entity();
    child->set_biology(std::make_shared<Biology>(false));
    child->get_biology()->set_genetic_vals(child_genetics);
    child->set_brain(std::make_shared<Brain>(*brainParent->get_brain()));
    std::vector<ActivationLayerReLU>& parent_layers = brainParent->get_brain()->get_layers();
    int layer_index = 0;
    for (auto& layer : parent_layers) {
        std::vector<double> mutated_weights = is_inbred
            ? mutate_vector_inbred(layer.get_weights(), child_inbreeding_gen)
            : mutate_vector(layer.get_weights());
        std::vector<double> mutated_biases = is_inbred
            ? mutate_vector_inbred(layer.get_biases(), child_inbreeding_gen)
            : mutate_vector(layer.get_biases());
        child->get_brain()->get_layers()[layer_index].ActivationLayerReLUOffsping(mutated_weights, mutated_biases);
        ++layer_index;
    }
    child->set_coordinates(Vector2d(rand() % GRID_SIZE, rand() % GRID_SIZE));

    child->birth_tick      = m_tick;
    child->inbreeding_gen  = child_inbreeding_gen;
    child->parent_ids      = {p1->get_id(), p2->get_id()};

    p1->last_repro_tick = m_tick;
    p2->last_repro_tick = m_tick;
    p1->child_ids.insert(child->get_id());
    p2->child_ids.insert(child->get_id());

    p1->biology_rem_energy(5.0 * p1->biology_energy_drain_rate());
    p2->biology_rem_energy(5.0 * p2->biology_energy_drain_rate());

    _entities.push_back(std::unique_ptr<Entity>(child));

    std::cout.rdbuf(originalCoutBuffer);
    return child;
}

void Simulation::set_primary_entity(const Entity& entity)
{
    _entities.clear();
    auto cloned = std::make_unique<Entity>();
    cloned->set_coordinates(Vector2d(rand() % GRID_SIZE, rand() % GRID_SIZE));
    if (entity.get_brain())   cloned->set_brain(std::make_unique<Brain>(*entity.get_brain()));
    if (entity.get_biology()) cloned->set_biology(std::make_unique<Biology>(*entity.get_biology()));
    cloned->get_biology()->add_energy(1.0);
    cloned->get_biology()->add_health(1.0);
    cloned->get_biology()->add_water(1.0);
    _entities.push_back(std::move(cloned));
}

void Simulation::set_primary_entity_random()
{
    _entities.clear();
    auto entity = std::make_unique<Entity>();
    entity->set_coordinates(Vector2d(rand() % GRID_SIZE, rand() % GRID_SIZE));
    std::vector<int> layer_sizes = {128, 200, 200, 8};
    entity->set_brain(std::make_shared<Brain>(layer_sizes));
    entity->set_biology(std::make_shared<Biology>(false));
    _entities.push_back(std::move(entity));
}

Entity* Simulation::get_primary_entity() const
{
    if (_entities.empty() || _current_entity_index >= (int)_entities.size())
        return nullptr;
    return _entities[_current_entity_index].get();
}

std::vector<double> Simulation::get_perception() const
{
    Perception::SensoryInput val = _perception->perceive_local_tiles(
        get_primary_entity()->get_coordinates().x,
        get_primary_entity()->get_coordinates().y,
        *_environment,
        2
    );
    return val.tile_values;
}

std::vector<double> Simulation::get_perception_expanded(const std::string& type) const
{
    Perception::SensoryInput val = _perception->perceive_local_tiles(
        get_primary_entity()->get_coordinates().x,
        get_primary_entity()->get_coordinates().y,
        *_environment,
        2
    );
    return val.tile_values;
}

int Simulation::pass_perception_to_brain()
{
    auto entity = get_primary_entity();
    if (!entity) {
        std::cerr << "No primary entity found for perception to brain!" << std::endl;
        return -1;
    }

    std::vector<double> filteredPerception;
    const std::vector<std::string> types = {"FOOD", "WATER", "TERRAIN_1", "TERRAIN_2", "TERRAIN_3"};
    float vision_value = entity->biology_get_genetic_value("Vision");
    int tilesToIgnore = std::max(static_cast<int>(25.0 - (25 * vision_value)), 1);

    for (const auto& type_str : types) {
        std::vector<double> perception = get_perception_expanded(type_str);
        std::vector<double> adapted = filter_perception(perception, tilesToIgnore);
        filteredPerception.insert(filteredPerception.end(), adapted.begin(), adapted.end());
    }
    filteredPerception.push_back(entity->biology_get_metrics()["Energy"]);
    filteredPerception.push_back(entity->biology_get_metrics()["Health"]);
    filteredPerception.push_back(entity->biology_get_metrics()["Water"]);

    if (_debug)
        std::cout << "Filtered Perception Length: " << filteredPerception.size() << std::endl;

    return entity->brain_get_decision(filteredPerception);
}

void Simulation::interpret_decision(int decision_code)
{
    switch (static_cast<DecisionCodes>(decision_code)) {
        case DecisionCodes::MOVE_UP:
        case DecisionCodes::MOVE_DOWN:
        case DecisionCodes::MOVE_LEFT:
        case DecisionCodes::MOVE_RIGHT:
            Simulation::execute_movement(decision_code); break;
        case DecisionCodes::STAY_STILL:
            break;
        case DecisionCodes::CONSUME:
            Simulation::consumption(); break;
        case DecisionCodes::REPRODUCE: {
            Entity* parent1 = get_primary_entity();
            int vision_radius = std::max(2, static_cast<int>(4 * parent1->biology_get_genetic_value("Vision")));
            Entity* parent2 = nullptr;
            for (int j = 0; j < (int)_entities.size(); ++j) {
                if (j == _current_entity_index || _entities[j]->biology_check_death()) continue;
                int dx = std::abs(_entities[j]->get_coordinates().x - parent1->get_coordinates().x);
                int dy = std::abs(_entities[j]->get_coordinates().y - parent1->get_coordinates().y);
                if (std::max(dx, dy) <= vision_radius) { parent2 = _entities[j].get(); break; }
            }
            if (parent2 != nullptr) reproduce(parent1, parent2);
            break;
        }
        case DecisionCodes::SLEEP: {
            Entity* entity = get_primary_entity();
            if (entity->sleep_ticks_remaining == 0) {
                static std::mt19937 sleep_rng(std::random_device{}());
                std::uniform_real_distribution<float> interrupt_dist(0.02f, 0.05f);
                std::uniform_real_distribution<float> bad_sleep_dist(0.0f, 1.0f);
                entity->sleep_ticks_remaining  = 4;
                entity->sleep_interrupt_chance = interrupt_dist(sleep_rng);
                entity->sleep_regen_total      = (bad_sleep_dist(sleep_rng) < 0.20f) ? 0.16f : 0.32f;
            }
            break;
        }
        default: break;
    }
}

void Simulation::execute_movement(int direction)
{
    Movement::Action action = Movement::direction_to_action(static_cast<Movement::Direction>(direction), 0);
    auto entity = get_primary_entity();
    int prev_x = entity->x;
    int prev_y = entity->y;
    std::vector<int> new_coords = Movement::execute_movement_wraparound(
        entity->x, entity->y, action, GRID_SIZE, GRID_SIZE, entity->biology_get_metrics()["Energy"]);
    if (new_coords[0] >= GRID_SIZE || new_coords[1] >= GRID_SIZE || new_coords[0] < 0 || new_coords[1] < 0) {
        std::cerr << "Movement out of bounds (" << new_coords[0] << ", " << new_coords[1] << ")" << std::endl;
        return;
    }
    entity->set_coordinates(Vector2d(new_coords[0], new_coords[1]));
    entity->biology_movement("Terrain 1");
    auto nearby = _resource_manager->findResourcesInRange(Position(entity->x, entity->y), 0);
    if (!nearby.empty() && nearby[0]->getType() == ResourceType::WATER) {
        double mass = entity->biology_get_genetic_value("Mass");
        double thirst_threshold = 0.2 * (1.0 + mass);
        if (entity->biology_get_metrics()["Water"] < thirst_threshold) {
            double waterGained = nearby[0]->consume(mass);
            entity->biology_drink(waterGained);
        }
    }
}

void Simulation::consumption()
{
    Entity* entity = get_primary_entity();
    auto nearby = _resource_manager->findResourcesInRange(Position(entity->x, entity->y), 0);
    if (!nearby.empty()) {
        double gained = nearby[0]->consume(entity->biology_get_genetic_value("Mass"));
        if (nearby[0]->getType() == ResourceType::FOOD)
            entity->biology_eat(gained);
        else if (nearby[0]->getType() == ResourceType::WATER)
            entity->biology_drink(gained);
    }
}

int Simulation::tick()
{
    if (m_paused) return 2;

    ++m_tick;

    for (int i = 0; i < (int)_entities.size(); ++i) {
        _current_entity_index = i;
        if (_entities[i]->biology_check_death()) continue;

        Entity* ent = _entities[i].get();

        // Process in-progress sleep before brain gets control
        if (ent->sleep_ticks_remaining > 0) {
            static std::mt19937 sleep_tick_rng(std::random_device{}());
            std::uniform_real_distribution<float> roll(0.0f, 1.0f);
            bool interrupted = roll(sleep_tick_rng) < ent->sleep_interrupt_chance;
            uint8_t ticks_completed = 4 - ent->sleep_ticks_remaining + 1;
            if (interrupted || ent->sleep_ticks_remaining == 1) {
                float regen = (static_cast<float>(ticks_completed) / 4.0f) * ent->sleep_regen_total;
                ent->get_biology()->add_energy(static_cast<double>(regen));
                ent->sleep_ticks_remaining  = 0;
                ent->sleep_interrupt_chance = 0.0f;
                ent->sleep_regen_total      = 0.0f;
            } else {
                --ent->sleep_ticks_remaining;
            }
            ent->update_biology();
            continue;
        }

        int decision;
        if (m_godMode) {
            if (m_tick % 25 == 0)      decision = REPRODUCE;
            else if (m_tick % 8 == 0)  decision = CONSUME;
            else                       decision = static_cast<int>(m_tick % 4);
        } else {
            decision = pass_perception_to_brain();
        }
        interpret_decision(decision);
        _entities[i]->update_biology();

        if (m_godMode) {
            auto bio = _entities[i]->get_biology();
            bio->add_energy(1.0);
            bio->add_health(1.0);
            bio->add_water(1.0);
        }
    }
    _current_entity_index = 0;

    int alive = 0;
    for (const auto& e : _entities) {
        if (!e->biology_check_death()) ++alive;
    }

    int activeInterval = m_fastForwardActive ? m_fastForwardInterval : m_displayInterval;
    if (m_tick % activeInterval == 0) {
        // Save previous positions for trail rendering
        _prev_entity_positions.clear();
        for (const auto& kv : _entity_pos_map)
            _prev_entity_positions.insert(kv.first);

        _entity_pos_map.clear();
        for (const auto& e : _entities) {
            if (!e->biology_check_death())
                _entity_pos_map[e->get_coordinates().y * GRID_SIZE + e->get_coordinates().x] = e.get();
        }
        display_environment(alive);
    }

    // Snapshot state
    SimulationState state;
    state.tick              = m_tick;
    state.timestamp         = static_cast<double>(std::time(nullptr));
    state.agentCount        = static_cast<uint32_t>(alive);
    state.totalResources    = static_cast<uint32_t>(_resource_manager->getResourceCount());
    double totalEnergy = 0.0;
    for (const auto& ent : _entities) {
        auto metrics = ent->biology_get_metrics();
        if (metrics.count("Energy")) totalEnergy += metrics.at("Energy");
    }
    state.totalEnergy        = totalEnergy;
    state.averageAgentEnergy = alive > 0 ? totalEnergy / alive : 0.0;
    m_stateHistory->push(state);

#ifdef ALIFE_USE_DB
    if (m_autoSave)
        m_autoSave->tick(m_tick, [this]() { return buildSavePayload(); });
#endif

    if (!m_godMode && alive == 0) {
        std::cout << "\033[H\033[2J"; // clear on exit
        std::cout << "All entities have died at tick " << m_tick << ".\n";
        return -1;
    }
    return 0;
}

size_t Simulation::get_entity_count() const { return _entities.size(); }

#ifdef ALIFE_USE_DB
void Simulation::enableAutoSave(std::shared_ptr<AutoSave> autoSave) { m_autoSave = std::move(autoSave); }
void Simulation::disableAutoSave() { m_autoSave.reset(); }
const AutoSaveStats* Simulation::autoSaveStats() const {
    if (!m_autoSave) return nullptr;
    return &m_autoSave->stats();
}
#endif

const SimulationState* Simulation::latestState() const {
    if (m_stateHistory->empty()) return nullptr;
    return &m_stateHistory->latest();
}

int Simulation::getWorldSize() const { return GRID_SIZE; }
const ResourceManager* Simulation::getResourceManager() const { return _resource_manager.get(); }

void Simulation::reset() {
    _entities.clear();
    _environment.reset();
    _resource_manager.reset();
    _perception.reset();
    m_tick = 0;
    m_stateHistory = std::make_unique<CircularBuffer<SimulationState>>(STATE_HISTORY_CAPACITY);
    initialize();
}

void Simulation::setDisplayInterval(int interval) { m_displayInterval = std::max(1, interval); }
void Simulation::setPaused(bool paused) { m_paused = paused; }
void Simulation::setFastForward(bool enabled, int interval) {
    m_fastForwardActive = enabled;
    if (interval > 0) m_fastForwardInterval = interval;
}

#ifdef ALIFE_USE_DB
SimulationSavePayload Simulation::buildSavePayload()
{
    SimulationSavePayload payload;
    payload.tick          = m_tick;
    payload.realTimestamp = static_cast<double>(std::time(nullptr));
    payload.worldWidth    = GRID_SIZE;
    payload.worldHeight   = GRID_SIZE;
    payload.stateHistory  = m_stateHistory.get();

    double totalEnergy = 0.0;
    for (const auto& ent : _entities) {
        AgentSaveData agent;
        agent.agentId = static_cast<uint64_t>(ent->get_id());
        agent.posX    = static_cast<int32_t>(ent->x);
        agent.posY    = static_cast<int32_t>(ent->y);
        auto metrics  = ent->biology_get_metrics();
        if (metrics.count("Energy"))     agent.energy    = metrics.at("Energy");
        if (metrics.count("Max Energy")) agent.maxEnergy = metrics.at("Max Energy");
        if (metrics.count("Health"))     agent.fitness   = metrics.at("Health");
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
    if (perception.empty() || tilesToIgnore <= 0) return perception;

    int tile_count = static_cast<int>(perception.size());
    int grid_size  = static_cast<int>(std::sqrt(tile_count));
    if (grid_size % 2 == 0) return perception;

    int radius       = grid_size / 2;
    int ignore_count = std::min(tilesToIgnore, tile_count > 0 ? tile_count - 1 : 0);
    std::vector<bool> ignore(tile_count, false);
    int ignored = 0;

    auto mark_tile = [&](int rx, int ry) {
        if (ignored >= ignore_count) return;
        int x = rx + radius, y = ry + radius;
        if (x < 0 || y < 0 || x >= grid_size || y >= grid_size) return;
        int idx = x * grid_size + (grid_size - 1 - y);
        if (idx < tile_count && !ignore[idx]) { ignore[idx] = true; ++ignored; }
    };

    for (int r = radius; r >= 1 && ignored < ignore_count; --r) {
        mark_tile(0, -r);
        for (int i = 1; i <= r && ignored < ignore_count; ++i) { mark_tile(-i, -r); mark_tile(i, -r); }
        for (int y = -r+1; y <= r-1 && ignored < ignore_count; ++y) { mark_tile(-r, y); mark_tile(r, y); }
        mark_tile(-r, r); mark_tile(r, r);
        for (int i = r-1; i >= 1 && ignored < ignore_count; --i) { mark_tile(-i, r); mark_tile(i, r); }
        mark_tile(0, r);
    }
    if (ignored < ignore_count) mark_tile(0, 0);

    for (int i = 0; i < tile_count; ++i)
        if (ignore[i]) perception[i] = 0.0;
    return perception;
}

void Simulation::biologySetCoordinates(Vector2d coords)
{
    auto entity = get_primary_entity();
    if (entity) entity->set_coordinates(coords);
    else std::cerr << "No primary entity found!" << std::endl;
}

Vector2d Simulation::biologyGetCoordinates() const
{
    auto entity = get_primary_entity();
    return entity ? entity->get_coordinates() : Vector2d(0, 0);
}

void Simulation::display_environment(int alive) const
{
    if (!_environment) return;

    constexpr const char* kSolid = u8"██";
    constexpr const char* kShade = u8"░░";

    // First render: full clear. Subsequent: cursor-home only (no flicker)
    if (m_firstRender) {
        std::cout << "\033[2J";
        const_cast<Simulation*>(this)->m_firstRender = false;
    }
    std::cout << "\033[H";

    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            int idx = y * GRID_SIZE + x;
            auto it = _entity_pos_map.find(idx);
            Entity* entity_here = (it != _entity_pos_map.end()) ? it->second : nullptr;

            if (entity_here) {
                double health = entity_here->biology_get_metrics()["Health"];
                int g = (int)(health * 255), b = (int)(health * 255);
                std::cout << "\033[38;2;255;" << g << ";" << b << "m" << kSolid << "\033[0m";
            } else if (_prev_entity_positions.count(idx)) {
                // Trail: tile occupied last frame but not this one
                std::cout << "\033[38;2;110;40;40m" << kShade << "\033[0m";
            } else {
                auto res = _resource_manager->findResourcesInRange(Position(x, y), 0);
                if (!res.empty()) {
                    double ev = res[0]->getEnergyValue();
                    int intensity = static_cast<int>(ev * 255);
                    if (res[0]->getType() == ResourceType::FOOD)
                        std::cout << "\033[38;2;" << intensity << ";" << intensity << ";0m" << kSolid << "\033[0m";
                    else
                        std::cout << "\033[38;2;0;0;" << intensity << "m" << kSolid << "\033[0m";
                } else {
                    double tv = _environment->getTileValue(Vector2d(x, y));
                    double norm = (tv + 2.0) / 4.0;
                    int r = (int)(norm * 255), g = (int)((1-norm)*255), b = (int)((1-norm)*255);
                    std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m" << kShade << "\033[0m";
                }
            }
        }
        std::cout << "\033[K\n"; // erase to EOL then newline
    }

    // Stats bar
    int total = (int)_entities.size();
    int aliveCount = (alive >= 0) ? alive : total;
    double totalEnergy = 0.0;
    for (const auto& e : _entities) {
        auto m = e->biology_get_metrics();
        if (m.count("Energy")) totalEnergy += m.at("Energy");
    }
    double avgEnergy = aliveCount > 0 ? totalEnergy / aliveCount : 0.0;

    std::cout << "\033[K";
    std::cout << "  Tick: \033[97m" << m_tick << "\033[0m"
              << "  |  Alive: \033[97m" << aliveCount << "/" << total << "\033[0m"
              << "  |  Avg Energy: \033[97m" << std::fixed
              << std::setprecision(2) << avgEnergy << "\033[0m"
              << "  |  Resources: \033[97m" << _resource_manager->getResourceCount() << "\033[0m"
              << (m_godMode ? "  [\033[93mGOD\033[0m]" : "")
              << "\033[K\n";

    // Legend
    std::cout << "\033[K";
    std::cout << "  "
              << "\033[38;2;255;255;255m" << kSolid << "\033[0m entity  "
              << "\033[38;2;200;200;0m"   << kSolid << "\033[0m food  "
              << "\033[38;2;0;0;200m"     << kSolid << "\033[0m water  "
              << "\033[38;2;100;150;100m" << kShade << "\033[0m terrain  "
              << "\033[38;2;110;40;40m"   << kShade << "\033[0m trail"
              << "\033[K\n";

    std::cout << "\033[K  [P] Pause  [F] Fast-fwd  [Q] Quit\033[K\n";

    std::cout.flush();

    if (m_renderDelayMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(m_renderDelayMs));
}

float Simulation::get_vision_value() const
{
    auto entity = get_primary_entity();
    if (!entity) { std::cerr << "No primary entity!" << std::endl; return 0.0f; }
    return entity->biology_get_genetic_value("Vision");
}

void Simulation::testAccess()
{
    std::cout << "Testing access..." << std::endl;
    float val = environGetTileValue(0, 0);
    std::cout << "Tile at 0,0: " << val << std::endl;
    auto entity = get_primary_entity();
    if (entity) {
        auto metrics = entity->biology_get_metrics();
        for (const auto& pair : metrics) std::cout << pair.first << ": " << pair.second << std::endl;
    } else {
        std::cerr << "No entity found." << std::endl;
    }
}
