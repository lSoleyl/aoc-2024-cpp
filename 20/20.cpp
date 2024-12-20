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
    auto startPos = fromOffset(findOffset('S'));
    auto endPos = fromOffset(findOffset('E'));
    // We will search from end to start... that way our cost map will for each position contain the minimum 
    // cost to the end, not the amount of steps from the start, which is not helpful for the second part
    finder.findPath(endPos, startPos, true);
  }

  // Part 1
  int calculateCheatSavings() {
    int savings = 0;
    auto path = finder.getCheapestPath();
    std::ranges::reverse(path); // we searched end->start so we need to reverse the shortest path

    for (auto position : path) {
      // Check on each position 
      auto walkCost = finder.getCost(position) - 2;
      for (auto direction : Vector::AllDirections()) {
        auto pos1 = position + direction;
        auto pos2 = pos1 + direction;
        auto cheatCost = finder.getCost(pos2);
        if (cheatCost < std::numeric_limits<int>::max()) { // i.e. valid and not a wall
          if (walkCost - cheatCost >= 100) {
            // Found a path where we can save 100 time steps by cheating
            ++savings;
          }
        }
      }
    }
    
    return savings;
  }

  // Part 2
  int calculateCheatSavings2() {
    // On each path along the shortest path, we can cheat
    // We must determine all positions with at least +100 cost, that are reachable in 20 steps
    std::map<int/*costToTarget*/, std::set<Vector>> distanceToTargetMap;
    for (auto [pos, cost] : finder.costMap) {
      distanceToTargetMap[cost].insert(pos);
    }

    int savings = 0;
    auto path = finder.getCheapestPath();
    std::ranges::reverse(path); // we searched end->start so we need to reverse the shortest path

    for (auto position : path) {
      // Now at each position check all paths in the distanceToTargetMap with a cost reduction of at least 100
      auto distanceToTarget = finder.getCost(position);

      for (auto& [jumpDistanceToTarget, jumpPositions] : distanceToTargetMap) {
        if (jumpDistanceToTarget + 100 > distanceToTarget) {
          // we can abort iteraton here, because we won't be able to save the 100ps for all following entries
          break;
        }
        
        // Now from these positions we must find the ones, which are reachable within up to 20 steps
        // And within the steps made we will save 100 ps
        for (auto jumpPosition : jumpPositions) {
          auto jumpDistance = position.stepDistance(jumpPosition);
          if (jumpDistance <= 20 && (distanceToTarget - jumpDistance) >= (jumpDistanceToTarget + 100)) {
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
  auto savings2 = field.calculateCheatSavings2();




  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << savings << "\n"; // 1422
  std::cout << "Part 2: " << savings2 << "\n"; // 1009299
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
