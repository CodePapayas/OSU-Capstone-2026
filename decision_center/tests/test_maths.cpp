#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../brain.hpp"

TEST_CASE("Activation Function Tests") {
    // ReLU Tests
    CHECK(relu(1) == 1);
    CHECK(relu(-1) == 0);

    // Sigmoid Tests
    double log_x = std::log(3);
    CHECK(sigmoid(0) == 0.5);
    CHECK(sigmoid(log_x) == 0.75);
    CHECK(sigmoid(-log_x) == 0.25);

    // Dot Product Tests
    std::vector<double> vector1 = {1.0, 4.0, 5.2};
    std::vector<double> vector2 = {1.5, 4.5, 5.5};
    std::vector<double> vector3 = {1.0, 4.0, 5.2, 6.9, 6.7, 0.0, 2.35, 3.25435};
    std::vector<double> vector4 = {1.5, 4.5, 5.5, 9.6, 7.6, 10.0, 3.52, 6.25435};
    std::vector<double> vectorNeg1 = {-1.0, 4.0, -5.2};
    std::vector<double> vectorNeg2 = {1.5, -4.5, 5.5};
    CHECK(dot_product(vector1, vector2) == 48.1);
    CHECK(dot_product(vectorNeg1, vectorNeg2) == -48.1);
    CHECK(dot_product(vector3, vector4) == doctest::Approx(193.886).epsilon(1e-6));

    // Activation Layer Tests - check returned vector size (n_out * n_in)
    ActivationLayerReLU layer(2, 2);
    CHECK(layer.get_weight_count() == 4);
    CHECK(layer.get_biases_count() == 2);
    std::vector<double> biases = layer.get_biases();
        for (double b : biases) {
            CHECK(b >= -1.0);
            CHECK(b <= 1.0);
        }
    std::vector<double> weights = layer.get_weights();
        for (double w : weights) {
            CHECK(w >= -1.0);
            CHECK(w <= 1.0);
        }

    // Forward method test - positive inputs
    // with positive inputs, ReLU should produce positive or zero outputs
    ActivationLayerReLU layerPos(3, 2);
    std::vector<double> inputPos = {1.0, 2.0, 3.0};
    std::vector<double> outputPos = layerPos.forward(inputPos);
    CHECK(outputPos.size() == 2);
    for (double val : outputPos) {
        CHECK(val >= 0.0);
    }

    // Forward method test - negative inputs
    // with negative inputs, ReLU may zero out some or all outputs depending on weights
    ActivationLayerReLU layerNeg(3, 2);
    std::vector<double> inputNeg = {-1.0, -2.0, -3.0};
    std::vector<double> outputNeg = layerNeg.forward(inputNeg);
    CHECK(outputNeg.size() == 2);
    for (double val : outputNeg) {
        CHECK(val >= 0.0);
    }
}

TEST_CASE("Brain and Decision Center Tests") {
    Brain brain({5, 8, 8, 6});
    CHECK(brain.get_layer_count() == 3);

    // Test that Brain::decide() works correctly
    std::vector<double> input1 {1.0, 2.0, 3.0, 4.0, 5.0};
    int decision = brain.decide(input1);
    CHECK(decision >= 0);
    CHECK(decision < 5);

    // Test determinism
    std::vector<double> input2 {1.0, 1.1, 1.2, 1.3, 1.4};
    int decision_a = brain.decide(input2);
    int decision_b = brain.decide(input2);
    CHECK(decision_a == decision_b);
}