#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>
#include <chrono>
#include <sstream>

#define NOMINMAX
#include <windows.h>

#include <common/vector.hpp>
#include <common/field.hpp>

const std::regex inputRegex("^p=([0-9]+),([0-9]+) v=(-?[0-9]+),(-?[0-9]+)$");

const Vector FIELD_SIZE(101, 103);

struct Robot {
  Vector pos, velocity;

  void step() {
    pos = (pos + velocity).pMod(FIELD_SIZE);
  }

  Vector quadrantVec() const {
    auto fieldCenter = FIELD_SIZE / 2;
    return pos.compare(fieldCenter);
  }

  void drawPos(HDC dc) {
    SetPixel(dc, pos.x, pos.y, RGB(0, 200, 0));
  }
};


struct Robots : public std::vector<Robot> {
  Robots(std::istream&& input) {
    for (std::string line; std::getline(input, line);) {
      std::smatch match;
      std::regex_match(line, match, inputRegex);
      Robot robot;
      robot.pos.x = std::stoi(match[1].str());
      robot.pos.y = std::stoi(match[2].str());
      robot.velocity.x = std::stoi(match[3].str());
      robot.velocity.y = std::stoi(match[4].str());
      push_back(robot);
    }
  }


  void step() {
    for (auto& robot : *this) {
      robot.step();
    }
  }

  int countAtPos(const Vector& pos) const {
    int number = 0;
    for (auto& robot : *this) {
      if (pos == robot.pos) {
        ++number;
      }
    }
    return number;
  }

  // return true if this could be a tree
  bool draw(HDC dc) {
    // Field used to track the positions of all bots and check for possible xmas trees
    static Field field(FIELD_SIZE.x, FIELD_SIZE.y, ' ');
    std::fill(field.data.begin(), field.data.end(), ' '); // reset field

    for (auto& robot : *this) {
      robot.drawPos(dc);
      field[robot.pos] = 'x';
    }
    
    // Now find a continuous line of at least 10 bots
    static std::string xxx(10, 'x');
    for (int row = 0; row < field.size.y; ++row) {
      if (!std::ranges::search(field.row(row), xxx).empty()) {
        // We found a line of at least 10 bots... could be a tree
        return true;
      }
    }
    
    return false;
  }
};





int main() {
  auto t1 = std::chrono::high_resolution_clock::now();
  Robots robots(std::ifstream("input.txt"));

  auto robotsCopy = robots;

  // Part 1:
  for (int i = 0; i < 100; ++i) {
    robots.step();
  }

  std::unordered_map<Vector, int> quadrants;
  for (auto& robot : robots) {
    auto quadrantVec = robot.quadrantVec();
    if (quadrantVec.x != 0 && quadrantVec.y != 0) { // not in the center
      ++quadrants[quadrantVec];
    }
  }

  auto sum = std::ranges::fold_left(quadrants, 1, [](int result, auto& entry) { return result * entry.second; });

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Result 1: " << sum << "\n"; // 208437768
  // Part 2: 7492 ... We could just search directly for the tree, but the animation is also nice to look at
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
  system("PAUSE");
  system("CLS");
  
  auto consoleHwnd = GetConsoleWindow();
  auto consoleDc = GetDC(consoleHwnd);

  HDC memDc = CreateCompatibleDC(consoleDc);
  HBITMAP memBmp = CreateCompatibleBitmap(consoleDc, FIELD_SIZE.x, FIELD_SIZE.y);
  SelectObject(memDc, memBmp);
  HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));
  RECT rcField = { 0 };
  rcField.right = FIELD_SIZE.x;
  rcField.bottom = FIELD_SIZE.y;

  
  robots = robotsCopy;

  const int SLEEP_DURATION = 0;

  for (int step = 0; true; ++step) {
    RECT rcText{ 10, 30, 100, 50};
    std::stringstream ss;
    ss << "Step " << step;
    DrawTextA(consoleDc, ss.str().c_str(), -1, &rcText, DT_LEFT | DT_TOP);

    // Clear image
    FillRect(memDc, &rcField, black);

    // Fill robots
    bool couldBeTree = robots.draw(memDc);

    // Copy to screen
    StretchBlt(consoleDc, 10, 50, 400, 400, memDc, 0, 0, FIELD_SIZE.x, FIELD_SIZE.y, SRCCOPY);

    if (couldBeTree) {
      if (MessageBoxA(consoleHwnd, "Is this the tree?", "Info", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        return 0;
      }
    }


    Sleep(SLEEP_DURATION);

    robots.step();
  }
}