#ifndef TILE_H
#define TILE_H

#include <iostream>
#include <vector>
#include <algorithm>

class Tile{
	private:
		int 	tile_id;
		int 	environment_id;
		int 	x_coord;
		int 	y_coord;
		float 	height;
        int     neighbor_ids[4];
		
		// struct to track chemical contents: type, quantity
		// for now, doesn't need to be actual chemical
		// agent can harvest/supply the chemical through a method

	public:
		// constructor
		Tile() {
			tile_id = -1;
			environment_id = -1;
			x_coord = -1;
			y_coord = -1;
			height = 0;
		};
		
		Tile(int id, int env_id, int x, int y, int z) : Tile(){
			tile_id = id;
			environment_id = env_id;
			x_coord = x;
			y_coord = y;
			height = z;
		};

		struct ChemicalTable{
			int chemical_id = -1;
			float quantity 	= 0.0f;
		};

		std::vector<ChemicalTable> chemicals; // ideally pre-ordered to be chemicals [0-id], initialized with quantity = 0

		// getters
		int GetTile_id() 			{return tile_id;}
		int GetEnvironment_id() 	{return environment_id;}
		int GetHeight() 			{return height;}
		
		// setters

		void SetHeight(int z) 			{height = z;}

		// tile-chemical interactions
		void AddChemical(int id, float amount){
			auto chem_found = std::find_if(chemicals.begin(), chemicals.end(), [&id](const ChemicalTable& c){return c.chemical_id == id;});
			if (chem_found != chemicals.end()){
				(*chem_found).quantity += amount;
			} else {
				ChemicalTable new_chem = {id, amount};
				chemicals.push_back(new_chem);
			}
		};

		int ReduceChemical(int id, float amount){
			int reduced = 0;
			chemicals[id].quantity -= amount;
			if(chemicals[id].quantity < 0){
				reduced = amount + chemicals[id].quantity;
			} else {
				reduced = amount;
			}
			return reduced;
		};
};

#endif