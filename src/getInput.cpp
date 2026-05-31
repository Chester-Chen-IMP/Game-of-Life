#include "getInput.hpp"

#include <locale.h>
#include <ncurses.h>
#include <unistd.h>
#include <wchar.h>

#include <array>
#include <string>
#include <variant>

#include "gameStatusDetect.hpp"
#include "getWidth.hpp"
#include "seedGenerator.hpp"

namespace {

  void drawGrid(
    int row_offset,
    int column_offset,
    const std::array<std::array<bool, CELLS_AREA>, CELLS_AREA>& grid
  ) {
    constexpr int WIDTH_OF_WCHAR = 2;
    for (int i = 0; i < MAP_AREA; ++i) {
      for (int j = 0; j < MAP_AREA; ++j) {
        bool is_border =
          (i == 0 || i == MAP_AREA - 1 || j == 0 || j == MAP_AREA - 1);
        if (is_border) {
          mvaddwstr(i + row_offset, j * WIDTH_OF_WCHAR + column_offset, L"🧱");
        } else {
          int gridX = i - 1;
          int gridY = j - 1;
          mvaddwstr(i + row_offset,
                    j * WIDTH_OF_WCHAR + column_offset,
                    grid[gridX][gridY] ? L"⬛" : L"⬜");
        }
      }
    }
  }

}  // namespace

std::variant<Cell, bool> input() {
  setlocale(LC_ALL, "");
  initscr();
  curs_set(0);
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  ESCDELAY = 0;
  mousemask(BUTTON1_CLICKED, NULL);

  std::array<std::array<bool, CELLS_AREA>, CELLS_AREA> grid {};
  bool exit_by_user = false;

  clear();
  int column_offset         = (COLS - MAP_AREA * 2) / 2;
  column_offset            -= column_offset % 2;
  int row_offset            = 3;
  const int WIDTH_OF_WCHAR  = 2;

  std::string seed_generate_text {"[🎲随机种子🎲]"};
  int text_display_width = getWidth(seed_generate_text);

  int text_row = LINES - 2;  // -2 是因为我的 kitty 终端设置了标签栏两行高
  int text_col = (COLS - text_display_width) / 2;

  drawGrid(row_offset, column_offset, grid);
  mvaddstr(text_row, text_col, seed_generate_text.c_str());
  refresh();

  bool done = false;
  while (!done) {
    int ch = getch();
    if (ch == ERR) {
      usleep(10000);
      continue;
    }

    if (ch == KEY_MOUSE) {
      MEVENT event;
      if (getmouse(&event) == OK) {
        int phy_row   = event.y;
        int phy_col   = event.x;
        int logic_row = phy_row - row_offset;
        int logic_col = (phy_col - column_offset) / WIDTH_OF_WCHAR;

        if (logic_row > 0
            && logic_row < MAP_AREA - 1
            && logic_col > 0
            && logic_col < MAP_AREA - 1) {
          int gridX          = logic_row - 1;
          int gridY          = logic_col - 1;
          grid[gridX][gridY] = !grid[gridX][gridY];
          move(phy_row, phy_col - (phy_col % WIDTH_OF_WCHAR));
          addwstr(grid[gridX][gridY] ? L"⬛" : L"⬜");
          refresh();
        } else if (phy_row == text_row
                   && phy_col >= text_col
                   && phy_col < text_col + text_display_width) {
          grid = seedGenerate();
          drawGrid(row_offset, column_offset, grid);
          mvaddstr(text_row, text_col, seed_generate_text.c_str());
          refresh();
        }
      }
      continue;
    }

    switch (ch) {
      case 'q':
      case 'Q':
        exit_by_user = true;
        endwin();
        return exit_by_user;
      case 'g':
      case 'G':
        grid = seedGenerate();
        drawGrid(row_offset, column_offset, grid);
        mvaddstr(text_row, text_col, seed_generate_text.c_str());
        refresh();
        break;
      case ' ':  // 开始游戏
        done = true;
        break;
      default:
        break;
    }
  }

  endwin();
  return grid;
}