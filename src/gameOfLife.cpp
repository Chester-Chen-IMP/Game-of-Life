#include "../include/gameOfLife.hpp"

#include <termios.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_set>

#include "../include/gameStatusDetect.hpp"
#include "../include/getInput.hpp"
#include "../include/pointAsIndex.hpp"
#include "../include/seedSaver.hpp"

const int SCREEN_WIDTH = 179;
const int SCREEN_HEIGHT = 48;

constexpr const int MAX_SPEED_LEVEL = 10;
constexpr const int MIN_SPEED_LEVEL = -10;
constexpr const int DEFAULT_SPEED_LEVEL = 0;
static int speed_level = DEFAULT_SPEED_LEVEL;
int speedLevelToDelay(int level);
static int current_delay_ms = speedLevelToDelay(speed_level);

bool game_state = true;

// 速度的对数映射
// 公式：delay = base * (ratio ^ (-level))
// level 越大，delay 越小
int speedLevelToDelay(int level) {
  constexpr const auto BASE_DELAY_MS = 500.0;  // level 为0时对应的延迟
  constexpr const auto RATIO = 1.5;            // 每变化一级，速度变化倍数

  auto delay = BASE_DELAY_MS * std::pow(RATIO, -static_cast<double>(level));

  if (delay < 10.0) delay = 10.0;
  if (delay > 5000.0) delay = 5000.0;
  return static_cast<int>(std::round(delay));
}

const char* getSpeedDescription(int level) {
  if (level <= -8) return "EXTREMELY SLOW";
  if (level <= -5) return "VERY SLOW";
  if (level <= -2) return "SLOW";
  if (level <= 0) return "NORMAL";
  if (level <= 2) return "FAST";
  if (level <= 5) return "VERY FAST";
  if (level <= 8) return "EXTREMELY FAST";
  return "TELEPORTATION";
}

struct TerminalInit {
  struct termios tty;
  struct termios old_tty;

  TerminalInit() {
    std::cout << "\033[?25l";
    std::cout << std::flush;
    tcgetattr(STDIN_FILENO, &tty);
    old_tty = tty;

    tty.c_lflag &= ~(ICANON | ECHO);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
  }

  ~TerminalInit() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tty);
    std::cout << "\033[?25h" << std::flush;
  }
};

bool user_aborted = false;

void gameState(bool& state) {
  if (kbhit()) {
    char key = getchar();
    switch (key) {
      case 'q':
      case 'Q':
        state = false;
        user_aborted = true;
        break;
      case '+':
      case '=':
        if (speed_level < MAX_SPEED_LEVEL) {
          speed_level++;
          current_delay_ms = speedLevelToDelay(speed_level);
        }
        break;
      case '-':
      case '_':
        if (speed_level > MIN_SPEED_LEVEL) {
          speed_level--;
          current_delay_ms = speedLevelToDelay(speed_level);
        }
        break;
      case 'r':
      case 'R':
        speed_level = DEFAULT_SPEED_LEVEL;
        current_delay_ms = speedLevelToDelay(speed_level);
        break;
    }
  }
}

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
  user_aborted = false;
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
  size_t iteration_counts = 0;
  std::array<Point, 8> points_around{};
  std::unordered_set<std::string> history{};

  auto cells = input();

  TerminalInit term;
  auto seed = gridToString(cells);
  auto initial_state = gridToString(cells);
  history.insert(initial_state);

  auto is_seed_saved = seedSaver(seed);

  while (game_state == true) {
    gameState(game_state);
    std::cout << "\033[2J\033[H" << std::flush;
    std::cout << "世代：" << iteration_counts << " | 速度等级："
              << (speed_level > 0 ? "+" : "") << speed_level << "（"
              << getSpeedDescription(speed_level) << "）"
              << "延迟：" << current_delay_ms << "ms"
              << " | [+/-] 调节速度 | [r/R] 重置速度 | [q/Q] 退出游戏\n\n\n"
              << std::flush;

    auto cells_at_start = cells;
    auto cells_next = cells;
    int column_offset = (SCREEN_WIDTH - MAP_AREA * 2) / 2;
    column_offset -= column_offset % 2;

    for (int i = 0; i < MAP_AREA; ++i) {
      for (int space_counts = 0; space_counts < column_offset; ++space_counts) {
        std::cout << ' ';
      }
      for (int j = 0; j < MAP_AREA; ++j) {
        if (i == 0 || i == MAP_AREA - 1) {
          std::cout << "🧱";
        } else if (j == 0 || j == MAP_AREA - 1) {
          std::cout << "🧱";
        }
        if (i < MAP_AREA - 1 && j < MAP_AREA - 1 && i > 0 && j > 0) {
          int gridX = i - 1;
          int gridY = j - 1;
          forAround(cells_at_start, cells_next[gridX][gridY],
                    Point(gridX, gridY));
          std::cout << (cells[gridX][gridY] == 1 ? "⬛" : "⬜");
        }
      }
      std::cout << '\n';
    }
    if (user_aborted) {
      std::cout << "\n模拟中断。\n种子已保存至seed.txt。\n";
      return;
    }
    cells = cells_next;
    iteration_counts++;
    if (cells_at_start == cells) {
      if (is_seed_saved) {  // 实际是检测用户有没有输入种子
        if (isAllDead(cells)) {
          std::cout << "\033[2J\033[H";
          std::cout << "\n所有细胞死亡。\n模拟结束。";
          break;
        }
        std::cout << "\033[2J\033[H";
        std::cout << "\n检测到状态稳定。\n模拟结束。";
        break;
      }  // 没有就跳过这一步
      break;
    } else if (isCyclic(cells, history)) {
      std::cout << "\033[2J\033[H";
      std::cout << "\n检测到循环状态。\n模拟结束。";
      break;
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(current_delay_ms));
      std::cout << "\033[2J\033[H";
      continue;
    }
  }

  if (is_seed_saved) {  // 和上面逻辑差不多
    std::cout << "\n迭代次数：" << iteration_counts
              << "。\n种子已保存至seed.txt。\n";
  } else {
    std::cout << "\n未进行任何操作。游戏退出。\n";
  }
}