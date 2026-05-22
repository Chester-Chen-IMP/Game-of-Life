#pragma once
#include <array>

#include "getInput.hpp"
#include "pointAsIndex.hpp"

void game();
void forAround(std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>&, bool&,
               const Point&);