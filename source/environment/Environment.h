/*
 *  Dillon Stickler - Oregon State University - 2026
 */

#pragma once

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <tuple>
#include <iostream>
#include <vector>       // added vector dep to change up arrays
#include <unordered_map>
#include <string>
#include "MathVector.hpp"
#include "../environment/PerlinNoise.hpp"

// placeholder classes for functionality, may be extrapolated into their own files later
class Vector2d;
class Tile;
class Chunk;

class Environment
{
private:    
    int _size_x;
    int _size_y;
    std::vector<std::vector<Tile*>> tiles;    
    std::unordered_map <int, Vector2d> tile_map; // get the X,Y for it for simplicity
    PerlinNoise2d height_noise;
    PerlinNoise2d moisture_noise;
    PerlinNoise2d temperature_noise;
    PerlinNoise2d temperature_multiplier;
    Vector2d temperature_movement;
public:
    Environment(int size_x, int size_y, PerlinNoise2d *perlin = nullptr);
    Vector2d boundCoords(Vector2d pos);

    Tile *getTile(Vector2d pos);
    std::vector<double> getTileValues(Vector2d pos);
    void setTileValues(Vector2d pos, std::vector<double> v);
    double getTileValue(Vector2d pos, int index);
    std::string getTileType(Vector2d pos);
    void setTileValue(Vector2d pos, double v, int index);

    int getTileAmountX() {return tiles.size();};
    int getTileAmountY() {return tiles[0].size();};
    int getTileArea() {return tiles[0].size() * tiles.size();};
    void updateTiles();
    Vector2d getTileFromID(int id);
    double calculateTemperature(Vector2d pos);
};

#endif