#include "entity.hpp"
#include <iostream>
#include <iomanip>
#include "biology.hpp"
#include "brain.hpp"
#include "../source/Environment.h"

// Static ID counter for entities
static long long entity_id_counter = 0;

Entity::Entity()
    : _biology(nullptr), _brain(nullptr), _location(nullptr), _id(entity_id_counter++)
{
}

// ==================== Setters ====================

void Entity::set_biology(const std::shared_ptr<Biology>& biology)
{
    _biology = biology;
}

void Entity::set_brain(const std::shared_ptr<Brain>& brain)
{
    _brain = brain;
}

void Entity::set_location(const std::any& location)
{
    _location = location;
}


// ==================== Getters ====================

long long Entity::get_id() const
{
    return _id;
}

std::shared_ptr<Biology> Entity::get_biology() const
{
    return _biology;
}

std::shared_ptr<Brain> Entity::get_brain() const
{
    return _brain;
}

// Why the hell did I make this a 2d vector?
void Entity::set_coordinates(const Vector2d& coords)
{
    x = static_cast<int>(coords.x);
    y = static_cast<int>(coords.y);
}

Vector2d Entity::get_coordinates() const
{
    return Vector2d(x, y); 
}

std::any Entity::get_location() const
{
    //Deprecated at this point
    return _location;
}

// ==================== Brain Related Methods ====================

int Entity::brain_get_decision(const std::vector<double>& inputs)
{
    if (_brain == nullptr)
    {
        std::cerr << "Warning: Where dat brain at?" << std::endl;
        return -1;
    }
    // get a decision
    return _brain->decide(inputs);
}

// ==================== Biology Related Methods ====================

// All below Presently untested in the simulation
void Entity::biology_add_chemical(const std::string& chem, double amount)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot add chemical" << std::endl;
        return;
    }
    _biology->add_chemical(chem, amount);
}

void Entity::biology_rem_chemical(const std::string& chem, double amount)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot remove chemical" << std::endl;
        return;
    }
    _biology->add_chemical(chem, amount * -1);
}

void Entity::biology_eat(double amount)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot eat" << std::endl;
        return;
    }
    double net_energy = _biology->eat_energy(amount);
    std::cout << "Creature consumed " << amount << " energy for net " 
              << net_energy << " energy" << std::endl;
}

void Entity::biology_add_energy(double amount)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot add energy" << std::endl;
        return;
    }
    _biology->add_energy(amount);
}

void Entity::biology_rem_energy(double amount)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot remove energy" << std::endl;
        return;
    }
    _biology->add_energy(amount * -1);
}

void Entity::biology_drink(double amount)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot drink" << std::endl;
        return;
    }
    double net_water = _biology->drink_water(amount);
    std::cout << "Creature consumed " << amount << " water for net " 
              << net_water << " water" << std::endl;
}

void Entity::biology_add_water(double amount)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot add water" << std::endl;
        return;
    }
    _biology->add_water(amount);
}

void Entity::biology_rem_water(double amount)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot remove water" << std::endl;
        return;
    }
    _biology->add_water(amount * -1);
}

void Entity::biology_add_health(double amount)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot add health" << std::endl;
        return;
    }
    _biology->add_health(amount);
}

void Entity::biology_rem_health(double amount)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot remove health" << std::endl;
        return;
    }
    _biology->add_health(amount * -1);
}

bool Entity::biology_check_death() const
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot check death" << std::endl;
        return false;
    }
    return _biology->check_death();
}

void Entity::update_biology()
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology is nullptr, cannot update" << std::endl;
        return;
    }
    _biology->update();
}

// Just a easy way to get all values I guess
std::unordered_map<std::string, double> Entity::biology_get_metrics()
{
    std::unordered_map<std::string, double> metrics;
    
    if (_biology == nullptr)
    {
        std::cerr << "Warning: It has a face but a no body!" << std::endl;
        return metrics;
    }

    double health = _biology->get_health();
    double energy = _biology->get_energy();
    double water = _biology->get_water();
    
    _biology->print_vals();
    
    metrics["Health"] = health;
    metrics["Energy"] = energy;
    metrics["Water"] = water;
    
    return metrics;
}

std::unordered_map<std::string, double> Entity::biology_get_genetics()
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: no biology detected" << std::endl;
        return std::unordered_map<std::string, double>();
    }

    _biology->display_genetic_vals();
    return _biology->get_genetic_vals();
}

double Entity::biology_get_genetic_value(const std::string& gene)
{
    if (_biology == nullptr)
    {
        std::cerr << "Warning: Biology may not have been set" << std::endl;
        return -1.0;
    }

    try
    {
        double val = _biology->get_efficiency(gene);
        std::cout << gene << ": " << val << std::endl;
        return val;
    }
    catch (const std::out_of_range&)
    {
        std::cerr << "Key error! Genetics had no key " << gene << std::endl;
        return -1.0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception getting genetic value: " << e.what() << std::endl;
        return -1.0;
    }
}

std::pair<double, double> Entity::biology_movement(const std::string& terrain)
{
    double edrain = 0.0;
    double wdrain = 0.0;

    if (_biology == nullptr)
    {
        std::cerr << "Warning: There is no biology." << std::endl;
        return std::make_pair(edrain, wdrain);
    }

    try
    {
        edrain = _biology->movement_energy_drain(terrain);
    }
    catch (const std::out_of_range&)
    {
        edrain = 0.0;
    }
    catch (const std::exception&)
    {
        edrain = 0.0;
    }

    try
    {
        wdrain = _biology->movement_water_drain(terrain);
    }
    catch (const std::out_of_range&)
    {
        wdrain = 0.0;
    }
    catch (const std::exception&)
    {
        wdrain = 0.0;
    }

    std::cout << "Energy drain for " << terrain << ": " << edrain << std::endl
              << "Water drain for " << terrain << ": " << wdrain << std::endl;

    return std::make_pair(edrain, wdrain);
}
