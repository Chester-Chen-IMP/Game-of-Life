#include "gameStatusDetect.hpp"

#include <poll.h>
#include <unistd.h>

#include <array>
#include <string>

#include "getInput.hpp"
#include "history.hpp"

using Cell = std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>;

bool kbhit() {
  struct pollfd pfd;
  pfd.fd      = STDIN_FILENO;
  pfd.events  = POLLIN;
  pfd.revents = 0;
  int result  = poll(&pfd, 1, 0);
  return result > 0 && (pfd.revents & POLLIN);
}

std::string gridToString(const Cell& grid) {
  std::string result;
  result.reserve(CELLS_AREA * CELLS_AREA);
  for (int i = 0; i < CELLS_AREA; ++i)
    for (int j = 0; j < CELLS_AREA; ++j)
      result.push_back(grid[i][j] ? '1' : '0');
  return result;
}

bool isCyclic(const Cell& grid, History<std::string>& history) {
  std::string currentState = gridToString(grid);
  if (history.contains(currentState)) return true;
  history.push(std::move(currentState));
  return false;
}

bool isAllDead(const Cell& grid) {
  for (int i = 0; i < CELLS_AREA; ++i) {
    for (int j = 0; j < CELLS_AREA; ++j)
      if (grid[i][j] == 1) return false;
  }
  return true;
}