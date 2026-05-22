#include "gameStatusDetect.hpp"

#include <array>
#include <cstdio>
#include <sstream>
#include <string>

#include "getInput.hpp"

bool kbhit() {
  struct timeval tv = {0, 0};
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

char getch() {
  char ch = getchar();
  return ch;
}

std::string gridToString(
    const std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>& grid) {
  std::stringstream ss;
  for (int i = 0; i < CELLS_AREA; ++i) {
    for (int j = 0; j < CELLS_AREA; ++j) {
      ss << (grid[i][j] == 1 ? "1" : "0");
    }
  }
  return ss.str();
}

bool isCyclic(const std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>& grid,
              std::unordered_set<std::string>& history) {
  std::string currentState = gridToString(grid);

  if (history.find(currentState) != history.end()) {
    return true;
  }

  history.insert(currentState);
  return false;
}

bool isAllDead(
    const std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>& grid) {
  for (int i = 0; i < CELLS_AREA; ++i) {
    for (int j = 0; j < CELLS_AREA; ++j) {
      if (grid[i][j] == 1) {
        return false;
      }
    }
  }
  return true;
}