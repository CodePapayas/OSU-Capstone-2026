#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include "./tile.h"

class Environment{
	public:
		std::vector<std::vector<Tile>> *tiles;
		
		// current iteration will sample terrain and chemical data from a predefined heightmap image for simplicity
		void	GenerateFromValueNoise(){
			srand(time(NULL));
			int count = 0;
			for(int x = 0; x < width; x++){
				for(int y = 0; y < height; y++){
					(*tiles)[x][y] = Tile(count, environment_id, x, y, (rand() % 9));
					(*tiles)[x][y].AddChemical(rand() % 40 + 1, rand() % 999 + 1);
					count++;
				}
			}
		};
		void	GenerateFromPerlinNoise();
		void	GenerateFromImages(std::string tile_filepath, std::string chem_filepath);
		void 	GenerateFromLUA();
		
		// methods for SQL-handling
		void 	FromSQL();
		void 	ToSQL();
		
		Environment(int id, const int x, const int y) {
            width = x;
            height = y;
            tiles = new std::vector<std::vector<Tile>>(x, std::vector<Tile>(y)); // access is via tile[x][y].attribute, all tiles are uninitialized
        };
		
	private:
		int environment_id;
		int simulation_id;
		int width;
		int height;
		
};

#endif