/*
 *  Dillon Stickler - Oregon State University - 2026
 */

#include "Environment.h"
#include "MathVector.hpp"
// Environment Class
// Responsible for creating, handling, and accessing the simulation environment data.
// Agents and other simulation entities can access specific data necessary through
// accessing the environment, the desired chunk, and the desired tile in the chunk.

// ------------------------------[TO-DO]------------------------------
// - Implement perlin noise
// - Optimize data storage to be faster
// - More getters and setters, just cause
// - Refactor tiles and chunks to remain ungenerated until accessed
//      - Likely won't be until we get Agent functionality implemented
// - Adjust the global variables chunk_amt and tile_amt to be configurable at runtime
//      - ... and by extent, adjust the "chunks" and "tiles" arrays/vectors/lists to accomodate this
// - Memory management considerations
//      - Mainly pointer cleanup on deconstruction
// - Class method overloads to allow integer parameters instead of Vector2d

// Tile class, holds environment data at coordinate position.
class Tile
{
	std::unordered_map<std::string, double> values; // currently an arbitrary value for tracking things like noise
    std::string terrain_type; // placeholder for now, will be used to track the type of terrain for the tile, which will affect movement and energy drain for entities on it. Will likely be an int or enum in practice, but string is easier for testing for now.
	public:
		Tile() {
            terrain_type="Terrain Efficiency " + std::to_string(rand() % 3 + 1); // placeholder random terrain type
            values = {{"height", 0},{"moisture", 0},{"curr_temp", 0}};
        };
        std::unordered_map<std::string, double> getValues(){return values;};
        void setValues(std::unordered_map<std::string, double> m){values = m;};
        double getValue(std::string key){return values[key];};
        void setValue(std::string key, double v){values[key] = v;};
        std::string getTerrainType(){return terrain_type;};
};

// constructor for environment
// currently, generates the whole chunk grid
Environment::Environment(int size_x, int size_y)
{
    _size_x = size_x;
    _size_y = size_y;
    for(int x = 0; x < _size_x; x++){
        std::vector<Tile*> tile_col;
        for(int y = 0; y < _size_y; y++){
            int curr_id = tile_map.size();
            tile_map[curr_id] = Vector2d(x,y);
            tile_col.push_back(new Tile());
        }
        tiles.push_back(tile_col);
    }
};

// function that converts and clamps passed position data to chunk, tile coordinates of range [0, chunks * tiles per chunk - 1].
Vector2d Environment::boundCoords(Vector2d pos)
{
    // Converts absolute position coordinates to the array index system.
    // Takes in the pos value and clamps it to the range of the chunks array and the tiles array inside chunks.
    // Input: Vector2d pos;
    // Ouput: Vector2d tile_pos;
    
    int tile_x = pos.x;
    int tile_y = pos.y;

    tile_x = (tile_x < 0) ? 0 : tile_x;
    tile_x = (tile_x < _size_x) ? tile_x : (_size_x - 1);
    tile_y = (tile_y < 0) ? 0 : tile_y;
    tile_y = (tile_y < _size_y) ? tile_y : (_size_y - 1);

    Vector2d tile_pos =	Vector2d(tile_x, tile_y);
    
    return tile_pos;
}

Tile *Environment::getTile(Vector2d pos)
{
    pos = boundCoords(pos);

    Tile *curr_tile = tiles[pos.x][pos.y];
    return curr_tile;
}

std::unordered_map<std::string, double> Environment::getTileValues(Vector2d pos)
{
    // input: Vector2 position; Desired X,Y coordinate access in environment
    // ouput: double value; the value in the tile

    std::unordered_map<std::string, double> result = getTile(pos)->getValues();
    return result;
};

void Environment::setTileValues(Vector2d pos, std::unordered_map<std::string, double> v)
{
    getTile(pos)->setValues(v);
}

double Environment::getTileValue(Vector2d pos, std::string key)
{
    // input: Vector2 position; Desired X,Y coordinate access in environment
    // ouput: double value; the value in the tile
    return getTile(pos)->getValues()[key];
};

std::string Environment::getTileType(Vector2d pos)
{
    return getTile(pos)->getTerrainType();
}

void Environment::setTileValue(Vector2d pos, double v, std::string key)
{
    getTile(pos)->setValue(key, v);
}

Vector2d Environment::getTileFromID(int id)
{
    // input: int ID; Desired chunk id
    // output: Vector2d chunk_coord; The coordinate position of the chunk.
    Vector2d *chunk_coord = &tile_map[id];
    if(chunk_coord){
        return *chunk_coord;
    }
    return Vector2d(-1, -1);
};