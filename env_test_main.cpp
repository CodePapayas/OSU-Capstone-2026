#include "./source/Environment.h"
#include <iostream>
#include <cassert>      // for testing
#include "./source/MathVector.hpp"
#include "./source/PerlinNoise.hpp"

void test_DisplayEnvironment(Environment *env){
    std::cout << "Generating environment grid..." << std::endl;
    int chunks = env->getChunksInEnvironment();
    int tiles = env->getTilesPerChunk();
    int env_size = tiles * chunks;
    std::cout << "Displaying " << env_size << "x" << env_size << " tile environment grid." << std::endl;
    for(int y = 0; y < env_size; y++){
        for(int x = 0; x < env_size; x++){
            std::cout << env->getTileValue(Vector2d(x,y)) << "\t ";
        }
        std::cout << std::endl;
    }
}

void test_GetTileValue(Environment *env){
    std::cout << "Testing bounds..." << std::endl;
    int chunks = env->getChunksInEnvironment();
    int tiles = env->getTilesPerChunk();
    std::cout << "Below zero..." << std::endl;
    std::cout << env->getTileValue(Vector2d(0,-1)) << "\t ";
    std::cout << env->getTileValue(Vector2d(-1, 0)) << "\t ";
    std::cout << env->getTileValue(Vector2d(-1,-1)) << "\t ";
    std::cout << "Above limit..." << std::endl;
    std::cout << env->getTileValue(Vector2d(0, chunks * tiles)) << "\t ";
    std::cout << env->getTileValue(Vector2d(chunks * tiles, 0)) << "\t ";
    std::cout << env->getTileValue(Vector2d(chunks * tiles, chunks * tiles)) << "\t\n ";
}


void test_GetChunkFromID(Environment *env){
    std::cout << "Get chunk from id..." << std::endl;
    int get_id = 0;

    for(int x = 0; x <  env->getChunksInEnvironment() ; x++){
        for(int y = 0; y <  env->getChunksInEnvironment() ; y++){
            Vector2d curr_chunk = env->getChunkFromID(get_id);
            assert(curr_chunk.x == x && curr_chunk.y == y);
            get_id++;
        }
    }
}

void test_SampleNoiseLayered(){
    PerlinNoise2d noise = PerlinNoise2d();
    noise.SetFrequency(1);
    noise.SetAmplitude(200);
    noise.SetOctaves(8);
    for(int y = 0; y < 16; y++){
        for(int x = 0; x < 16; x++){
            std::cout << noise.SampleNoiseLayered(Vector2d(x,y)) << "\t ";
        }
        std::cout << std::endl;
    }
}

int main(){
    Environment height_map; // generates a 2d array of normalized floats
    test_DisplayEnvironment(&height_map);
    test_GetChunkFromID(&height_map);
    test_GetTileValue(&height_map);
    test_SampleNoiseLayered();
    return 0;
}