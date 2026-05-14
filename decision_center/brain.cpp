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

    // temporary fix to an issue regarding the perception inputs through V2 not matching the size of the weights through V1
    // currently, will not work if the sizes are not equal, so we are just capping it until we find a better way to pad out values with understanding

    int min = (v1.size() < v2.size()) ? v1.size() : v2.size();
    for (int i = 0; i < min; ++i) {
        result += v1[i] * v2[i];
    }
    return result;
}

//Softmax() function
// Source: https://github.com/ZigaSajovic/dCpp/blob/master/examples/softmax.cpp
std::vector<double> soft_max(const std::vector<double>& V) {
    if (V.empty()) return {};

    double max_val = *std::max_element(V.begin(), V.end());

    std::vector<double> output;
    output.reserve(V.size());

    std::vector<double> exp_vals;
    exp_vals.reserve(V.size());

    double sum = 0.0;

    // Shift by max_val, exponentiate, sum
    for (double v : V) {
        double e = std::exp(v - max_val);
        exp_vals.push_back(e);
        sum += e;
    }

    // Normalize
    for (double e : exp_vals) {
        output.push_back(e / sum);
    }

    return output;
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
        b = 0.01;
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

        // concerns for weigh-perception mismatching visualized

        // 0, 0, 2, 1, 0, 2 
        // 0, 1, 2, 1, 0, 1

        // 0, 0, 2, 1, 0, 2 
        // 1, 2, 1, 1

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
Brain::Brain(std::vector<int> layer_sizes, unsigned int seed) : gen(seed) {
    
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

std::vector<ActivationLayerReLU>& Brain::get_layers() {
    return layers;
}

//Softmax decision center
int Brain::softDecide(const std::vector<double>& input) {
    std::vector<double> current_output = input;

    for (int i = 0; i < layers.size(); ++i) {
        current_output = layers[i].forward(current_output);
    }

    std::vector<double> probabilities = soft_max(current_output);

    std::discrete_distribution<int> dist(probabilities.begin(), probabilities.end());

    return dist(this->gen);

}