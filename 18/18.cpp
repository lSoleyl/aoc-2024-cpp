
#include <fstream>
#include <iostream>
#include <chrono>

#include <set>
#include <unordered_map>

#include <common/field.hpp>


struct ExpandEntry {
  ExpandEntry(Vector pos, int cost) : position(pos), cost(cost) {}

  // Entries are ordered based on their cost to expand the low cost entries first
  // We must include the position as well because we use an std::set<> and 
  // we would otherwise not include expand entries for different positions if they have the same costs
  std::strong_ordering operator<=>(const ExpandEntry& other) const {
    auto result = cost <=> other.cost;
    if (result == std::strong_ordering::equal) {
      result = position <=> other.position;
    }
    return result;
  }


  Vector position;
  int cost; // up until that position on the current path
};

struct MemorySpace : public Field {
  MemorySpace(std::istream&& input) : Field(71,71, '.') {
    Vector pos;
    char sep;
    while (input >> pos.x >> sep >> pos.y) {
      bytePositions.push_back(pos);
    }
  }

  // Part 1
  int findPath(Vector from, Vector to) {
    // simple dijkstra
    std::set<ExpandEntry> expandList = { {from, 0} };
    while (!expandList.empty()) {
      auto entry = *expandList.begin();
      expandList.erase(expandList.begin());

      if (!updateCosts(entry)) {
        // DO not expand this node, because we already know a cheaper (or equally expensive) path to this node
        // which thus has already been expanded
        continue;
      }

      if (entry.position == to) {
        // We are expanding the end tile -> we are done
        return entry.cost;
      }

      // We have 4 paths to expand one in each direction
      for (auto direction : Vector::AllDirections()) {
        auto nextPosition = entry.position + direction;
        if (validPosition(nextPosition) && (*this)[nextPosition] != '#') {
          // No wall -> we can expand in that direction
          expandList.insert({ nextPosition, entry.cost + 1 });
        }
      }
    }

    return -1; // no path found
  }

  
  int getCost(Vector position) const {
    auto pos = costMap.find(position);
    return (pos != costMap.end()) ? pos->second : std::numeric_limits<int>::max();
  }

  /** Add new/lower costs to each position or return false if the cost map already
   *  contains lower costs
   */
  bool updateCosts(ExpandEntry& fromEntry) {
    auto pos = costMap.find(fromEntry.position);
    if (pos != costMap.end()) {
      if (pos->second <= fromEntry.cost) {
        return false; // no update to more expensive path
      } else {
        pos->second = fromEntry.cost;
      }
    } else {
      // new path with yet unknown cost
      costMap.emplace(fromEntry.position, fromEntry.cost);
    }
    return true;
  }

  std::vector<Vector> bytePositions;
  std::unordered_map<Vector/*position*/, int/*cost*/> costMap;
};



int main()
{
  auto t1 = std::chrono::high_resolution_clock::now();

  MemorySpace memSpace(std::ifstream("input.txt"));
  for (int i = 0; i < 1024; ++i) {
    memSpace[memSpace.bytePositions[i]] = '#';
  }

  int minCost = memSpace.findPath(memSpace.topLeft(), memSpace.bottomRight());

  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part1: " << minCost << "\n"; // 290
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
