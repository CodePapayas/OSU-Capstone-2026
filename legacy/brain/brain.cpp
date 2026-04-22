#include "brain.hpp"
#include <stdexcept>
#include <cstdint>
#include <iostream>

namespace {

// Lesson learned: These need to be called in seq->push_back() and need the Functional wrapper as well
void append_activation(torch::nn::Sequential& seq, const std::string& name) {
  if (name == "relu") {
    seq->push_back(torch::nn::Functional(torch::relu));
    return;
  }
  if (name == "tanh") {
    seq->push_back(torch::nn::Functional(torch::tanh));
    return;
  }
  if (name == "sigmoid") {
    seq->push_back(torch::nn::Functional(torch::sigmoid));
    return;
  }
  if (name == "none") {
    return;
  }
  throw std::invalid_argument("Unknown activation: " + name);
}

}

AgentBrain::AgentBrain(const std::vector<int64_t>& dims,
                       const std::vector<std::string>& acts) {
  if (dims.size() < 2) throw std::invalid_argument("need at least two dims");
  if (acts.size() != dims.size()-1) throw std::invalid_argument("acts size mismatch");

  register_module("seq", seq);

  for (size_t i = 0; i + 1 < dims.size(); ++i) {
    auto lin = torch::nn::Linear(dims[i], dims[i+1]);
    linears_.push_back(lin);
    seq->push_back(lin);
    append_activation(seq, acts[i]);
  }
}

torch::Tensor AgentBrain::forward(torch::Tensor x) {
  return seq->forward(x);
}

// Parity Helpers

// This will be utilized when we build an actual agents brain
int64_t AgentBrain::countWeights() const {
  int64_t total = 0;
  for (const auto& lin : linears_) {
    total += lin->weight.numel();
    if (lin->options.with_bias()) 
      total += lin->bias.numel();
  }
  return total;
}

void AgentBrain::loadFromGenome(const std::vector<float>&) {
  // TODO -  implement biochemical genome loading
  std::cerr << "[AgentBrain] loadFromGenome() called but not yet defined.\n";
}
