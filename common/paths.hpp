#pragma once

#include <set>
#include <vector>
#include <unordered_map>
#include <ranges>


#include "field.hpp"

/** Path finding class for Fields
 */

template<typename T>
struct PathFinderT {
  PathFinderT(FieldT<T>& field) : field(field) {}
  PathFinderT(FieldT<T>& field, Vector from, Vector to) : field(field), from(from), to(to) {}

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


  int findPath(Vector from, Vector to, bool expandAllFields = false) {
    this->from = from;
    this->to = to;
    return findPath(expandAllFields);
  }


  /** Calculates the minimal path from->to and returns the costs (or -1 if no such path exists)
   */
  int findPath(bool expandAllFields = false) {
    std::set<ExpandEntry> expandList = { {from, 0} };
    int pathCost = -1;
    while (!expandList.empty()) {
      auto entry = *expandList.begin();
      expandList.erase(expandList.begin());

      if (!updateCosts(entry)) {
        // DO not expand this node, because we already know a cheaper (or equally expensive) path to this node
        // which thus has already been expanded
        continue;
      }

      if (entry.position == to) {
        if (!expandAllFields) {
          // We are expanding the end tile -> we are done expanding
          return entry.cost;
        } else {
          pathCost = entry.cost;
        }
      }

      // We have 4 paths to expand one in each direction
      for (auto direction : Vector::AllDirections()) {
        auto nextPosition = entry.position + direction;
        if (field.validPosition(nextPosition) && field[nextPosition] != '#') {
          // No wall -> we can expand in that direction
          expandList.insert({ nextPosition, entry.cost + 1 });
        }
      }
    }

    return pathCost;
  }

  /** Returns greedily the cheapest path from->to after findPath has been called
   */
  std::vector<Vector> getCheapestPath() const {
    std::vector<Vector> path;

    if (costMap.find(to) == costMap.end()) {
      return path; // no path found -> return empty vector to signal this
    }

    int cost = getCost(to);
    for (Vector pos = to; pos != from; ) {
      path.push_back(pos);

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
    path.push_back(from);
    std::ranges::reverse(path);
    return path;
  }




  int getCost(Vector position) const {
    auto pos = costMap.find(position);
    return (pos != costMap.end()) ? pos->second : std::numeric_limits<int>::max();
  }

  /** Add new/lower costs to each position or return false if the cost map already
   *  contains lower costs
   */
  bool updateCosts(const ExpandEntry& fromEntry) {
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


  FieldT<T>& field;
  Vector from, to;
  std::unordered_map<Vector/*position*/, int/*cost*/> costMap;
};

using PathFinder = PathFinderT<char>;

