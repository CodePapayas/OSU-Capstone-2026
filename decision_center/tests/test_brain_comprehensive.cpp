/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462 
  
  Comprehensive Brain/Neural Network Tests
  Coverage: Activation functions, dot products, edge cases, numerical stability
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../decision_center/tests/doctest.h"
#include "../decision_center/brain.hpp"
#include <cmath>
#include <vector>
#include <limits>

TEST_CASE("ReLU Activation Function - Comprehensive") {
    SUBCASE("Basic positive values") {
        CHECK(relu(1.0) == 1.0);
        CHECK(relu(5.5) == 5.5);
        CHECK(relu(100.0) == 100.0);
    }
    
    SUBCASE("Zero and negative values") {
        CHECK(relu(0.0) == 0.0);
        CHECK(relu(-1.0) == 0.0);
        CHECK(relu(-5.5) == 0.0);
        CHECK(relu(-100.0) == 0.0);
    }
    
    SUBCASE("Very small values") {
        CHECK(relu(0.0001) == 0.0001);
        CHECK(relu(-0.0001) == 0.0);
        CHECK(relu(1e-10) == 1e-10);
    }
    
    SUBCASE("Very large values") {
        CHECK(relu(1e6) == 1e6);
        CHECK(relu(1e10) == 1e10);
    }
}

TEST_CASE("Sigmoid Activation Function - Comprehensive") {
    SUBCASE("Standard values") {
        CHECK(sigmoid(0.0) == doctest::Approx(0.5).epsilon(1e-6));
        
        double log_3 = std::log(3.0);
        CHECK(sigmoid(log_3) == doctest::Approx(0.75).epsilon(1e-6));
        CHECK(sigmoid(-log_3) == doctest::Approx(0.25).epsilon(1e-6));
    }
    
    SUBCASE("Boundary behavior") {
        // Sigmoid should approach 1 for large positive values
        CHECK(sigmoid(10.0) > 0.9999);
        CHECK(sigmoid(100.0) > 0.9999);
        
        // Sigmoid should approach 0 for large negative values
        CHECK(sigmoid(-10.0) < 0.0001);
        CHECK(sigmoid(-100.0) < 0.0001);
    }
    
    SUBCASE("Symmetry around 0.5") {
        // sigmoid(-x) + sigmoid(x) should equal 1
        CHECK(sigmoid(-1.0) + sigmoid(1.0) == doctest::Approx(1.0).epsilon(1e-6));
        CHECK(sigmoid(-5.0) + sigmoid(5.0) == doctest::Approx(1.0).epsilon(1e-6));
    }
    
    SUBCASE("Monotonicity") {
        // Sigmoid should be monotonically increasing
        CHECK(sigmoid(-5.0) < sigmoid(-4.0));
        CHECK(sigmoid(-1.0) < sigmoid(0.0));
        CHECK(sigmoid(0.0) < sigmoid(1.0));
        CHECK(sigmoid(1.0) < sigmoid(2.0));
    }
    
    SUBCASE("Range bounds") {
        // Sigmoid output should always be in (0, 1)
        CHECK(sigmoid(-100.0) > 0.0);
        CHECK(sigmoid(-100.0) < 1.0);
        CHECK(sigmoid(100.0) > 0.0);
        CHECK(sigmoid(100.0) < 1.0);
    }
}

TEST_CASE("Tanh Activation Function - Comprehensive") {
    SUBCASE("Standard values") {
        CHECK(tanh_func(0.0) == doctest::Approx(0.0).epsilon(1e-6));
        CHECK(tanh_func(1.0) == doctest::Approx(std::tanh(1.0)).epsilon(1e-6));
    }
    
    SUBCASE("Boundary behavior") {
        // Tanh should approach 1 for large positive values
        CHECK(tanh_func(10.0) > 0.9999);
        CHECK(tanh_func(100.0) > 0.9999);
        
        // Tanh should approach -1 for large negative values
        CHECK(tanh_func(-10.0) < -0.9999);
        CHECK(tanh_func(-100.0) < -0.9999);
    }
    
    SUBCASE("Odd function symmetry") {
        // tanh(-x) = -tanh(x)
        CHECK(tanh_func(-1.0) == doctest::Approx(-tanh_func(1.0)).epsilon(1e-6));
        CHECK(tanh_func(-5.0) == doctest::Approx(-tanh_func(5.0)).epsilon(1e-6));
    }
    
    SUBCASE("Range bounds") {
        // Tanh output should always be in (-1, 1)
        CHECK(tanh_func(-100.0) > -1.0);
        CHECK(tanh_func(-100.0) < 1.0);
        CHECK(tanh_func(100.0) > -1.0);
        CHECK(tanh_func(100.0) < 1.0);
    }
}

