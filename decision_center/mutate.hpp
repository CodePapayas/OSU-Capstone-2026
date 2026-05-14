#include <cmath>
#include <vector>
#include <numeric>
#include <random>
#include <ctime>
#include <algorithm>
#include <unordered_map>

const double MUTATION_CHANCE = 0.02; // 1% chance to mutate each weight or bias

std::vector<double> mutate_vector(const std::vector<double>& original);
std::unordered_map<std::string, double> mutate_genetics(const std::unordered_map<std::string, double>& original);