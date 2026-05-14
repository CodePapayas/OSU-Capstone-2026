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
#include "MathVector.hpp"

const int chunk_amt = 3;
const int tile_amt = 9;

// placeholder classes for functionality, may be extrapolated into their own files later
class Vector2d;
class Tile;
class Chunk;


class Environment
{
private:
    double x_origin = ((double)(chunk_amt * tile_amt)) / 2.0;
    double y_origin = ((double)(chunk_amt * tile_amt)) / 2.0;
    std::vector<std::vector<Chunk*>> chunks;    
    std::unordered_map <int, Vector2d> chunk_map; // get the X,Y for it for simplicity
public:
    Environment();
    std::tuple<Vector2d, Vector2d> toChunkCoords(Vector2d pos);
    float getTileValue(Vector2d pos);
    int getTilesPerChunk()          {return tile_amt;};
    int getChunksInEnvironment()    {return chunk_amt;};
    Vector2d getChunkFromID(int id);
};

#endif