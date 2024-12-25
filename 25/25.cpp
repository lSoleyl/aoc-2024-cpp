#include <fstream>
#include <iostream>
#include <chrono>
#include <array>
#include <vector>
#include <algorithm>

#include <common/stream.hpp>

using Key = std::array<int, 5>;
using Lock = std::array<int, 5>;

struct LockData {
  LockData(std::istream&& input) {
    auto lines = stream::lines(input);
    for (auto chunk : lines | std::views::chunk(8)) {
      int lineNo = 0;
      bool isLock;
      std::array<std::string, 5> elementLines;
      for (auto line : chunk) {
        switch (lineNo) {
          case 0: // first line determines the type
            isLock = line[0] == '#';
            break;

          case 6:
          case 7:
            break; // ignore last and following empty line

          default:
            elementLines[lineNo - 1] = line;
            break;
        }

        ++lineNo;
      }

      if (!isLock) {
        std::ranges::reverse(elementLines);
      }

      std::array<int, 5> heights = { 0,0,0,0,0 };
      for (int height = 0; height < elementLines.size(); ++height) {
        auto& line = elementLines[height];
        for (int column = 0; column < line.size(); ++column) {
          if (line[column] == '#') {
            heights[column] = height + 1;
          }
        }
      }

      (isLock ? locks : keys).push_back(heights);
    }
  }

  int countMatchingKeys() const {
    int matchingPairs = 0;
    for (auto& key : keys) {
      for (auto& lock : locks) {
        if (keyMatchesLock(key, lock)) {
          ++matchingPairs;
        }
      }
    }
    return matchingPairs;
  }

  static bool keyMatchesLock(const Key& key, const Lock& lock) {
    for (int i = 0; i < key.size(); ++i) {
      if (key[i] + lock[i] > 5) {
        return false;
      }
    }
    return true;
  }


  std::vector<Key> keys;
  std::vector<Lock> locks;
};





int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  
  LockData data(std::ifstream("input.txt"));
  auto matchingPairs = data.countMatchingKeys();

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << matchingPairs << "\n"; // 3395
  // Apparently this has no part 2... man parsing the input was more effort than the actual task x)
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
