#include "../include/getInput.hpp"

#include <locale.h>
#include <ncurses.h>
#include <wchar.h>

#include <cstdlib>
#include <iostream>

std::array<std::array<bool, CELLS_AREA>, CELLS_AREA> input() {
  setlocale(LC_ALL, "");
  initscr();
  curs_set(0);
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  ESCDELAY = 0;
  mousemask(BUTTON1_CLICKED, NULL);

  std::array<std::array<bool, CELLS_AREA>, CELLS_AREA> grid{};

  clear();

  int column_offset = (SCREEN_WIDTH - MAP_AREA * 2) / 2;
  int row_offset = 3;
  const int WIDTH_OF_WCHAR = 2;

  for (int i = 0; i < MAP_AREA; ++i) {
    for (int j = 0; j < MAP_AREA; ++j) {
      move(i + row_offset, j * WIDTH_OF_WCHAR + column_offset);
      bool is_border =
          (i == 0 || i == MAP_AREA - 1 || j == 0 || j == MAP_AREA - 1);
      mvaddstr(i + row_offset, j * WIDTH_OF_WCHAR + column_offset,
               is_border ? "🧱" : "⬜");
    }
  }
  refresh();

  int ch = 0;
  while ((ch = getch()) != ' ') {
    if (ch == KEY_MOUSE) {
      MEVENT event;
      if (getmouse(&event) == OK) {
        int phy_row = event.y;
        int phy_col = event.x;
        int logic_row = phy_row - row_offset;
        int logic_col = (phy_col - column_offset) / WIDTH_OF_WCHAR;

        if (logic_row > 0 && logic_row < MAP_AREA - 1 && logic_col > 0 &&
            logic_col < MAP_AREA - 1) {
          int gridX = logic_row - 1;
          int gridY = logic_col - 1;
          grid[gridX][gridY] = !grid[gridX][gridY];
          move(phy_row, phy_col % WIDTH_OF_WCHAR == 1 ? phy_col : phy_col - 1);
          addwstr(grid[gridX][gridY] ? L"⬛" : L"⬜");
          wnoutrefresh(stdscr);
          doupdate();
        }
      }
    }
#if 1
    if (ch == 'q' || ch == 'Q') {
      std::cout << "已退出。";
      endwin();
      exit(0);
    }
#endif
  }
  endwin();
  return grid;
}
