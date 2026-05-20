#include "gameOfLife.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>

#include "gameStatusDetect.hpp"
#include "getInput.hpp"
#include "pointAsIndex.hpp"

void forAround(std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>& cells,
               bool& cell, const Point& center_point) {
  static std::array<Point, 8> points_around{};
  int alive_amounts = 0;
  points_around = {Point(center_point.x - 1, center_point.y + 1),
                   Point(center_point.x - 1, center_point.y),
                   Point(center_point.x - 1, center_point.y - 1),
                   Point(center_point.x, center_point.y + 1),
                   Point(center_point.x, center_point.y - 1),
                   Point(center_point.x + 1, center_point.y + 1),
                   Point(center_point.x + 1, center_point.y),
                   Point(center_point.x + 1, center_point.y - 1)};
  for (auto each : points_around) {
    if (at(cells, each) == 1) {
      alive_amounts++;
    }
  }
  if (cell == 0) {
    if (alive_amounts == 3) {
      cell = 1;
    }
  }
  if (cell == 1) {
    if (alive_amounts < 2 || alive_amounts > 3) {
      cell = 0;
    }
  }
}

void game() {
  static bool game_status = true;
  static size_t counts = 0;
  static std::chrono::milliseconds delta_time(500);
  std::array<std::array<bool, CELLS_AREA>, CELLS_AREA> cells = input();
  std::cout << "\033[?25l" << std::flush;
  std::array<Point, 8> points_around{};
  while (game_status == true) {
    if (kbhit()) {
      char key = getchar();
      switch (key) {
        case 'q':
        case 'Q':
          game_status = false;
          break;
        case '+':
          delta_time += std::chrono::milliseconds(50);
          break;
        case '-':
          if (delta_time - std::chrono::milliseconds(50) >
              std::chrono::milliseconds(0))
            delta_time -= std::chrono::milliseconds(50);
          break;
        case 'r':
        case 'R':
          delta_time = std::chrono::milliseconds(500);
      }
    }
    std::cout << "\033[2J\033[H";
    auto cells_at_start = cells;
    auto cells_next = cells;
    for (int i = 0; i < MAP_AREA; ++i) {
      for (int j = 0; j < MAP_AREA; ++j) {
        if (i == 0 || i == MAP_AREA - 1) {
          std::cout << "🧱";
        }  // 顶部和底部的边框
        if ((j == 0 || j == MAP_AREA - 1) && !(i == 0 || i == MAP_AREA - 1)) {
          std::cout << "🧱";
        }  // 除去顶部和底部后的边框
        if (i < MAP_AREA - 1 && j < MAP_AREA - 1 && i > 0 && j > 0) {
          forAround(cells_at_start, cells_next[i - 1][j - 1],
                    Point(i - 1, j - 1));
          if (cells_next[i - 1][j - 1] == 1) {
            std::cout << "⬛";
          }
          if (cells_next[i - 1][j - 1] == 0) {
            std::cout << "⬜";
          }
        }
      }
      std::cout << '\n';
    }
    cells = cells_next;
    if (cells_at_start == cells) {
      break;
    } else {
      counts++;
      std::this_thread::sleep_for(std::chrono::milliseconds(delta_time));
      continue;
    }
  }
  std::cout << "\n迭代次数：" << counts << '\n';
  std::cout << "\033[?25h";
}