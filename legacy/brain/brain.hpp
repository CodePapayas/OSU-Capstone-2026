#pragma once
#include <torch/torch.h>
#include <vector>

struct AgentBrain : torch::nn::Module {
  torch::nn::Sequential seq;
  std::vector<torch::nn::Linear> linears_;

  AgentBrain(const std::vector<int64_t>& dims,
             const std::vector<std::string>& acts);

  torch::Tensor forward(torch::Tensor x);

  int64_t countWeights() const;
  void loadFromGenome(const std::vector<float>& flat);
};