TEST_CASE("Dot Product - Comprehensive") {
    SUBCASE("Basic orthogonal vectors") {
        std::vector<double> v1 = {1.0, 0.0, 0.0};
        std::vector<double> v2 = {0.0, 1.0, 0.0};
        CHECK(dot_product(v1, v2) == doctest::Approx(0.0).epsilon(1e-6));
    }
    
    SUBCASE("Parallel vectors") {
        std::vector<double> v1 = {1.0, 2.0, 3.0};
        std::vector<double> v2 = {2.0, 4.0, 6.0};  // 2 * v1
        double expected = 2.0 * (1.0 + 4.0 + 9.0);  // 28.0
        CHECK(dot_product(v1, v2) == doctest::Approx(expected).epsilon(1e-6));
    }
    
    SUBCASE("Given test cases") {
        std::vector<double> vector1 = {1.0, 4.0, 5.2};
        std::vector<double> vector2 = {1.5, 4.5, 5.5};
        CHECK(dot_product(vector1, vector2) == doctest::Approx(48.1).epsilon(1e-6));
        
        std::vector<double> vectorNeg1 = {-1.0, 4.0, -5.2};
        std::vector<double> vectorNeg2 = {1.5, -4.5, 5.5};
        CHECK(dot_product(vectorNeg1, vectorNeg2) == doctest::Approx(-48.1).epsilon(1e-6));
        
        std::vector<double> vector3 = {1.0, 4.0, 5.2, 6.9, 6.7, 0.0, 2.35, 3.25435};
        std::vector<double> vector4 = {1.5, 4.5, 5.5, 9.6, 7.6, 10.0, 3.52, 6.25435};
        CHECK(dot_product(vector3, vector4) == doctest::Approx(193.886).epsilon(1e-3));
    }
    
    SUBCASE("Zero vectors") {
        std::vector<double> v1 = {0.0, 0.0, 0.0};
        std::vector<double> v2 = {1.0, 2.0, 3.0};
        CHECK(dot_product(v1, v2) == doctest::Approx(0.0).epsilon(1e-6));
        CHECK(dot_product(v2, v1) == doctest::Approx(0.0).epsilon(1e-6));
    }
    
    SUBCASE("Single element vectors") {
        std::vector<double> v1 = {5.0};
        std::vector<double> v2 = {3.0};
        CHECK(dot_product(v1, v2) == doctest::Approx(15.0).epsilon(1e-6));
    }
    
    SUBCASE("Negative values") {
        std::vector<double> v1 = {-1.0, -2.0, -3.0};
        std::vector<double> v2 = {-1.0, -2.0, -3.0};
        CHECK(dot_product(v1, v2) == doctest::Approx(14.0).epsilon(1e-6));
    }
    
    SUBCASE("Mixed positive and negative") {
        std::vector<double> v1 = {1.0, -2.0, 3.0, -4.0};
        std::vector<double> v2 = {-1.0, 2.0, -3.0, 4.0};
        // 1*-1 + -2*2 + 3*-3 + -4*4 = -1 - 4 - 9 - 16 = -30
        CHECK(dot_product(v1, v2) == doctest::Approx(-30.0).epsilon(1e-6));
    }
    
    SUBCASE("Large vectors") {
        std::vector<double> v1(100, 1.0);
        std::vector<double> v2(100, 2.0);
        CHECK(dot_product(v1, v2) == doctest::Approx(200.0).epsilon(1e-6));
    }
    
    SUBCASE("Commutativity") {
        std::vector<double> v1 = {1.5, 2.5, 3.5};
        std::vector<double> v2 = {4.5, 5.5, 6.5};
        CHECK(dot_product(v1, v2) == doctest::Approx(dot_product(v2, v1)).epsilon(1e-6));
    }
    
    SUBCASE("Precision with small numbers") {
        std::vector<double> v1 = {0.0001, 0.0002, 0.0003};
        std::vector<double> v2 = {0.0001, 0.0002, 0.0003};
        double expected = 0.0001*0.0001 + 0.0002*0.0002 + 0.0003*0.0003;
        CHECK(dot_product(v1, v2) == doctest::Approx(expected).epsilon(1e-10));
    }
}

