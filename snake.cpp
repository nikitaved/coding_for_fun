#include <curses.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>
#include <deque>
#include <array>
#include <algorithm>

using Point = std::array<int, 2>;
using Food = Point;
using ScreenSize = Point;
using BitMap = std::vector<char>;

class Snake {
  public:
    using Segment = Point;
    using Body = std::deque<Segment>;
    enum class Direction {Up, Left, Down, Right};

  private:
    Body body;
    Direction direction;
    ScreenSize screenSize;

    Segment& getHeadPosition();
    Segment predictHeadMove() const;
    void turn(Direction where);
    Segment mirrorSegment(Segment segment) const;
    int convert2D2Flat(Point location) const;
    bool isReachable(Point start, Point end) const;
    void move();

  public:
    Snake(Body initBody, ScreenSize screenSize,
        Direction initDirection = Direction::Left);

    const Segment& getHeadPosition() const;
    const Segment& getTailPosition() const;
    const Body& getBody() const;
    Direction getDirection() const;

    bool isInsideBody(Point point) const;
    bool isSelfCollision() const;
    bool isReachable(Point location) const;
    void moveMirror();
    bool grow(Food food);

    void turnLeft();
    void turnRight();
};

/*
 * Snake class implementation
 */

Snake::Segment Snake::predictHeadMove() const {
  Segment head = body.front();
  Segment newHead = head;
  switch (direction) {
    case Direction::Left:
      newHead[1] -= 1; // decrease x
      break;
    case Direction::Right:
      newHead[1] += 1; // increase x
      break;
    case Direction::Up:
      newHead[0] -= 1; // decrease y
      break;
    case Direction::Down:
      newHead[0] += 1; // increase y
      break;
  }
  return newHead;
}

void Snake::turn(Direction where) {
  auto isLeft = (where == Direction::Left);
  switch (direction) {
    case Direction::Left:
      direction = isLeft ? Direction::Down : Direction::Up;
      break;
    case Direction::Right:
      direction = isLeft ? Direction::Up : Direction::Down;
      break;
    case Direction::Up:
      direction = isLeft ? Direction::Left : Direction::Right;
      break;
    case Direction::Down:
      direction = isLeft ? Direction::Right : Direction::Left;
      break;
  }
}

Snake::Snake(Snake::Body initBody,
    ScreenSize initScreenSize,
    Snake::Direction initDirection)
  : body{initBody}, screenSize{initScreenSize}, direction{initDirection}
{}

Snake::Segment& Snake::getHeadPosition() {
  return body.front();
}

const Snake::Segment& Snake::getHeadPosition() const {
  return body.front();
}

const Snake::Segment& Snake::getTailPosition() const {
  return body.back();
}

const Snake::Body& Snake::getBody() const {
  return body;
}

Snake::Direction Snake::getDirection() const {
  return direction;
}

bool Snake::isInsideBody(Point point) const {
  for (auto& segment : body) {
    if (point == segment)
      return true;
  }
  return false;
}

bool Snake::isSelfCollision() const {
  Segment newHead = predictHeadMove();
  return isInsideBody(newHead);
}

bool Snake::isReachable(Point start, Point end) const {
  BitMap isSeen(screenSize[0] * screenSize[1], false);
  for (auto& segment : body) {
    if (segment == end)
      return false;
    isSeen[convert2D2Flat(segment)] = true;
  }

  std::deque<Point> bfs;
  bfs.push_front(start);
  isSeen[convert2D2Flat(start)] = true;
  while (bfs.size()) {
    auto currLocation = bfs.front();
    bfs.pop_front();
    if (currLocation == end) {
      return true;
    }

    // check neighours
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dx = -1; dx <= 1; ++dx) {
        if (std::abs(dy) == std::abs(dx))
          continue;
        Point neighbour{currLocation[0] + dy, currLocation[1] + dx};
        neighbour = mirrorSegment(neighbour);
        if (!isSeen[convert2D2Flat(neighbour)]) {
          bfs.push_back(neighbour);
          isSeen[convert2D2Flat(neighbour)] = true;
        }
      }
    }
  }

  return false;
}

bool Snake::isReachable(Point location) const {
  return isReachable(predictHeadMove(), location);
}
void Snake::move() {
  body.pop_back(); // remove tail
  Segment newHead = predictHeadMove();
  body.push_front(newHead);
}

