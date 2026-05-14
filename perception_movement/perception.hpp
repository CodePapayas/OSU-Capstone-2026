#pragma once

#include <vector>

class Environment;

/**
 * Perception module - handles sensory input from environment
 * Agent perceives tiles in a radius around its position and extracts float values
 */

class Perception {
public:
    /**
     * Structure to hold sensory information from the environment
     * Contains tile values in a radius around the agent and the agent's position
     */
    struct SensoryInput {
        std::vector<double> tile_values;  // Float values from tiles in perception radius
        int entity_x;                    // Agent's current X position
        int entity_y;                    // Agent's current Y position
        int perception_radius;           // Radius used for perception
        int grid_size;                   // Size of perception grid (2*radius + 1)
    };

    /**
     * Perceive tiles in a radius around the agent
     * @param entity_x - Agent's current X position
     * @param entity_y - Agent's current Y position
     * @param environment - Reference to the Environment object
     * @param radius - Perception radius (default 2)
     * @return SensoryInput containing tile values in the perception radius
     */
    static SensoryInput perceive_local_tiles(
        int entity_x,
        int entity_y,
        Environment& environment,
        int radius = 2
    );

    /**
     * Legacy function for backward compatibility with difficulty map interface
     * @deprecated Use perceive_local_tiles with Environment instead
     */
    static SensoryInput perceive_local_environment(
        int entity_x,
        int entity_y,
        const std::vector<std::vector<double>>& difficulty_map
    );

private:
    /**
     * Helper function to extract tile values in a square radius
     * @param center_x - Center X position
     * @param center_y - Center Y position
     * @param environment - Reference to Environment
     * @param radius - Perception radius
     * @return Vector of tile values in the perception area
     */
    static std::vector<double> extract_tile_values_in_radius(
        int center_x,
        int center_y,
        Environment& environment,
        int radius
    );
};
