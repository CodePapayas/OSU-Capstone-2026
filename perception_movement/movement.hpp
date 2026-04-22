#pragma once

#include <vector>

// Forward declaration
class Environment;

/**
 * Movement module - converts decisions to actions and executes movement
 * Handles movement within environment boundaries
 */

class Movement {
public:
    enum Direction {
        NORTH = 0,
        SOUTH = 1,
        WEST = 2,
        EAST = 3,
        NORTHEAST = 4,
        NORTHWEST = 5,
        SOUTHEAST = 6,
        SOUTHWEST = 7,
        STAY = 8
    };

    struct Action {
        Direction direction;
        int dx;                 // Change in X position
        int dy;                 // Change in Y position
        double energy_cost;     // Energy cost of this action
    };

    /**
     * Convert brain output (decision logits) to a movement action
     * Uses argmax to select the action with highest score
     * @param logits - Output from brain/decision system
     * @return Action representing the chosen movement
     */
    static Action decide_movement(const std::vector<double>& logits);

    /**
     * Execute movement with environment boundary checking
     * @param current_x - Agent's current X position (will be updated)
     * @param current_y - Agent's current Y position (will be updated)
     * @param action - The action to execute
     * @param environment - Reference to Environment for boundary checking
     * @param energy - Agent's current energy (will be updated)
     * @return true if movement was successful, false if blocked
     */
    static bool execute_movement(
        int& current_x,
        int& current_y,
        const Action& action,
        Environment& environment,
        double& energy
    );

    /**
     * Execute movement with wraparound (toroidal) environment
     * @param current_x - Agent's current X position (will be updated)
     * @param current_y - Agent's current Y position (will be updated)
     * @param action - The action to execute
     * @param env_width - Environment width
     * @param env_height - Environment height
     * @param energy - Agent's current energy (will be updated)
     * @return true if movement was successful, false if blocked
     */
    static std::vector<int> execute_movement_wraparound(int &current_x, int &current_y, const Action &action, int env_width, int env_height, double &energy);

    /**
     * Legacy version with explicit width/height parameters
     * @param current_x - Agent's current X position (will be updated)
     * @param current_y - Agent's current Y position (will be updated)
     * @param action - The action to execute
     * @param env_width - Environment width
     * @param env_height - Environment height
     * @param energy - Agent's current energy (will be updated)
     * @return true if movement was successful, false if blocked
     */
    static bool execute_movement(
        int& current_x,
        int& current_y,
        const Action& action,
        int env_width,
        int env_height,
        double& energy
    );

    /**
     * Convert a direction enum to an Action struct
     * @param dir - Direction to convert
     * @param base_energy_cost - Base energy cost for movement (default 0.1)
     * @return Action representing the direction
     */
    static Action direction_to_action(Direction dir, double base_energy_cost = 0.1);

private:
    /**
     * Check if a position is within environment bounds
     * @param x - X position to check
     * @param y - Y position to check
     * @param environment - Reference to Environment
     * @return true if position is valid, false otherwise
     */
    static bool is_valid_position(int x, int y, Environment& environment);
};
