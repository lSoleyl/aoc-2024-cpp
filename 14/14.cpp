#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>
#include <chrono>
#include <sstream>
#include <windows.h>

#include <common/vector.hpp>


const std::regex inputRegex("^p=([0-9]+),([0-9]+) v=(-?[0-9]+),(-?[0-9]+)$");

const Vector FIELD_SIZE(101, 103);

const Vector tlQuadrant(10, 10);
const Vector trQuadrant(FIELD_SIZE.column - 10, 10);

struct Robot {
  Vector pos, velocity;

  void step() {
    pos = (pos + velocity).pMod(FIELD_SIZE);
  }

  Vector quadrantVec() const {
    auto fieldCenter = FIELD_SIZE / 2;
    return pos.compare(fieldCenter);
  }

  // Returns true if the pixel doesn't violate the possibility of being a christmas tree
  bool drawPos(HDC dc) {
    SetPixel(dc, pos.column, pos.row, RGB(0, 200, 0));
    // not in topleft,topright miniquadrants (they should be empty for a tree)
    return pos.compare(tlQuadrant) != Vector(-1, -1) && pos.compare(trQuadrant) != Vector(1, -1);
  }
};


struct Robots : public std::vector<Robot> {
  Robots(std::istream&& input) {
    for (std::string line; std::getline(input, line);) {
      std::smatch match;
      std::regex_match(line, match, inputRegex);
      Robot robot;
      robot.pos.column = std::stoi(match[1].str());
      robot.pos.row = std::stoi(match[2].str());
      robot.velocity.column = std::stoi(match[3].str());
      robot.velocity.row = std::stoi(match[4].str());
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
    bool couldBeTree = true;
    for (auto& robot : *this) {
      couldBeTree &= robot.drawPos(dc);
    }
    return couldBeTree;
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
    if (quadrantVec.column != 0 && quadrantVec.row != 0) { // not in the center
      ++quadrants[quadrantVec];
    }
  }

  auto sum = std::ranges::fold_left(quadrants, 1, [](int result, auto& entry) { return result * entry.second; });

  system("PAUSE");
  
  auto consoleHwnd = GetConsoleWindow();
  auto consoleDc = GetDC(consoleHwnd);

  HDC memDc = CreateCompatibleDC(consoleDc);
  HBITMAP memBmp = CreateCompatibleBitmap(consoleDc, FIELD_SIZE.column, FIELD_SIZE.row);
  SelectObject(memDc, memBmp);
  HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));
  HBRUSH red = CreateSolidBrush(RGB(255, 0, 0));
  RECT rcField = { 0 };
  rcField.right = FIELD_SIZE.column;
  rcField.bottom = FIELD_SIZE.row;


  robots = robotsCopy;

  const int START_STEP = 7492; // the actual solution
  const int SLEEP_DURATION = 1000000;



  for (int step = 0; true; ++step) {
    if (step >= START_STEP) { // skip steps
      RECT rcText{ 10, 30, 100, 50};
      std::stringstream ss;
      ss << "Step " << step;
      DrawTextA(consoleDc, ss.str().c_str(), -1, &rcText, DT_LEFT | DT_TOP);

      // Clear image
      FillRect(memDc, &rcField, black);
      // Fill robots
      bool couldBeTree = robots.draw(memDc);
      if (couldBeTree) {
        RECT rcTLQ{ 0, 0, tlQuadrant.column, tlQuadrant.row };
        FrameRect(memDc, &rcTLQ, red);
        RECT rcTRQ{ trQuadrant.column, 0, FIELD_SIZE.column, trQuadrant.row };
        FrameRect(memDc, &rcTRQ, red);
      }

      // Copy to screen
      StretchBlt(consoleDc, 10, 50, 400, 400, memDc, 0, 0, FIELD_SIZE.column, FIELD_SIZE.row, SRCCOPY);

      if (couldBeTree) {
        MessageBoxA(NULL, "Could be a Tree", "Info", MB_ICONINFORMATION);
      }


      Sleep(SLEEP_DURATION);
    }

    robots.step();
  }



  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Result 1: " << sum << "\n"; // 208437768
  // Part 2: 7492 (no automatic image search, because I had no idea what shape/size I was supposed to expect)
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}