#pragma once
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <array>
#include <string>
#include <unordered_set>

#include "getInput.hpp"

bool kbhit();
char getch();

std::string gridToString(
    const std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>& grid);

bool isCyclic(const std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>& grid,
              std::unordered_set<std::string>& history);

bool isAllDead(
    const std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>& grid);