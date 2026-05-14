/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462 
*/
#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <cmath>

using namespace std;

struct Position {
    int32_t x, y;
    Position() : x(0), y(0) {}
    Position(int32_t x, int32_t y) : x(x), y(y) {}
    
    int32_t manhattanDistance(const Position& other) const {
        return abs(x - other.x) + abs(y - other.y);
    }
    double euclideanDistance(const Position& other) const {
        int32_t dx = x - other.x, dy = y - other.y;
        return sqrt(double(dx * dx + dy * dy));
    }
    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
};

enum class ResourceType { FOOD, WATER, MINERAL, PLANT, CUSTOM };

/**
 * ResourceNode - Consumable energy sources in environment (renewable or depleting)
 */
class ResourceNode {
public:
    ResourceNode(Position pos, ResourceType type, double energyValue, bool renewable = false);
    
    Position getPosition() const { return m_position; }
    ResourceType getType() const { return m_type; }
    double getEnergyValue() const { return m_currentEnergy; }
    double getMaxEnergy() const { return m_maxEnergy; }
    bool isDepleted() const { return m_currentEnergy <= 0.0; }
    bool isRenewable() const { return m_renewable; }
    uint64_t getID() const { return m_id; }
    
    double consume(double amount);  // Extract energy, returns actual consumed
    void update(double deltaTime);  // Regenerate if renewable
    void regenerate(double amount);
    bool isInRange(const Position& agentPos, int32_t interactionRange = 1) const;

private:
    static uint64_t s_nextID;       // For generating unique IDs
    
    uint64_t m_id;                  // Unique identifier
    Position m_position;            // Grid position
    ResourceType m_type;            // Resource category
    double m_currentEnergy;         // Current available energy
    double m_maxEnergy;             // Maximum energy capacity
    bool m_renewable;               // Can regenerate?
    double m_regenerationRate;      // Energy restored per tick (if renewable)
    double m_timeSinceLastUpdate;   // For regeneration timing
};

/**
 * ResourceManager - Spatial management and queries for all resources
 */
class ResourceManager {
public:
    ResourceManager() = default;
    
    void addResource(unique_ptr<ResourceNode> resource);
    ResourceNode* createResource(Position pos, ResourceType type, double energyValue, bool renewable = false);
    void update(double deltaTime);
    
    vector<ResourceNode*> findResourcesInRange(const Position& pos, int32_t range);
    ResourceNode* findNearestResource(const Position& pos, int32_t maxRange = 0);
    size_t removeDepletedResources();
    
    size_t getResourceCount() const { return m_resources.size(); }
    double getTotalEnergy() const;
    void clear();

private:
    vector<unique_ptr<ResourceNode>> m_resources;
};
