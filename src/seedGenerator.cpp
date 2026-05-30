#include "seedGenerator.hpp"

#include <random>

std::array<std::array<bool, CELLS_AREA>, CELLS_AREA> seedGenerate() {
  static std::mt19937_64 engine(std::random_device {}());
  static std::bernoulli_distribution dist(0.5);
  std::array<std::array<bool, CELLS_AREA>, CELLS_AREA> rand_seed {};
  for (auto& row : rand_seed)
    for (auto& cell : row) cell = dist(engine);
  return rand_seed;
}