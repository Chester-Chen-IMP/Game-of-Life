#include "gameOfLife.hpp"

#include <locale.h>
#include <ncurses.h>
#include <unistd.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <stack>
#include <string>
#include <thread>

#include "gameStatusDetect.hpp"
#include "getInput.hpp"
#include "history.hpp"
#include "pointAsIndex.hpp"
#include "seedSaver.hpp"

using Cell = std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>;

constexpr const int MAX_SPEED_LEVEL     = 10;
constexpr const int MIN_SPEED_LEVEL     = -10;
constexpr const int DEFAULT_SPEED_LEVEL = 0;

int speedLevelToDelay(int level);

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

int speedLevelToDelay(int level) {
  constexpr const auto BASE_DELAY_MS = 500.0;
  constexpr const auto RATIO         = 1.5;

  auto delay = BASE_DELAY_MS * std::pow(RATIO, -static_cast<double>(level));
  if (delay < 10.0) delay = 10.0;
  if (delay > 5000.0) delay = 5000.0;
  return static_cast<int>(std::round(delay));
}

bool nextCellState(const Cell& cells, const Point& center_point) {
  int alive_amounts = 0;
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      if (dx == 0 && dy == 0) continue;
      if (at(cells, Point(center_point.x + dx, center_point.y + dy)))
        ++alive_amounts;
    }
  }
  bool current_alive = at(cells, center_point);
  if (current_alive) return alive_amounts == 2 || alive_amounts == 3;
  return alive_amounts == 3;
}

class GameController;

class LifeGrid {
 public:
  explicit LifeGrid(Cell cells)
    : cells_(std::move(cells)),
      next_cells_(cells_),
      iteration_count_(0),
      stable_(true),
      seed_(gridToString(cells_)) {
    history_.push(seed_);
    undo_stack_.push(cells_);
  }

  const Cell& cells() const noexcept { return cells_; }

  Cell& setCells() noexcept { return cells_; }

  size_t iterationCount() const noexcept { return iteration_count_; }

  bool isStable() const noexcept { return stable_; }

  const std::string& seed() const noexcept { return seed_; }

  void step() {
    next_cells_ = cells_;
    stable_     = true;
    for (int i = 0; i < CELLS_AREA; ++i) {
      for (int j = 0; j < CELLS_AREA; ++j) {
        bool next_state   = nextCellState(cells_, Point(i, j));
        next_cells_[i][j] = next_state;
        if (next_state != cells_[i][j]) stable_ = false;
      }
    }
  }

  void advance() {
    step();
    cells_ = std::move(next_cells_);
    iteration_count_++;
    while (!redo_stack_.empty()) redo_stack_.pop();
    if (cells_ == undo_stack_.top()) return;
    undo_stack_.push(cells_);
  }

  bool undo() {
    if (undo_stack_.size() <= 1) return false;
    redo_stack_.push(cells_);
    undo_stack_.pop();
    cells_ = undo_stack_.top();
    iteration_count_--;
    history_.pop();
    return true;
  }

  bool redo() {
    if (redo_stack_.empty()) return false;
    cells_ = redo_stack_.top();
    undo_stack_.push(cells_);
    history_.push(gridToString(cells_));
    redo_stack_.pop();
    iteration_count_++;
    return true;
  }

  bool isCyclic() { return ::isCyclic(cells_, history_); }

  bool isAllDead() const { return ::isAllDead(cells_); }

 private:
  Cell cells_;
  Cell next_cells_;
  size_t iteration_count_;
  bool stable_;
  std::string seed_;
  History<std::string> history_;
  std::stack<Cell> undo_stack_;
  std::stack<Cell> redo_stack_;
  friend class GameController;
};

class NcursesRenderer {
 public:
  NcursesRenderer() {
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    clear();
    refresh();
  }

  ~NcursesRenderer() {
    nodelay(stdscr, FALSE);
    nocbreak();
    echo();
    curs_set(1);
    endwin();
  }

  void renderStatus(size_t iteration_count,
                    int speed_level,
                    int delay_ms,
                    const char* speed_desc,
                    bool paused) const {
    move(header_row_, 0);
    clrtoeol();
    printw(
      "世代：%zu | 速度等级：%s%d（%s） 延迟：%dms | [+/-] 调节速度 | "
      "[r/R] 重置速度 | [SPACE] %s | [←/→] 后退/前进 | [q/Q] 退出游戏",
      iteration_count,
      speed_level > 0 ? "+" : "",
      speed_level,
      speed_desc,
      delay_ms,
      paused ? "▶" : "⏸"
    );
  }

