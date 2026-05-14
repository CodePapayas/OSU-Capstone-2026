#include "perception.hpp"
#include "../source/Environment.h"

Perception::SensoryInput Perception::perceive_local_tiles(
    int entity_x,
    int entity_y,
    Environment& environment,
    int radius) {

    SensoryInput sensory;
    sensory.entity_x = entity_x;
    sensory.entity_y = entity_y;
    sensory.perception_radius = radius;
    sensory.grid_size = 2 * radius + 1;

    // Extract tile values in a square grid around the agent
    sensory.tile_values = extract_tile_values_in_radius(
        entity_x, entity_y, environment, radius
    );

    return sensory;
}

std::vector<double> Perception::extract_tile_values_in_radius(
    int center_x,
    int center_y,
    Environment& environment,
    int radius) {
    
    std::vector<double> tile_values;
    
    // Calculate the total environment size
    int env_size = environment.getChunksInEnvironment() * environment.getTilesPerChunk();
    
    // Scan a square grid centered on the agent's position
    // Goes from (center_x - radius) to (center_x + radius)
    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            int tile_x = (center_x + dx + env_size) % env_size; // Wrap around horizontally
            int tile_y = (center_y + dy + env_size) % env_size; // Wrap around vertically
            
            // Check if the tile is within environment bounds
            if (tile_x >= 0 && tile_x < env_size && 
                tile_y >= 0 && tile_y < env_size) {
                
                // Get the tile value from the environment
                // Using the Vector2d class from Environment.h
                Vector2d position(tile_x, tile_y);
                double tile_value = environment.getTileValue(position);
                tile_values.push_back(tile_value);
            } else {
                // Outside bounds - use a default value (e.g., 0.0)
                tile_values.push_back(0.0);
            }
        }
    }
    
    return tile_values;
}

// Legacy function for backward compatibility
Perception::SensoryInput Perception::perceive_local_environment(
    int entity_x,
    int entity_y,
    const std::vector<std::vector<double>>& difficulty_map) {

    SensoryInput sensory;
    sensory.entity_x = entity_x;
    sensory.entity_y = entity_y;

    // Simple placeholder implementation for backward compatibility
    // Convert difficulty map format to tile values
    std::vector<double> features;
    
    if (!difficulty_map.empty() && entity_y < difficulty_map.size() && 
        entity_x < difficulty_map[entity_y].size()) {
        features.push_back(static_cast<double>(difficulty_map[entity_y][entity_x]));
    } else {
        features.push_back(0.5);
    }
    
    sensory.tile_values = features;
    sensory.perception_radius = 0;
    sensory.grid_size = 1;

    return sensory;
}