Snake::Segment Snake::mirrorSegment(Segment segment) const {
  for (int coord = 0; coord < 2; ++coord) {
    if (segment[coord] < 0)
      segment[coord] = screenSize[coord] - 1;
    else if (segment[coord] >= screenSize[coord])
      segment[coord] = 0;
  }
  return segment;
}

int Snake::convert2D2Flat(Point location) const {
  return screenSize[1] * location[0] + location[1];
}

void Snake::moveMirror() {
  move();
  Segment& headPosition = getHeadPosition();
  headPosition = mirrorSegment(headPosition);
}

bool Snake::grow(Food food) {
  if (predictHeadMove() == food) {
    body.push_front({food[0], food[1]});
    return true;
  }
  return false;
}

void Snake::turnLeft() {
  turn(Direction::Left);
}

void Snake::turnRight() {
  turn(Direction::Right);
}

enum class Input : char {
  KeyLeft = 'j',
  KeyRight = 'k',
  KeyQuit = 'q',
  KeyYes = 'y',
  KeyNo = 'n'
};

enum class Graphics : char {
  SnakeSegment = 'O',
  Food = 'F',
  Blank = ' ',
  HorizontalBar = '-',
  VerticalBal =  '|',
  Corner = '*'
};


int main(int argc, char const* argv[]) {
  /*
   * Init ncurses
   */
  initscr();
  cbreak();
  noecho();
  curs_set(0);

  int maxLines = LINES - 1;
  int maxCols = COLS - 1;


  /*
   * Main Loop
   */
  bool quitGame = false;
  while (!quitGame) {
    nodelay(stdscr, TRUE); // make input non-blocking
    bool gameOver = false;
    /*
     * Init game configuration
     */
    clear();
    ScreenSize screenSize{10, 50};
    Point center{screenSize[0] / 2, screenSize[1] / 2};

    // draw border
    for (int i = 0; i < screenSize[1]; ++i) {
      mvaddch(screenSize[0], i, static_cast<char>(Graphics::HorizontalBar));
    }
    for (int i = 0; i < screenSize[0]; ++i) {
      mvaddch(i, screenSize[1], static_cast<char>(Graphics::VerticalBal));
    }
    mvaddch(screenSize[0], screenSize[1], static_cast<char>(Graphics::Corner));
    refresh();

    Snake::Body snakeBody;
    int snakeLen = 5;
    for (int seg{}; seg < snakeLen; ++seg) {
      snakeBody.push_back({center[0], center[1] + seg});
    }
    Snake::Direction direction = Snake::Direction::Left;
    Snake snake{snakeBody, screenSize, direction};

    Food food{center[0], center[1] - 10};
    int score = 0;

    while (!gameOver) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      // drawing
      for (auto& segment : snake.getBody()) {
        mvaddch(segment[0], segment[1], static_cast<char>(Graphics::SnakeSegment));
      }
      mvaddch(food[0], food[1], static_cast<char>(Graphics::Food));
      mvaddstr(maxLines, 0, (std::string{"Score : "} + std::to_string(score)).data());
      refresh();

      // input
      switch (static_cast<Input>(getch())) {
        case Input::KeyLeft:
          snake.turnLeft();
          break;
        case Input::KeyRight:
          snake.turnRight();
          break;
        case Input::KeyQuit:
          quitGame = true;
          break;
      }
      if (quitGame) break;

      if (!snake.isSelfCollision()) {
        if (!snake.grow(food)) {
          auto tail = snake.getTailPosition();
          mvaddch(tail[0], tail[1], static_cast<char>(Graphics::Blank));
          snake.moveMirror();
        }
        else {
          ++score;
          while (!snake.isReachable(food))
            food = {rand() % screenSize[0], rand() % screenSize[1]};
        }
      }
      else {
        gameOver = true;
      }
    } // while (!gameOver)
    
    /*
     * End
     */
    if (quitGame) break;
    nodelay(stdscr, FALSE); // make input blocking
    mvaddstr(maxLines - 1, 0,
      "Want to play once more? "
      "Press 'y' to start a new game, or 'n' to quit.");
    refresh();
    bool receivedYesOrNo = false;
    while (!receivedYesOrNo) {
      switch (static_cast<Input>(getch())) {
        case Input::KeyYes:
          receivedYesOrNo = true;
          break;
        case Input::KeyNo:
          receivedYesOrNo = true;
          quitGame = true;
          break;
      }
    }
  } // while (!quitGame)

  endwin();

  return 0;
}