  void renderGrid(const Cell& cells) const {
    const int offset = columnOffset();
    for (int i = 0; i < MAP_AREA; ++i) {
      move(body_row_ + i, offset);
      for (int j = 0; j < MAP_AREA; ++j) {
        if (i == 0 || i == MAP_AREA - 1 || j == 0 || j == MAP_AREA - 1) {
          addstr("🧱");
        } else {
          int gridX = i - 1;
          int gridY = j - 1;
          addstr(cells[gridX][gridY] ? "⬛" : "⬜");
        }
      }
    }
  }

  void renderMessage(const std::string& message) const {
    clear();
    mvprintw(0, 0, "%s", message.c_str());
    refresh();
  }

  void renderMessage(const char* message) const {
    clear();
    mvprintw(0, 0, "%s", message);
    refresh();
  }

  void clearScreen() const { clear(); }

  void refreshScreen() const { refresh(); }

 private:
  int columnOffset() const {
    int width  = COLS;
    int offset = (width - MAP_AREA * 2) / 2;
    return offset < 0 ? 0 : offset;
  }

  const int header_row_ = 0;
  const int body_row_   = 3;
};

class GameController {
 public:
  GameController()
    : grid_(input()),
      speed_level_(DEFAULT_SPEED_LEVEL),
      current_delay_ms_(speedLevelToDelay(DEFAULT_SPEED_LEVEL)),
      game_state_(true),
      paused_(false),
      user_aborted_(false) {}

  void run() {
    std::thread listener(&GameController::listen, this);

    while (game_state_) {
      renderer_.clearScreen();
      renderer_.renderStatus(grid_.iterationCount(),
                             speed_level_.load(),
                             current_delay_ms_.load(),
                             getSpeedDescription(speed_level_.load()),
                             paused_.load());
      renderer_.renderGrid(grid_.cells());
      renderer_.refreshScreen();

      if (paused_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }

      grid_.advance();
      if (user_aborted_) {
        renderer_.renderMessage("模拟中断。\n种子已保存至seed.txt。\n");
        break;
      }

      if (grid_.isStable()) {
        renderer_.clearScreen();
        if (seedSaver(grid_.seed())) {
          if (grid_.isAllDead())
            renderer_.renderMessage("所有细胞死亡。\n模拟结束。");
          else
            renderer_.renderMessage("检测到状态稳定。\n模拟结束。");
        }
        break;
      }

      if (grid_.isCyclic()) {
        renderer_.renderMessage("检测到循环状态。\n模拟结束。\n");
        break;
      }

      std::this_thread::sleep_for(
        std::chrono::milliseconds(current_delay_ms_.load())
      );
    }

    game_state_ = false;
    listener.join();

    const bool is_seed_saved = seedSaver(grid_.seed(), grid_.iterationCount());
    if (is_seed_saved) {
      renderer_.renderMessage("迭代次数："
                              + std::to_string(grid_.iterationCount())
                              + "。\n种子已保存至seed.txt。\n");
    } else {
      renderer_.renderMessage("未进行任何操作。游戏退出。\n");
    }
  }

 private:
  void listen() {
    while (game_state_) {
      int key = getch();
      if (key == ERR) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }
      handleKey(key);
    }
  }

  void handleKey(int key) {
    switch (key) {
      case 'q':
      case 'Q':
        game_state_   = false;
        user_aborted_ = true;
        break;
      case ' ':
        paused_ = !paused_.load();
        break;
      case '+':
      case '=':
        if (speed_level_ < MAX_SPEED_LEVEL) {
          speed_level_++;
          current_delay_ms_ = speedLevelToDelay(speed_level_.load());
        }
        break;
      case '-':
      case '_':
        if (speed_level_ > MIN_SPEED_LEVEL) {
          speed_level_--;
          current_delay_ms_ = speedLevelToDelay(speed_level_.load());
        }
        break;
      case 'r':
      case 'R':
        speed_level_      = DEFAULT_SPEED_LEVEL;
        current_delay_ms_ = speedLevelToDelay(speed_level_.load());
        break;
      default:
        break;
    }

    if (key == KEY_LEFT) {
      grid_.undo();
      renderer_.renderGrid(grid_.cells_);
      renderer_.renderStatus(grid_.iterationCount(),
                             speed_level_.load(),
                             current_delay_ms_.load(),
                             getSpeedDescription(speed_level_.load()),
                             paused_.load());
      refresh();
    }

    if (key == KEY_RIGHT) {
      grid_.redo();
      renderer_.renderGrid(grid_.cells_);
      renderer_.renderStatus(grid_.iterationCount(),
                             speed_level_.load(),
                             current_delay_ms_.load(),
                             getSpeedDescription(speed_level_.load()),
                             paused_.load());
      refresh();
    }
  }

  LifeGrid grid_;
  NcursesRenderer renderer_;
  std::atomic<int> speed_level_;
  std::atomic<int> current_delay_ms_;
  std::atomic<bool> game_state_;
  std::atomic<bool> paused_;
  std::atomic<bool> user_aborted_;
};

void game() {
  GameController controller;
  controller.run();
}
