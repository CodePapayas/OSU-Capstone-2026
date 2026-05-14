#include <stdlib.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include "environment.h"
#include "tile.h"

int main(){ 

    // generate a list of all chemicals
    // somehow feed that into tiles as a full list of all chemicals as quantities

    int width = 10;
    int height = 20;

    Environment env1(1, width, height);
    Environment env2(2, width, height);

    env1.GenerateFromValueNoise();

    for(int x = 0; x < 10; x++){
		for(int y = 0; y < 20; y++){
            std::cout << (*env1.tiles)[x][y].GetHeight() << " ";
		}
        std::cout << std::endl;
	}

    std::cout << std::endl;

    int count = 0;
    std::vector<std::vector<int>> height_buffer(width, std::vector<int>(height, 0)); 
	for(int x = 0; x < 10; x++){
		for(int y = 0; y < 20; y++){
            int average = (*env1.tiles)[x][y].GetHeight();
            int amount = 1;
            if (x > 0)      {average += (*env1.tiles)[x-1][y].GetHeight(); amount++;};
            if (x < width-1)  {average += (*env1.tiles)[x+1][y].GetHeight(); amount++;};
            if (y > 0)      {average += (*env1.tiles)[x][y-1].GetHeight(); amount++;};
            if (y < height-1) {average += (*env1.tiles)[x][y+1].GetHeight(); amount++;};
            average /= amount;
			count++;
            height_buffer[x][y] = average;
            std::cout << height_buffer[x][y] << " ";
		}
        std::cout << std::endl;
	}
    return 0;
}