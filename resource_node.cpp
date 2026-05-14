/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462 
*/
#include "../include/resource_node.h"
#include <algorithm>
#include <limits>

using namespace std;

// Initialize static ID counter
uint64_t ResourceNode::s_nextID = 1;

ResourceNode::ResourceNode(Position pos, ResourceType type, double energyValue, bool renewable)
    : m_id(s_nextID++)              // Unique ID for tracking
    , m_position(pos)
    , m_type(type)
    , m_currentEnergy(energyValue)
    , m_maxEnergy(energyValue)
    , m_renewable(renewable)
    , m_regenerationRate(renewable ? energyValue * 0.01 : 0.0)  // Renewable resources regen at 1%/tick
    , m_timeSinceLastUpdate(0.0)
{
}

double ResourceNode::consume(double amount) {
    if (amount <= 0.0) return 0.0;
    
    // Can only consume what's available
    double consumed = min(amount, m_currentEnergy);
    m_currentEnergy -= consumed;
    
    return consumed;  // Actual amount taken (may be less than requested)
}

void ResourceNode::update(double deltaTime) {
    // Non-renewable or already full? Nothing to do
    if (!m_renewable || m_currentEnergy >= m_maxEnergy) {
        return;
    }
    
    m_timeSinceLastUpdate += deltaTime;
    
    // Regenerate based on rate and time passed
    double regenerated = m_regenerationRate * deltaTime;
    m_currentEnergy = min(m_currentEnergy + regenerated, m_maxEnergy);  // Cap at max
}

void ResourceNode::regenerate(double amount) {
    if (!m_renewable) return;
    
    m_currentEnergy = min(m_currentEnergy + amount, m_maxEnergy);
}

bool ResourceNode::isInRange(const Position& agentPos, int32_t interactionRange) const {
    return m_position.manhattanDistance(agentPos) <= interactionRange;
}

// ResourceManager implementation

void ResourceManager::addResource(unique_ptr<ResourceNode> resource) {
    m_resources.push_back(std::move(resource));
}

ResourceNode* ResourceManager::createResource(Position pos, ResourceType type, double energyValue, bool renewable) {
    auto resource = make_unique<ResourceNode>(pos, type, energyValue, renewable);
    ResourceNode* ptr = resource.get();
    addResource(std::move(resource));
    return ptr;
}

void ResourceManager::update(double deltaTime) {
    for (auto& resource : m_resources) {
        resource->update(deltaTime);
    }
}

vector<ResourceNode*> ResourceManager::findResourcesInRange(const Position& pos, int32_t range) {
    vector<ResourceNode*> result;
    
    for (auto& resource : m_resources) {
        if (!resource->isDepleted() && resource->isInRange(pos, range)) {
            result.push_back(resource.get());
        }
    }
    
    return result;
}

ResourceNode* ResourceManager::findNearestResource(const Position& pos, int32_t maxRange) {
    ResourceNode* nearest = nullptr;
    double minDistance = numeric_limits<double>::max();
    
    for (auto& resource : m_resources) {
        if (resource->isDepleted()) continue;  // Skip empty resources
        
        double distance = resource->getPosition().euclideanDistance(pos);
        
        // Optional range filter (0 = unlimited)
        if (maxRange > 0 && distance > maxRange) continue;
        
        // Track closest one
        if (distance < minDistance) {
            minDistance = distance;
            nearest = resource.get();
        }
    }
    
    return nearest;  // nullptr if nothing found
}

size_t ResourceManager::removeDepletedResources() {
    size_t initialCount = m_resources.size();
    
    // Erase-remove idiom: removes depleted non-renewables
    // Renewables stay even if empty (they can regenerate)
    m_resources.erase(
        remove_if(m_resources.begin(), m_resources.end(),
            [](const unique_ptr<ResourceNode>& r) {
                return r->isDepleted() && !r->isRenewable();
            }),
        m_resources.end()
    );
    
    return initialCount - m_resources.size();  // How many we removed
}

double ResourceManager::getTotalEnergy() const {
    double total = 0.0;
    for (const auto& resource : m_resources) {
        total += resource->getEnergyValue();
    }
    return total;
}

void ResourceManager::clear() {
    m_resources.clear();
}
