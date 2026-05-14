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
class Tile{
	float value = 0.0f; // currently an arbitrary value for tracking things like noise
	public:
		Tile() {
            value = (float)(rand() % 11) / 10.0f; //placeholder random value noise
        };
        float getValue(){return value;};          //
};

// Chunk class, holds multiple tiles
class Chunk{                              
    std::vector<std::vector<Tile*>> tiles;        // vector array for holding tiles
    std::unordered_map <int, Vector2d> tile_map;  // key is tile ID, value is a vector2d with the [x][y] position of the tile in the 2d array tiles
    public:
    	Chunk(){
            int tile_id = 0;
            for(int x = 0; x < tile_amt; x++){
                std::vector<Tile*> tile_col;
                for(int y = 0; y < tile_amt; y++){
                    tile_col.push_back(new Tile());
                    tile_map[tile_id] = Vector2d(x,y);
                }
                tiles.push_back(tile_col);
            }
        };	
        Tile *getTile(int x, int y){
            return tiles[x][y];
        }
};

// constructor for environment
// currently, generates the whole chunk grid
Environment::Environment(){
    for(int x = 0; x < chunk_amt; x++){
        std::vector<Chunk*> chunk_col;
        for(int y = 0; y < chunk_amt; y++){
            int curr_id = chunk_map.size();
            chunk_map[curr_id] = Vector2d(x,y);
            chunk_col.push_back(new Chunk());
        }
        chunks.push_back(chunk_col);
    }
};

// function that converts and clamps passed position data to chunk, tile coordinates of range [0, chunks * tiles per chunk - 1].
std::tuple<Vector2d, Vector2d> Environment::toChunkCoords(Vector2d pos){
    // Converts absolute position coordinates to the array index system.
    // Takes in the pos value and clamps it to the range of the chunks array and the tiles array inside chunks.
    // Input: Vector2d pos;
    // Ouput: Vector2d chunk_pos, Vector2d tile_pos;
    
    int true_x = 	pos.x; //pos.x - x_origin;
    int true_y = 	pos.y; //pos.y - y_origin; // got ambitious here, wanted to account for if (0,0) was the center of the grid and not the top left corner
    
    int chunk_x = 	(true_x / tile_amt) % chunk_amt;
    int chunk_y = 	(true_y / tile_amt) % chunk_amt;
    
    int tile_x = 	true_x % tile_amt;
    int tile_y = 	true_y % tile_amt;
    
    // keep chunk position bound
    if (chunk_x < 0)                {chunk_x = 0;}
    else if (chunk_x >= chunk_amt)  {chunk_x = chunk_amt - 1;}
    if (chunk_y < 0)                {chunk_y = 0;}
    else if (chunk_y >= chunk_amt)  {chunk_y = chunk_amt - 1;}

    if (tile_x < 0)                {tile_x = 0;}
    else if (tile_x >= tile_amt)  {tile_x = tile_amt - 1;}
    if (tile_y < 0)                {tile_y = 0;}
    else if (tile_y >= tile_amt)  {tile_y = tile_amt - 1;}
    
    Vector2d chunk_pos = 	Vector2d(chunk_x, chunk_y);
    Vector2d tile_pos =		Vector2d(tile_x, tile_y);
    
    return {chunk_pos, tile_pos};
}

float Environment::getTileValue(Vector2d pos){
    // input: Vector2 position; Desired X,Y coordinate access in environment
    // ouput: float value; the value in the tile
    auto[chunk_pos, tile_pos] = this->toChunkCoords(pos);

    Chunk *curr_chunk   = chunks[(int)chunk_pos.x][(int)chunk_pos.y];
    Tile *curr_tile     = curr_chunk->getTile((int)tile_pos.x, (int)tile_pos.y);

    float result          = curr_tile->getValue();
    return result;
};

Vector2d Environment::getChunkFromID(int id){
    // input: int ID; Desired chunk id
    // output: Vector2d chunk_coord; The coordinate position of the chunk.
    Vector2d *chunk_coord = &chunk_map[id];
    if(chunk_coord){
        return *chunk_coord;
    }
    return Vector2d(-1, -1);
};