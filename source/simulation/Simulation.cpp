#include "Simulation.hpp"
#include "../environment/Environment.h"
#include "../environment/PerlinNoise.hpp"
#include "../entity/decision_center/entity.hpp"
#include "../entity/decision_center/brain.hpp"
#include "../entity/decision_center/biology.hpp"
#include "../entity/perception_movement/perception.hpp"
#include "../entity/perception_movement/movement.hpp"
#include "../environment/resource_node.h"
#include "../entity/decision_center/mutate.hpp"
#include "../entity/decision_center/biology_constants.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <format>

Simulation::Simulation()
    : _environment(nullptr)
{
}

Simulation::~Simulation() = default;

void Simulation::initialize(int num_entities)
{
    num_entities = std::max(1, std::min(num_entities, 50));

    int size = 32;
    _environment = std::make_unique<Environment>(size, size);
    std::cout << "Environment created successfully!" << std::endl;

    std::vector<int> layer_sizes = {228, 300, 300, 7};
    for (int i = 0; i < num_entities; ++i) {
        auto entity = std::make_unique<Entity>();
        entity->set_coordinates(Vector2d(rand() % _environment->getTileAmountX(),
                                         rand() % _environment->getTileAmountY()));
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
    // Example of seeding some resources in the environment
    _resource_manager->clear(); // Clear existing resources before seeding new ones
    for (int x =0; x < _environment->getTileAmountX(); x += 1) {
        for (int y = 0; y < _environment->getTileAmountY(); y += 1) {
            float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            if (randomValue > 0.9){ // 10% chance to create a resource
                // 20% chance to create 1 of 4 chemical nodes
                int rType = rand()%5;
                if (rType ==4) {
                    rType= rand()%4+2;
                }
                // Otherwise a coinflip between water and energy
                else{
                    rType = rand() % 2;
                }
                ResourceType type = static_cast<ResourceType>(rType); // Randomly choose a resource type
                double energyValue = static_cast<double>(rand()) / static_cast<double>(RAND_MAX); // Random energy value between 0 and 1
                bool renewable = (rand() % 2) == 0; // Randomly decide if it's renewable
                _resource_manager->createResource(Position(x, y), type, energyValue, renewable);
                /*
                std::cout << "Seeded resource at (" << x << ", " << y << ") with energy " << energyValue 
                          << " and type " << (type == ResourceType::FOOD ? "FOOD" : "WATER") 
                          << (renewable ? " (Renewable)" : " (Non-renewable)") << std::endl;
                */
            }
        }
    }
}   

float Simulation::environGetTileValue(int x, int y) const
{
    return _environment->getTileValue(Vector2d(x, y), 0);
}


Entity* Simulation::reproduce(Entity* p1, Entity* p2)
{
    // Redirect cout to null to suppress output during reproduction
    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(nullptr);
    Entity* brainParent = (rand() % 2 == 0) ? p1 : p2; // pick one parent for brain
    std::unordered_map<std::string, double> p1_genetics = p1->biology_get_genetics();
    std::unordered_map<std::string, double> p2_genetics = p2->biology_get_genetics();
    std::unordered_map<std::string, double> child_genetics;
    // make a choice between each parent for each value in the genetics and then mutate it before passing to the child
    for (const auto& pair : p1_genetics)
    {
        const std::string& gene = pair.first;
        double val1 = pair.second;
        double val2 = p2_genetics[gene];
        double chosen_val = (rand() % 2 == 0) ? val1 : val2; // Randomly choose one parent's value
        child_genetics[gene] = chosen_val;
    }
    // Mutate the child's genetics
    child_genetics = mutate_genetics(child_genetics);
    // Create a new entity with the child's genetics
    Entity* child = new Entity();
    child->set_biology(std::make_shared<Biology>(false)); // false for random genetics, will be overwritten by set_genetic_vals
    child->get_biology()->set_genetic_vals(child_genetics);
    // Copy the parents brain, mutate hthe weights and biases, and set it to the child,
    child->set_brain(std::make_shared<Brain>(*brainParent->get_brain()));   
    std::vector<ActivationLayerReLU>& parent_layers = brainParent->get_brain()->get_layers();
    int layer_index = 0;
    for (auto& layer : parent_layers) {
        std::vector<double> mutated_weights = mutate_vector(layer.get_weights());
        std::vector<double> mutated_biases = mutate_vector(layer.get_biases());
        child->get_brain()->get_layers()[layer_index].ActivationLayerReLUOffsping(mutated_weights, mutated_biases);
        layer_index++;
    }
    // child-get_brain()->set_layers(mutated_layers);
    _entities.push_back(std::unique_ptr<Entity>(child));
    // Restore the original cout buffer
    std::cout.rdbuf(originalCoutBuffer);
    return child;
}

void Simulation::set_primary_entity(const Entity& entity){
    _entities.clear();
    
    auto cloned = std::make_unique<Entity>();
    //cloned->set_coordinates(Vector2d(0,0)); // Set initial coordinates for the entity
    cloned->set_coordinates(Vector2d(rand() % _environment->getTileAmountX(), rand() % _environment->getTileAmountY())); // Set random initial coordinates for the entity
    if (entity.get_brain()) {
        cloned->set_brain(std::make_unique<Brain>(*entity.get_brain()));
    }
    if (entity.get_biology()) {
        cloned->set_biology(std::make_unique<Biology>(*entity.get_biology()));
    }
    // Make sure the clone doesn't copy the metrics of the original.
    cloned->get_biology()->add_energy(1.);
    cloned->get_biology()->add_health(1.);
    cloned->get_biology()->add_water(1.);
    _entities.push_back(std::move(cloned));
}
void Simulation::set_primary_entity_random(){
    _entities.clear(); // Clear existing entities
    auto entity = std::make_unique<Entity>();
    std::cout << "Entity created successfully with ID: " << entity->get_id() << std::endl;
    //entity->set_coordinates(Vector2d(0, 0)); // Set initial coordinates for the entity
    entity->set_coordinates(Vector2d(rand() % _environment->getTileAmountX(), rand() % _environment->getTileAmountY())); // Set random initial coordinates for the entity
    // Create a brain with a neural network architecture
    // Architecture: 28 inputs -> 8 hidden -> 8 hidden -> 6 outputs () (128 inputs for 5x5 perception of 3 environtypes and food and water + 3 internal state metrics)
    std::vector<int> layer_sizes = {128, 200, 200, 7}; // 28 because perception size is 5x5 and then the entity's internal state (3 values for now)
    auto brain = std::make_shared<Brain>(layer_sizes);
    std::cout << "Brain created successfully with " << brain->get_layer_count() << " layers!" << std::endl;

    // Create biology
    auto biology = std::make_shared<Biology>(false);  // false for randomized genetics
    std::cout << "Biology created successfully!" << std::endl;

    // Set the brain and biology on the entity
    entity->set_brain(brain);
    entity->set_biology(biology);
    std::cout << "Entity configured with brain and biology!" << std::endl;

    // Add entity to the simulation
    _entities.push_back(std::move(entity));
}
Entity* Simulation::get_primary_entity() const
{
    if (_entities.empty() || _current_entity_index >= (int)_entities.size())
    {
        return nullptr;
    }
    return _entities[_current_entity_index].get();
}

std::vector<double> Simulation::get_perception() const
{
    Perception::SensoryInput val = _perception->perceive_local_tiles(
        get_primary_entity()->get_coordinates().x,
        get_primary_entity()->get_coordinates().y,
        *_environment,
        std::max(2, static_cast<int>(4 * get_primary_entity()->biology_get_genetic_value("Vision")))
    );
    return val.tile_values;
}

std::vector<double> Simulation::get_perception_expanded(const std::string& type) const
{
    std::vector<double> expanded_perception;
    std::vector<double> type_values = _perception->extract_tile_values_in_radius_of_type(
        get_primary_entity()->get_coordinates().x,
        get_primary_entity()->get_coordinates().y,
        *_environment,
            2,
        *_resource_manager,
        type
    );
return type_values;
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
    //std::vector<double> perception = get_perception();
    std::vector<double> filteredPerception;
    enum Types {FOOD=0, WATER=1, TERRAIN_1=2, TERRAIN_2=3, TERRAIN_3=4};
    // Iterate over each tile type and concatenate the perception values for each type together. This is a bit hacky but it works for now until we have a better system for encoding tile types and perception.
    for (int type = FOOD; type <= TERRAIN_3; type++) {
        std::string type_str;
        switch (type) {
            case FOOD:
                type_str = "FOOD";
                break;
            case WATER:
                type_str = "WATER";
                break;
            case TERRAIN_1:
                type_str = "TERRAIN_1";
                break;
            case TERRAIN_2:
                type_str = "TERRAIN_2";
                break;
            case TERRAIN_3:
                type_str = "TERRAIN_3";
                break;
        }

        // FILTERS OUT TOO MANY TILES, is causing a size mismatch in the dot product function in the brain module, causing vector subscript out-of-bounds issue
        std::vector<double> perception = get_perception_expanded(type_str);
        // Get the strength of the entities vision and determine how many tiles to ignore
        float vision_value = entity->biology_get_genetic_value("Vision");
        int tilesToIgnore = std::max(static_cast<int>(25.0 - (25 * vision_value)), 1); // at max vision (1.0), ignore 0 tiles, at min vision (0.0) ignore 24 tiles (only sees own tile) 
    
        // Add the filtered values to the master perception list
        std::vector<double> adaptedVision = filter_perception(perception, tilesToIgnore);
        filteredPerception.insert(filteredPerception.end(), adaptedVision.begin(), adaptedVision.end());  
    }
    filteredPerception.push_back(entity->biology_get_metrics()["Energy"]);
    filteredPerception.push_back(entity->biology_get_metrics()["Health"]);
    filteredPerception.push_back(entity->biology_get_metrics()["Water"]);
    // Get the decision from the brain
    if (_debug){
        std::cout << "Filtered Perception Length: " << filteredPerception.size() << " with "<< entity->biology_get_genetic_value("Vision")<<std::endl;
    }
    int decision = entity->brain_get_decision(filteredPerception);
    return decision;
}

void Simulation::interpret_decision(int decision_code)
{
    auto entity = get_primary_entity();
    std::cout << "Entity's current position" << " (" << entity->x << ", " << entity->y << ")" << std::endl;
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
            Simulation::consumption();
            break;
        case DecisionCodes::REPRODUCE:
        {
            Entity* parent1 = get_primary_entity();
            int vision_radius = std::max(2, static_cast<int>(4 * parent1->biology_get_genetic_value("Vision")));
            Entity* parent2 = nullptr;
            for (int j = 0; j < (int)_entities.size(); ++j) {
                if (j == _current_entity_index || _entities[j]->biology_check_death()) continue;
                int dx = std::abs(_entities[j]->get_coordinates().x - parent1->get_coordinates().x);
                int dy = std::abs(_entities[j]->get_coordinates().y - parent1->get_coordinates().y);
                if (std::max(dx, dy) <= vision_radius) {
                    parent2 = _entities[j].get();
                    break;
                }
            }
            if (parent2) {
                reproduce(parent1, parent2);
                std::cout << "Entity reproduced." << std::endl;
            }
            break;
        }
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
    std::vector<int> new_coords = Movement::execute_movement_wraparound(entity->x, entity->y, action, _environment->getTileAmountX(), _environment->getTileAmountY(), entity->biology_get_metrics()["Energy"]);
    if(new_coords[0]>=_environment->getTileAmountX() || new_coords[1] >= _environment->getTileAmountY() || new_coords[0] < 0 || new_coords[1] < 0){
        std::cerr << "Error: Movement resulted in out of bounds coordinates (" << new_coords[0] << ", " << new_coords[1] << ")" << std::endl;
        return;
    }
    entity->set_coordinates(Vector2d(new_coords[0], new_coords[1]));
    std::string terrain_type = _environment->getTileType(Vector2d(entity->x, entity->y));
    std::cout << "Entity moved from (" << prev_x << ", " << prev_y << ") to (" << entity->x << ", " << entity->y << ") on type " << terrain_type << std::endl;
    // Need to drain energy based on the terrain type of the new tile and the entity's biology
    //entity->biology_movement(_environment->getTileType((entity->x, entity->y))); // Something like this in practice
    entity->biology_movement(terrain_type); // Placeholder until we have actual terrain types implemented
    
    //check if there's a resource on the new tile and consume it if there is
    ResourceNode* resource = _resource_manager->getResourceAtPosition(Position(entity->x, entity->y));
    if (resource) {
        double energyGained = resource->consume(entity->biology_get_genetic_value("Mass")); // Consume energy based on Mass ?
        if (resource->getType() == ResourceType::FOOD) {
            std::cout << "Entity consumed FOOD resource for" << energyGained << " raw energy." << std::endl;
            entity->biology_eat(energyGained); // Add the consumed energy to the entity's biology
        } else if (resource->getType() == ResourceType::WATER) {
            std::cout << "Entity consumed WATER resource for " << energyGained << " raw water." << std::endl;
            entity->biology_drink(energyGained); // Add the consumed energy to the entity's biology
        }
    }

}   

void Simulation::consumption(){
    Entity* entity = get_primary_entity();
    ResourceNode* resource = _resource_manager->getResourceAtPosition(Position(entity->x, entity->y));
    if (resource) {
        double energyGained = resource->consume(entity->biology_get_genetic_value("Mass")); // Consume energy based on Mass ?
        if (resource->getType() == ResourceType::FOOD) {
            std::cout << "Entity consumed FOOD resource for" << energyGained << " raw energy." << std::endl;
            entity->biology_eat(energyGained); // Add the consumed energy to the entity's biology
        } else if (resource->getType() == ResourceType::WATER) {
            std::cout << "Entity consumed WATER resource for " << energyGained << " raw water." << std::endl;
            entity->biology_drink(energyGained); // Add the consumed energy to the entity's biology
        } 
        else if (resource->getType() == ResourceType::CHEMICAL_1) {
            std::cout << "Entity consumed CHEMICAL 1 resource for " << energyGained << " raw chemical energy." << std::endl;
            entity->biology_consume_chemical("CHEMICAL_1", energyGained); // Add the consumed chemical to the entity's biology, using the tile type as a proxy for chemical type for now
        }
        else if (resource->getType() == ResourceType::CHEMICAL_2) {
            std::cout << "Entity consumed CHEMICAL 2 resource for " << energyGained << " raw chemical energy." << std::endl;
            entity->biology_consume_chemical("CHEMICAL_2", energyGained); // Add the consumed chemical to the entity's biology, using the tile type as a proxy for chemical type for now
        }
        else if (resource->getType() == ResourceType::CHEMICAL_3) {
            std::cout << "Entity consumed CHEMICAL 3 resource for " << energyGained << " raw chemical energy." << std::endl;
            entity->biology_consume_chemical("CHEMICAL_3", energyGained); // Add the consumed chemical to the entity's biology, using the tile type as a proxy for chemical type for now
        }
        else if (resource->getType() == ResourceType::CHEMICAL_4) {
            std::cout << "Entity consumed CHEMICAL 4 resource for " << energyGained << " raw chemical energy." << std::endl;
            entity->biology_consume_chemical("CHEMICAL_4", energyGained); // Add the consumed chemical to the entity's biology, using the tile type as a proxy for chemical type for now
        }
    }
    else{
        std::cout << "Entity could not consume anything on this tile..." << endl; 
    }
}

int Simulation::tick(int print){
    _debug = print;
    if (!print){
        std::cout.setstate(std::ios_base::failbit);
    }

    _environment->updateTiles();

    for (int i = 0; i < (int)_entities.size(); ++i) {
        _current_entity_index = i;
        if (_entities[i]->biology_check_death()) continue;

        int decision = pass_perception_to_brain();
        interpret_decision(decision);
        _entities[i]->update_biology();
        _entities[i]->biology_get_metrics(true);
    }

    _current_entity_index = 0;

    cout << _environment->getTileAmountX() << "x" << _environment->getTileAmountY() << endl;
    if (print){
        display_environment();
    }

    int alive = 0;
    for (const auto& e : _entities) {
        if (!e->biology_check_death()) ++alive;
    }

    if (!print){
        std::cout.clear();
    }

    if (alive == 0) {
        std::cout << "All entities have died. Ending simulation." << std::endl;
        return -1;
    }
    std::cout << alive << "/" << _entities.size() << " entities alive." << std::endl;
    return 0;
}

size_t Simulation::get_entity_count() const
{
    return _entities.size();
}

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

    constexpr const char* kSolidBlock = u8"\u2588\u2588";
    constexpr const char* kLightShade = u8"\u2591\u2591";

    std::cout << "\n=== Environment Display ===\n" << std::endl;
    for (int y = 0; y < _environment->getTileAmountY(); ++y)
    {
        for (int x = 0; x < _environment->getTileAmountX(); ++x)
        {
            double tile_value = _environment->getTileValue(Vector2d(x, y), 0);

            Entity* entity_here = nullptr;
            for (const auto& e : _entities) {
                if (!e->biology_check_death() &&
                    e->get_coordinates().x == x && e->get_coordinates().y == y) {
                    entity_here = e.get();
                    break;
                }
            }

            if (entity_here)
            {
                double curr_health = entity_here->biology_get_metrics()["Health"];
                int r, g, b;
                r = 255;
                g = (int)(curr_health * 255);
                b = (int)(curr_health * 255);
                std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m"
                          << kSolidBlock
                          << "\033[0m";
            }
            else if(!_resource_manager->findResourcesInRange(Position(x, y), 0).empty()) // Check if there's a resource at this location
            {
                // Display resource as Blue blocks if water, yellow for energy, with intensity based on the energy value of the resource
                double energy_value = _resource_manager->getResourceAtPosition(Position(x, y))->getEnergyValue();
                int intensity = static_cast<int>(energy_value * 255);
                if (_resource_manager->getResourceAtPosition(Position(x, y))->getType() == ResourceType::FOOD) {
                    // Yellow color for food
                    std::cout << "\033[38;2;" << intensity << ";" << intensity << ";0m"  // Yellow color with intensity
                              << kSolidBlock
                              << "\033[0m"; // Reset color
                }
                else {
                    // Blue color for water 
                    std::cout << "\033[38;2;0;0;" << intensity << "m"  // Blue color with intensity
                          << kSolidBlock
                          << "\033[0m"; // Reset color

                }
            }
            else
            {
                // Some hacky color coding adopted from another project.
                int r, g, b;

                double normalized = (tile_value + 2.0) / 4.0; // 0.0 to 1.0
                r = (int)(normalized * 255);
                g = (int)((1 - normalized) * 255);
                b = (int)((1 - normalized) * 255);

                //normalized = tile_value;

                bool aesthetic = true;
                if(aesthetic){
                    std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m"
                            << kLightShade
                            << "\033[0m";
                } else {
                    // Alternative print method, prints the value instead of 0
                    char buffer[20];
                    std::sprintf(buffer,"%.1f", tile_value); 
                    std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m"
                            << buffer
                            << " "
                            << "\033[0m"; 
                    
               }
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