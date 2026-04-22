#pragma once

#include <vector>

// Activation layer equations
double relu(double x);
double tanh_func(double x);
double sigmoid(double x);
double dot_product(std::vector<double> v1, std::vector<double> v2);

// Layer functions
class ActivationLayerReLU {
private:
    std::vector<double> weights;
    std::vector<double> biases;
    int n_in, n_out;

public:
    ActivationLayerReLU(int input_size, int output_size);
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

public:
    Brain(std::vector<int> layer_sizes);
    int decide(const std::vector<double>& input);
    int get_layer_count() const { return layers.size(); }
};