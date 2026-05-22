#pragma once

#include <array>

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
constexpr const int CELLS_AREA = 30;
constexpr const int MAP_AREA = 32;

std::array<std::array<bool, CELLS_AREA>, CELLS_AREA> input();