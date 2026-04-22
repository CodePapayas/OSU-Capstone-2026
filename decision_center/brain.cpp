#include <cmath>
#include <vector>
#include <numeric>
#include <random>
#include <ctime>
#include <algorithm>
#include "brain.hpp"

// Hyperbolic Tangent (tanh) Activation Function
// maps any real output to between [-1, 1]
double tanh_func(double x) {
    return std::tanh(x);
}

// Sigmoid Activation Function
// maps any real output to between [0, 1]
double sigmoid(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

// Rectified Linear Unit (ReLU) Activation Function
// outputs input directly if it is positive; otherwise outputs zero.
double relu(double x) {
    return x > 0 ? x : 0;
}

// Dot Product Function [adapted heavily from https://www.educative.io/answers/dot-product-of-two-vectors-in-cpp]
// computes the sum of element-wise products of two vectors.
double dot_product(std::vector<double> v1, std::vector<double> v2) {
    double result = 0;
    for (int i = 0; i < v1.size(); ++i) {
        result += v1[i] * v2[i];
    }
    return result;
}

/**
// ReLU Activation Layer Class
// fully connected layer with ReLU activation applied to outputs.
*/

// Constructor for ReLU Activation Layer
// initializes weights and biases with random values between [-1.0, 1.0]
ActivationLayerReLU::ActivationLayerReLU(int input_size, int output_size) {
    n_in = input_size;
    n_out = output_size;
    weights.resize(input_size * output_size);
    biases.resize(n_out);
    std::mt19937 gen(std::time(nullptr));
    std::uniform_real_distribution<double> dist(-1.0, 1.0);


    for (double& b : biases) {
        b = dist(gen);
    }

    for (double& w : weights) {
        w = dist(gen);
    }
}

// Forward Pass
// computes weighted sum plus bias, then applies ReLU activation to each output neuron.
std::vector<double> ActivationLayerReLU::forward(const std::vector<double>& input) {
    std::vector<double> output(n_out);

    for (int i = 0; i < n_out; ++i) {
        std::vector<double> weight_row(n_in);
        for (int j = 0; j < n_in; ++j) {
            weight_row[j] = weights[i * n_in + j];
        }

        double z = dot_product(weight_row, input) + biases[i];

        output[i] = relu(z);
    }
    return output;
}

// returns the bias vector
const std::vector<double>& ActivationLayerReLU::get_biases() const{
    return biases;
}

// returns the flattened weight matrix (stored row-major)
const std::vector<double>& ActivationLayerReLU::get_weights() const{
    return weights;
}


/**
// Brain Class
// array of activation layers; returns the index of the maximum value.
*/

// Constructor for the brain
Brain::Brain(std::vector<int> layer_sizes) {
    
    for (int i = 0; i < layer_sizes.size() - 1; ++i) {
        int n_in = layer_sizes[i];
        int n_out = layer_sizes[i+1];
        layers.push_back(ActivationLayerReLU(n_in, n_out));
    }
}

// Argmax decision center
int Brain::decide(const std::vector<double>& input) {
    std::vector<double> current_output = input;
    
    for (int i = 0; i < layers.size(); ++i) {
        current_output = layers[i].forward(current_output);
    }

    int max_index = std::max_element(current_output.begin(), current_output.end()) - current_output.begin();
    return max_index;
}