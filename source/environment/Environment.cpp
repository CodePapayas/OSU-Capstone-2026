/*
 *  Dillon Stickler - Oregon State University - 2026
 */

#include "Environment.h"

// Environment Class
// Responsible for creating, handling, and accessing the simulation environment data.
// Agents and other simulation entities can access specific data necessary through
// accessing the environment, the desired chunk, and the desired tile in the chunk.

// ------------------------------[TO-DO]------------------------------
// - Optimize data storage to be faster
// - More getters and setters, just cause
// - Memory management considerations
//      - Mainly pointer cleanup on deconstruction
// - Class method overloads to allow integer parameters instead of Vector2d?

// Tile class, holds environment data at coordinate position.
class Tile{
	std::vector<double> values; // currently an arbitrary value for tracking things like noise
    double current_temp;
    double current_moisture;
    std::string terrain_type; // placeholder for now, will be used to track the type of terrain for the tile, which will affect movement and energy drain for entities on it. Will likely be an int or enum in practice, but string is easier for testing for now.
	public:
		Tile(std::vector<double> value_list) {
            terrain_type="Terrain Efficiency " + std::to_string(rand() % 3 + 1); // placeholder random terrain type
            for(auto value : value_list){
                values.push_back(value); //placeholder random value noise
            }
        };
        std::vector<double> getValues(){return values;};
        void setValues(std::vector<double> v){values = v;};
        void setValue(double v, int index){values[index] = v;};
        void    setTemperature(double t) {current_temp = t;};
        double  getTemperature() {return current_temp;};
        void    setMoisture(double m) {current_moisture = m;};
        double  getMoisture() {return current_moisture;};
        void updateConditions();
        std::string getTerrainType(){return terrain_type;};
        
};

// constructor for environment
// currently, generates the whole chunk grid
Environment::Environment(int size_x, int size_y, PerlinNoise2d *perlin){
    _size_x = size_x;
    _size_y = size_y;

    height_noise =              (perlin != nullptr) ? perlin[0] : PerlinNoise2d(1234, 0.025, 1.0, 8);
    temperature_noise =         (perlin != nullptr) ? perlin[1] : PerlinNoise2d(2345, 0.015, 1.0, 2);
    temperature_multiplier =    (perlin != nullptr) ? perlin[2] : PerlinNoise2d(3456, 0.5, 1.0, 4);
    moisture_noise =            (perlin != nullptr) ? perlin[3] : PerlinNoise2d(3456, 0.5, 0.1, 4);
    temperature_movement =      Vector2d(0,0);

    for(int x = 0; x < _size_x; x++){
        std::vector<Tile*> tile_col;
        for(int y = 0; y < _size_y; y++){
            int curr_id = tile_map.size();
            tile_map[curr_id] = Vector2d(x,y);
            Vector2d pos = Vector2d(x,y);
            tile_col.push_back(new Tile({height_noise.SampleLayered(pos)}));
            tile_col[tile_col.size()-1]->setTemperature(calculateTemperature(pos));
            tile_col[tile_col.size()-1]->setMoisture(moisture_noise.SampleLayered(pos));
        }
        tiles.push_back(tile_col);
    }
};

// function that converts and clamps passed position data to chunk, tile coordinates of range [0, chunks * tiles per chunk - 1].
Vector2d Environment::boundCoords(Vector2d pos){
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

Tile *Environment::getTile(Vector2d pos){
    pos = boundCoords(pos);

    Tile *curr_tile = tiles[pos.x][pos.y];
    return curr_tile;
}

std::vector<double> Environment::getTileValues(Vector2d pos){
    // input: Vector2 position; Desired X,Y coordinate access in environment
    // ouput: double value; the value in the tile

    std::vector<double> result = getTile(pos)->getValues();
    return result;
};

void Environment::setTileValues(Vector2d pos, std::vector<double> v){
    getTile(pos)->setValues(v);
}

double Environment::getTileValue(Vector2d pos, int index){
    // input: Vector2 position; Desired X,Y coordinate access in environment
    // ouput: double value; the value in the tile
    return getTile(pos)->getValues()[index];
};

std::string Environment::getTileType(Vector2d pos){
    return getTile(pos)->getTerrainType();
}

void Environment::setTileValue(Vector2d pos, double v, int index){
    getTile(pos)->setValue(v, index);
}

Vector2d Environment::getTileFromID(int id){
    // input: int ID; Desired chunk id
    // output: Vector2d chunk_coord; The coordinate position of the chunk.
    Vector2d *chunk_coord = &tile_map[id];
    if(chunk_coord){
        return *chunk_coord;
    }
    return Vector2d(-1, -1);
};

void Environment::updateTiles(){
    temperature_movement = Vector2d(temperature_movement.x++, temperature_movement.y++);
    for(int x = 0; x < _size_x; x++){
        for(int y = 0; y < _size_y; y++){
            Vector2d pos = Vector2d(x,y);
            tiles[x][y]->setTemperature(calculateTemperature(pos));
        }
    }
};

double Environment::calculateTemperature(Vector2d pos){
        Vector2d temp_shift_pos = Vector2d(pos.x + temperature_movement.x, pos.y + temperature_movement.y);
        double temp_avg = temperature_noise.SampleLayered(pos);      
        double temp_shift = temperature_noise.SampleLayered(temp_shift_pos);       
        double temp_mult = temperature_multiplier.SampleLayered(pos);
        double curr_temp =  temp_avg + (temp_mult * temp_shift);
        return curr_temp;
};