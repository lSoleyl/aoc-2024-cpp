#include <iostream>
#include <fstream>
#include <chrono>
#include <ranges>
#include <set>
#include <unordered_map>

#include <common/field.hpp>
#include <common/hash.hpp>

const int RotationCost = 1000;
const int StepCost = 1;

struct ExpandEntry {
  ExpandEntry() = default;
  ExpandEntry(int cost, Vector position, Vector orientation) : cost(cost), position(position), orientation(orientation) {}

  // Entries are ordered based on their cost to expand the low cost entries first
  // We must include the position and orientation into the ordering as well because we use an std::set<> and 
  // we would otherwise not include expand entries for differnt positions if they have the same costs
  std::strong_ordering operator<=>(const ExpandEntry& other) const {
    auto result = cost <=> other.cost;
    if (result == std::strong_ordering::equal) {
      result = position.compWiseOrdering(other.position);
    }
    if (result == std::strong_ordering::equal) {
      result = orientation.compWiseOrdering(other.orientation);
    }
    return result;
  }

  int cost; // to reach this position
  Vector position; // of the node to expand
  Vector orientation; // at the position
};



struct Maze : public Field {
  Maze(std::istream&& input) : Field(input) {
    startPos = fromOffset(findOffset('S'));
    endPos = fromOffset(findOffset('E'));
  }

  int solve() { // <- return the min cost reaching end
    
    std::set<ExpandEntry> expandList = { {0, startPos, Vector::Right } };

    while (!expandList.empty()) {
      auto entry = *expandList.begin();
      expandList.erase(expandList.begin());

      if (entry.position == endPos) {
        // we are expanding the end tile -> the cost are now known
        return entry.cost;
      }
      
      if (!updateCosts(entry)) {
        // DO not expand this node, because we already know a cheaper (or equally expensive) path to this node
        // which thus has already been expanded
        continue;
      }

      // We have 3 paths to expand (step forward, CW rotation, CCW rotation)
      auto stepPosition = entry.position + entry.orientation;
      if ((*this)[stepPosition] != '#') {
        // No wall -> we can expand in that direction
        expandList.insert({ entry.cost + StepCost, stepPosition, entry.orientation });
      }

      // Always add both rotations (they will be conisdered later for expansion)
      // This will actually blow up the expand list quite a bit
      expandList.insert({ entry.cost + RotationCost, entry.position, entry.orientation.rotateCW() });
      expandList.insert({ entry.cost + RotationCost, entry.position, entry.orientation.rotateCCW() });
    }

    return -1;
  }

  int getCost(Vector position, Vector orientation) const {
    auto pos = costMap.find({ position, orientation });
    return (pos != costMap.end()) ? pos->second : std::numeric_limits<int>::max();
  }

  /** Add new/lower costs to each position/orientation or return false if the cost map already
   *  contains lower costs
   */
  bool updateCosts(ExpandEntry& fromEntry) {
    auto pos = costMap.find({ fromEntry.position, fromEntry.orientation });
    if (pos != costMap.end()) {
      if (pos->second <= fromEntry.cost) {
        return false; // no update to more expensive path
      } else {
        pos->second = fromEntry.cost;
      }
    } else {
      // new path with yet unknown cost
      costMap.insert({{ fromEntry.position, fromEntry.orientation }, fromEntry.cost});
    }
    return true;
  }

  
  Vector startPos;
  Vector endPos;
  std::unordered_map<std::pair<Vector/*pos*/, Vector/*orientation*/>, int> costMap; // All entries not in the cost map have implicitly an infinite cost
};




int main() {
  auto t1 = std::chrono::high_resolution_clock::now();
  Maze maze(std::ifstream("input.txt"));

  auto minCost = maze.solve();

  

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << minCost << "\n";
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
