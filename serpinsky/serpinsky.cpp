#include <curses.h>
#include <iostream>
#include <algorithm>
#include <array>
#include <cmath>
#include <thread>
#include <chrono>

struct Point {
  int y{};
  int x{};

  Point(int yInit, int xInit) : y{yInit}, x{xInit} {}

  Point() = default;

  Point operator+(const Point& other) const {
    return Point(y + other.y, x + other.x);
  }

  Point operator/(int div) const {
    return Point(y / div, x / div);
  }

  bool operator==(const Point& other) const {
    return (y == other.y) && (x == other.x);
  }

};

const size_t ARR_LEN = 3;
using Coordinates = std::array<Point, ARR_LEN>;

void drawSerpinsky(Coordinates tr, int maxIter=10, int currIter=1) {
  if (currIter > maxIter) return;

  //std::this_thread::sleep_for(std::chrono::milliseconds(10));
  //refresh();

  mvaddch(tr[0].y, tr[0].x, '#');
  mvaddch(tr[1].y, tr[1].x, '#');
  mvaddch(tr[2].y, tr[2].x, '#');

  Point p0 = (tr[0] + tr[1]) / 2;
  Point p1 = (tr[2] + tr[1]) / 2;
  Point p2 = (tr[2] + tr[0]) / 2;

  drawSerpinsky({tr[0], p0, p2}, maxIter, currIter + 1);
  drawSerpinsky({p0, tr[1], p1}, maxIter, currIter + 1);
  drawSerpinsky({p2, p1, tr[2]}, maxIter, currIter + 1);
}

int main(int argc, char const* argv[]) {
  /*
   * Init ncurses
   */
  initscr();
  cbreak();
  noecho();

  clear();

  int maxLines = LINES - 1;
  int maxCols  = COLS - 1;

  /*
   * Draw Serpinsky triangles
   */
  Coordinates tr{Point(0, 0), Point(0, maxCols), Point(maxLines, maxCols / 2)};
  drawSerpinsky(tr, std::max(
      static_cast<int>(std::log2(LINES)) + 1,
      static_cast<int>(std::log2(COLS)) + 1
    )
  );


  /*
   * End
   */
  mvaddstr(maxLines, 0, "Press any key to quit");
  refresh();
  getch();
  endwin();

  return 0;
}
