#pragma once

#include <vector>
#include <random>

// Activation layer equations
double relu(double x);
double tanh_func(double x);
double sigmoid(double x);
double dot_product(std::vector<double> v1, std::vector<double> v2);
std::vector<double> soft_max(const std::vector<double>& V);

// Layer functions
class ActivationLayerReLU {
private:
    std::vector<double> weights;
    std::vector<double> biases;
    int n_in, n_out;

public:
    ActivationLayerReLU(int input_size, int output_size);
    void ActivationLayerReLUOffsping(const std::vector<double>& weights, const std::vector<double>& biases);
    std::vector<double> forward(const std::vector<double>& input);
    int get_weight_count() const { return weights.size(); }
    int get_biases_count() const { return biases.size(); }
    const std::vector<double>& get_weights() const;
    const std::vector<double>& get_biases() const;
};

// Brain Class
// multi-layer neural network using ReLU activation layers.
// constructor takes layer_sizes vector defining network architecture (e.g., {5, 8, 8, 6} = 5 inputs, 8 and 8 hidden layer inputs, 6 outputs)
class Brain {
private:
    std::vector<ActivationLayerReLU> layers;
    std::mt19937 gen;

public:
    Brain(std::vector<int> layer_sizes, unsigned int seed = std::random_device{}());
    int decide(const std::vector<double>& input);
    int softDecide(const std::vector<double> &input);
    int get_layer_count() const { return layers.size(); }
    std::vector<ActivationLayerReLU>& get_layers();
    void set_layers(const std::vector<ActivationLayerReLU>& new_layers) { layers = new_layers; }
};