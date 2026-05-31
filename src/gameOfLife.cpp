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
    stable_stack_.push(stable_.load());
  }

  const Cell& cells() const noexcept { return cells_; }

  Cell& setCells() noexcept { return cells_; }

  size_t iterationCount() const noexcept { return iteration_count_; }

  bool isStable() const noexcept { return stable_stack_.top(); }

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
    if (!undo_stack_.empty())
      if (cells_ == undo_stack_.top()) return;
    undo_stack_.push(cells_);
    stable_stack_.push(stable_);
  }

  bool undo() {
    if (undo_stack_.size() <= 1) return false;
    redo_stack_.push(cells_);
    undo_stack_.pop();
    cells_ = undo_stack_.top();
    iteration_count_--;
    stable_stack_.pop();
    history_.pop();
    return true;
  }

  bool redo() {
    if (redo_stack_.empty()) return false;
    cells_ = redo_stack_.top();
    undo_stack_.push(cells_);
    redo_stack_.pop();
    iteration_count_++;
    {
      Cell temp_cells_ = cells_;
      stable_          = true;
      for (int i = 0; i < CELLS_AREA; ++i) {
        for (int j = 0; j < CELLS_AREA; ++j) {
          bool next_state   = nextCellState(temp_cells_, Point(i, j));
          temp_cells_[i][j] = next_state;
          if (next_state != cells_[i][j]) stable_ = false;
        }
      }
    }  // recalculate if stable
    stable_stack_.push(stable_);
    history_.push(gridToString(cells_));
    return true;
  }

  void doNow() { advance(); }

  bool isRedoEmpty() { return redo_stack_.empty(); }

  bool isCyclic() { return ::isCyclic(cells_, history_); }

  bool isAllDead() const { return ::isAllDead(cells_); }

 private:
  Cell cells_;
  Cell next_cells_;
  size_t iteration_count_;
  std::atomic<bool> stable_;
  std::string seed_;
  History<std::string> history_;
  std::stack<Cell> undo_stack_;
  std::stack<Cell> redo_stack_;
  std::stack<bool> stable_stack_;
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
    offset     = offset % 2 == 0 ? offset : offset - 1;
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
    renderFrame();

    std::string final_message;
    auto last_update = std::chrono::steady_clock::now();

    while (game_state_.load()) {
      int key;
      bool had_input = false;
      while ((key = getch()) != ERR) {
        handleKey(key);
        had_input = true;
      }

      if (!game_state_.load()) break;

      if (had_input && !user_aborted_.load()) {
        renderFrame();
        last_update = std::chrono::steady_clock::now();
      }

      if (paused_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }

      auto now = std::chrono::steady_clock::now();
      auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now
                                                              - last_update);
      if (elapsed < std::chrono::milliseconds(current_delay_ms_.load())) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }

      last_update = now;
      grid_.advance();

      renderer_.clearScreen();
      renderer_.renderStatus(grid_.iterationCount(),
                             speed_level_.load(),
                             current_delay_ms_.load(),
                             getSpeedDescription(speed_level_.load()),
                             paused_.load());
      renderer_.renderGrid(grid_.cells());
      renderer_.refreshScreen();

      if (user_aborted_.load()) {
        final_message = "模拟中断。\n种子已保存至seed.txt。\n";
        break;
      }

      if (grid_.isStable()) {
        if (seedSaver(grid_.seed()))
          if (grid_.isAllDead())
            final_message = "所有细胞死亡。\n模拟结束。";
          else
            final_message = "检测到状态稳定。\n模拟结束。";
        else
          final_message = "检测到状态稳定。\n模拟结束。";
        break;
      }

      if (grid_.isCyclic()) {
        final_message = "检测到循环状态。\n模拟结束。\n";
        break;
      }
    }

    if (!final_message.empty()) {
      showEndMessage(final_message);
    } else {
      const bool is_seed_saved =
        seedSaver(grid_.seed(), grid_.iterationCount());
      if (is_seed_saved) {
        showEndMessage("迭代次数："
                       + std::to_string(grid_.iterationCount())
                       + "。\n种子已保存至seed.txt。\n");
      } else {
        showEndMessage("未进行任何操作。游戏退出。\n");
      }
    }
  }

 private:
  void handleKey(int key) {
    switch (key) {
      case 'q':
      case 'Q':
        game_state_.store(false);
        user_aborted_.store(true);
        break;
      case ' ':
        paused_.store(!paused_.load());
        break;
      case '+':
      case '=':
        if (speed_level_.load() < MAX_SPEED_LEVEL) {
          speed_level_.store(speed_level_.load() + 1);
          current_delay_ms_.store(speedLevelToDelay(speed_level_.load()));
        }
        break;
      case '-':
      case '_':
        if (speed_level_.load() > MIN_SPEED_LEVEL) {
          speed_level_.store(speed_level_.load() - 1);
          current_delay_ms_.store(speedLevelToDelay(speed_level_.load()));
        }
        break;
      case 'r':
      case 'R':
        speed_level_.store(DEFAULT_SPEED_LEVEL);
        current_delay_ms_.store(speedLevelToDelay(speed_level_.load()));
        break;
      default:
        break;
    }

    if (key == KEY_LEFT) grid_.undo();

    if (key == KEY_RIGHT) {
      if (!grid_.isRedoEmpty())
        grid_.redo();
      else
        grid_.doNow();
    }
  }

  void showEndMessage(const std::string& message) {
    renderer_.renderMessage(message + "\n按任意键退出...");
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
  }

  void renderFrame() {
    renderer_.clearScreen();
    renderer_.renderStatus(grid_.iterationCount(),
                           speed_level_.load(),
                           current_delay_ms_.load(),
                           getSpeedDescription(speed_level_.load()),
                           paused_.load());
    renderer_.renderGrid(grid_.cells());
    renderer_.refreshScreen();
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