TEST_CASE("Activation Functions - Edge Cases") {
    SUBCASE("Very large inputs to sigmoid") {
        // Should not overflow or return NaN
        double result1 = sigmoid(1000.0);
        CHECK(!std::isnan(result1));
        CHECK(!std::isinf(result1));
        CHECK(result1 > 0.0);
        CHECK(result1 <= 1.0);
        
        double result2 = sigmoid(-1000.0);
        CHECK(!std::isnan(result2));
        CHECK(!std::isinf(result2));
        CHECK(result2 >= 0.0);
        CHECK(result2 < 1.0);
    }
    
    SUBCASE("Very large inputs to tanh") {
        double result1 = tanh_func(1000.0);
        CHECK(!std::isnan(result1));
        CHECK(!std::isinf(result1));
        CHECK(result1 > 0.0);
        CHECK(result1 <= 1.0);
        
        double result2 = tanh_func(-1000.0);
        CHECK(!std::isnan(result2));
        CHECK(!std::isinf(result2));
        CHECK(result2 >= -1.0);
        CHECK(result2 < 0.0);
    }
}

TEST_CASE("Dot Product - Edge Cases and Error Conditions") {
    SUBCASE("Empty vectors") {
        std::vector<double> v1;
        std::vector<double> v2;
        // Behavior with empty vectors - should return 0
        CHECK(dot_product(v1, v2) == 0.0);
    }
    
    SUBCASE("Very large numbers") {
        std::vector<double> v1 = {1e100, 1e100};
        std::vector<double> v2 = {1e-100, 1e-100};
        // Should handle extreme ranges
        double result = dot_product(v1, v2);
        CHECK(!std::isnan(result));
        CHECK(!std::isinf(result));
    }
}

TEST_CASE("Activation Layer ReLU Function") {
    // Test the ActivationLayerRelu function
    // Note: Current implementation just returns x, might need updating
    SUBCASE("Basic functionality") {
        CHECK(ActivationLayerRelu(5.0) == 5.0);
        CHECK(ActivationLayerRelu(-5.0) == -5.0);
        CHECK(ActivationLayerRelu(0.0) == 0.0);
    }
    
    // This test documents that ActivationLayerRelu currently
    // doesn't apply ReLU activation - it's a pass-through
    // This might be intentional or need fixing
    SUBCASE("Current behavior documentation") {
        // Unlike relu(), ActivationLayerRelu doesn't apply activation
        CHECK(ActivationLayerRelu(-10.0) != relu(-10.0));
    }
}

TEST_CASE("Numerical Stability Tests") {
    SUBCASE("Sigmoid with values near overflow threshold") {
        // exp(-x) could overflow for large x
        double result = sigmoid(710.0);  // Near exp() overflow
        CHECK(!std::isnan(result));
        CHECK(!std::isinf(result));
        CHECK(result > 0.999);
    }
    
    SUBCASE("Dot product with cancellation") {
        // Test for numerical cancellation issues
        std::vector<double> v1 = {1e16, 1.0, -1e16};
        std::vector<double> v2 = {1.0, 1.0, 1.0};
        double result = dot_product(v1, v2);
        // Due to floating point precision, might not be exactly 1.0
        CHECK(result == doctest::Approx(1.0).epsilon(0.1));
    }
}

TEST_CASE("Activation Functions - Special Mathematical Properties") {
    SUBCASE("Sigmoid derivative relationship") {
        // sigmoid'(x) = sigmoid(x) * (1 - sigmoid(x))
        double x = 2.0;
        double sig_x = sigmoid(x);
        // Approximate derivative numerically
        double h = 1e-7;
        double derivative_approx = (sigmoid(x + h) - sigmoid(x - h)) / (2 * h);
        double derivative_formula = sig_x * (1 - sig_x);
        CHECK(derivative_approx == doctest::Approx(derivative_formula).epsilon(1e-5));
    }
    
    SUBCASE("ReLU piecewise nature") {
        // ReLU should have a sharp transition at 0
        CHECK(relu(-0.0001) == 0.0);
        CHECK(relu(0.0001) == 0.0001);
        CHECK(relu(0.0) == 0.0);
    }
}

TEST_CASE("Vector Operations - Dimensionality") {
    SUBCASE("2D vectors") {
        std::vector<double> v1 = {3.0, 4.0};
        std::vector<double> v2 = {1.0, 0.0};
        CHECK(dot_product(v1, v2) == doctest::Approx(3.0).epsilon(1e-6));
    }
    
    SUBCASE("High-dimensional vectors") {
        std::vector<double> v1(1000, 0.5);
        std::vector<double> v2(1000, 2.0);
        CHECK(dot_product(v1, v2) == doctest::Approx(1000.0).epsilon(1e-4));
    }
}
