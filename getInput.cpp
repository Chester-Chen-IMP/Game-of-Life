#include "getInput.hpp"

#include <locale.h>
#include <ncurses.h>
#include <wchar.h>

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

  for (int i = 0; i < MAP_AREA; ++i) {
    for (int j = 0; j < MAP_AREA; ++j) {
      move(i, j * 2);
      bool isBorder =
          (i == 0 || i == MAP_AREA - 1 || j == 0 || j == MAP_AREA - 1);
      mvaddstr(i, j * 2, isBorder ? "🧱" : "⬜");
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
        int logic_row = phy_row;
        int logic_col = phy_col / 2;

        if (logic_row > 0 && logic_row < MAP_AREA - 1 && logic_col > 0 &&
            logic_col < MAP_AREA - 1) {
          int gridX = logic_row - 1;
          int gridY = logic_col - 1;
          grid[gridX][gridY] = !grid[gridX][gridY];
          move(phy_row, phy_col % 2 == 0 ? phy_col : phy_col - 1);
          addwstr(grid[gridX][gridY] ? L"⬛" : L"⬜");
          wnoutrefresh(stdscr);
          doupdate();
        }
      }
    }
  }
  endwin();
  return grid;
}
