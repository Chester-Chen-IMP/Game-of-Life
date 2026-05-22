#pragma once
#include <array>

#include "getInput.hpp"
#include "pointAsIndex.hpp"

static bool game_state = true;
void game();
void forAround(std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>&, bool&,
               const Point&);