#include <fstream>
#include <iostream>
#include <chrono>
#include <ranges>
#include <algorithm>
#include <numeric>
#include <map>
#include <unordered_map>

#include <common/paths.hpp>

struct RaceField : public Field {
  RaceField(std::istream&& input) : Field(input), finder(*this) {
    finder.findPath(fromOffset(findOffset('S')), fromOffset(findOffset('E')), true);
  }


  int calculateCheatSavings() {
    int savings = 0;
    auto path = finder.getCheapestPath();

    for (auto position : path) {
      // Check on each position 
      auto walkCost = finder.getCost(position) + 2;
      for (auto direction : Vector::AllDirections()) {
        auto pos1 = position + direction;
        auto pos2 = pos1 + direction;
        auto cheatCost = finder.getCost(pos2);
        if (cheatCost < std::numeric_limits<int>::max()) { // i.e. a valid position and not a wall
          if (cheatCost - walkCost >= 100) {
            // Found a path where we can save 100 time steps by cheating
            ++savings;
          }
        }
      }
    }
    
    return savings;
  }
  
  PathFinder finder;
};




int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  RaceField field(std::ifstream("input.txt"));
  
  auto savings = field.calculateCheatSavings();




  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << savings << "\n"; // 1422
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
