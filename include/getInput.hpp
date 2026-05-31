#pragma once
#include <array>
#include <variant>

constexpr const int CELLS_AREA = 30;
constexpr const int MAP_AREA   = 32;
using Cell = std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>;
std::variant<Cell, bool> input();