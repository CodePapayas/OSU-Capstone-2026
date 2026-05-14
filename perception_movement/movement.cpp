#include "movement.hpp"
#include "../source/Environment.h"
#include <algorithm>
#include <cmath>

Movement::Action Movement::decide_movement(const std::vector<double>& logits) {
    if (logits.empty()) {
        // No decision available - default to staying
        return direction_to_action(STAY, 0.0);
    }

    // Find the index with the maximum value (argmax)
    auto max_it = std::max_element(logits.begin(), logits.end());
    int max_index = std::distance(logits.begin(), max_it);

    // Map index to direction (assumes logits correspond to Direction enum)
    Direction chosen_direction = STAY;
    if (max_index >= 0 && max_index <= 8) {
        chosen_direction = static_cast<Direction>(max_index);
    }

    return direction_to_action(chosen_direction);
}

Movement::Action Movement::direction_to_action(Direction dir, double base_energy_cost) {
    Action action;
    action.direction = dir;
    action.energy_cost = base_energy_cost;

    switch (dir) {
        case NORTH:
            action.dx = 0;
            action.dy = -1;
            break;
        case SOUTH:
            action.dx = 0;
            action.dy = 1;
            break;
        case WEST:
            action.dx = -1;
            action.dy = 0;
            break;
        case EAST:
            action.dx = 1;
            action.dy = 0;
            break;
        case NORTHEAST:
            action.dx = 1;
            action.dy = -1;
            action.energy_cost = base_energy_cost * 1.414; // Diagonal costs more (sqrt(2))
            break;
        case NORTHWEST:
            action.dx = -1;
            action.dy = -1;
            action.energy_cost = base_energy_cost * 1.414;
            break;
        case SOUTHEAST:
            action.dx = 1;
            action.dy = 1;
            action.energy_cost = base_energy_cost * 1.414;
            break;
        case SOUTHWEST:
            action.dx = -1;
            action.dy = 1;
            action.energy_cost = base_energy_cost * 1.414;
            break;
        case STAY:
        default:
            action.dx = 0;
            action.dy = 0;
            action.energy_cost = 0.0; // No energy cost for staying
            break;
    }

    return action;
}

bool Movement::execute_movement(
    int& current_x,
    int& current_y,
    const Action& action,
    Environment& environment,
    double& energy) {

    // Calculate new position
    int new_x = current_x + action.dx;
    int new_y = current_y + action.dy;

    // Check if new position is valid
    if (!is_valid_position(new_x, new_y, environment)) {
        // Invalid move - stay in place but still consume energy
        energy -= action.energy_cost * 0.5; // Half energy cost for failed move
        return false;
    }

    // Check if agent has enough energy
    if (energy < action.energy_cost) {
        // Not enough energy - cannot move
        return false;
    }

    // Execute the movement
    current_x = new_x;
    current_y = new_y;
    energy -= action.energy_cost;

    return true;
}

std::vector<int> Movement::execute_movement_wraparound(
    int& current_x,
    int& current_y,
    const Action& action,
    int env_width,
    int env_height,
    double& energy) {
    
    std::vector<int> prev_coords = {current_x, current_y};
    // Calculate new position
    int new_x = (current_x + action.dx) % env_width;
    int new_y = (current_y + action.dy) % env_height;

    // Handle negative wraparound
    if (new_x < 0) new_x += env_width;
    if (new_y < 0) new_y += env_height;

    // Check if new position is within bounds
    if (new_x < 0 || new_x >= env_width || new_y < 0 || new_y >= env_height) {
        // Invalid move - stay in place but still consume energy
        energy -= action.energy_cost * 0.5; // Half energy cost for failed move
        return prev_coords;
    }

    // Check if agent has enough energy
    if (energy < action.energy_cost) {
        // Not enough energy - cannot move
        return prev_coords;
    }

    // Execute the movement
    current_x = new_x;
    current_y = new_y;
    energy -= action.energy_cost;
    std::vector<int> new_coords = {current_x, current_y};
    return new_coords;
}

bool Movement::is_valid_position(int x, int y, Environment& environment) {
    // Calculate environment size
    int env_size = environment.getChunksInEnvironment() * environment.getTilesPerChunk();

    // Check bounds
    return (x >= 0 && x < env_size && y >= 0 && y < env_size);
}
