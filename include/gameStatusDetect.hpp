#pragma once
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <array>
#include <string>

#include "getInput.hpp"
#include "history.hpp"

using Cell = std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>;

bool kbhit();

std::string gridToString(const Cell& grid);

bool isCyclic(const Cell& grid, History<std::string>& history);

bool isAllDead(const Cell& grid);