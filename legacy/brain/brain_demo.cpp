#include <torch/torch.h>
#include <iostream>
#include "brain.hpp"

// This is a small demonstration to show the feedforward brain architecture is correctly
// ported to C++

int main() {
  torch::manual_seed(21); // For demonstration purposes; Recreatable results
  

  AgentBrain brain({5, 8, 8}, {"tanh", "none"});

  auto x = torch::rand({2, 5});
  auto y = brain.forward(x);

  std::cout << "Input shape : " << x.sizes() << "\n";
  std::cout << "Output shape : " << y.sizes() << "\n";
  std::cout << "Output sample:\n" << y << "\n";
  return 0;
}
