#include "ImGuiVisualizer.hpp"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    string csvPath = "simulation_stats_out.csv";
    if (argc > 1) csvPath = argv[1];

    ImGuiVisualizer viz;

    if (!viz.loadCSV(csvPath)) {
        cerr << "Warning: Could not load '" << csvPath << "'.\n"
                  << "The dashboard will open empty. Use the Controls panel to load a CSV.\n";
    }

    viz.run();
    return 0;
}
