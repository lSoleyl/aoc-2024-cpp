
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
  std::set<Vector> findPath() {
    const Vector from = topLeft();
    const Vector to = bottomRight();

    // simple dijkstra
    costMap.clear();
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
        // We are expanding the end tile -> we are done expanding
        break;
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

    // Now greedily collect ONE cheapest path
    std::set<Vector> path;
    
    if (costMap.find(to) == costMap.end()) {
      return path; // no path found -> return empty set to signal this
    }
    
    int cost = getCost(to);
    for (Vector pos = to; pos != from; ) {
      path.insert(pos);

      for (auto direction : Vector::AllDirections()) {
        auto prevPos = pos + direction;
        auto prevCost = getCost(prevPos);
        if (prevCost < cost) {
          cost = prevCost;
          pos = prevPos;
          break; // found the next step in the cheapest path
        }
      }
    }

    path.insert(from);
    return path;
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

  // Part 1
  for (int i = 0; i < 1024; ++i) {
    memSpace[memSpace.bytePositions[i]] = '#';
  }
  auto minPath = memSpace.findPath();
  auto minCost = minPath.size();


  // Part 2: Brute force of simply performing dijskstra after each change takes ~ 3s
  //         The smarter algorithm below will calculate the shortest path and only 
  //         recalculate the path if a byte falls onto that shortest path to verify that
  //         a shortest path still exists. This approach takes 60ms

  Vector bytePos;
  for (int i = 1024; i < memSpace.bytePositions.size(); ++i) {
    bytePos = memSpace.bytePositions[i];
    memSpace[bytePos] = '#';

    if (minPath.contains(bytePos)) {
      // The byte fell into the current minimal path -> recalculate the minimal path
      // to ensure it still exists
      minPath = memSpace.findPath();
      if (minPath.empty()) {
        break; // no path found!
      }
    }
  }

  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part1: " << minCost << "\n"; // 290
  std::cout << "Part2: " << bytePos << "\n"; // 64,54
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
