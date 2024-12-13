#include <iostream>
#include <fstream>
#include <list>
#include <chrono>
#include <ranges>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#include <common/field.hpp>
#include <common/hash.hpp>

struct Region {
  Region(char type, const Vector& startPosition, const Field& field) : type(type) {
    collectPositions(startPosition, field);
  }

  char type;
  std::unordered_set<Vector> positions;

private:
  void collectPositions(const Vector& startPosition, const Field& field) {
    positions.insert(startPosition);
    for (auto direction : Vector::ALL_DIRECTIONS) {
      auto neighbor = startPosition + direction;
      if (field.validPosition(neighbor) && field[neighbor] == type && !positions.contains(neighbor)) {
        collectPositions(neighbor, field);
      }
    }
  }
};


int main() {
  auto t1 = std::chrono::high_resolution_clock::now();
  Field field(std::ifstream("input.txt"));

  std::vector<Region> regions;
  
  // Collect the regions while checking all positions
  for (size_t offset = 0; offset < field.data.size(); ++offset) {
    auto position = field.fromOffset(offset);
    auto type = field[position];
    
    if (!std::ranges::any_of(regions, [&](const Region& region) { return region.type == type && region.positions.contains(position); })) {
      // Region not yet known -> new region
      regions.emplace_back(type, position, field);
    }
  }


  int cost = 0;
  int discountCost = 0;

  for (auto& region : regions) {
    int area = region.positions.size();
    int perimeter = 0;
    int sides = 0;

    std::unordered_set<std::pair<Vector/*position*/, Vector/*direction*/>> countedSides; // to not double count
    
    for (auto& position : region.positions) {
      // Count up the sides, which either touch the field border or another type of field
      // The total number of these sides make up the perimeter
      for (auto direction : Vector::ALL_DIRECTIONS)  {
        auto neighbor = position + direction;
        if (!field.validPosition(neighbor) || field[neighbor] != region.type) {
          // This side is not touching another internal field 
          ++perimeter;
          
          if (!countedSides.contains({ position, direction })) {
            // We didn't count this side yet... continue in CW and CCW direction from current position and
            // mark all positions making up this side to not double count them when checking other positions
            ++sides;
            for (auto sideDirection : { direction.rotateCW(), direction.rotateCCW() }) {
              // Continue in this direction for as long as the positions are part of the field and
              // as long as the line doesn't enter into the field (not being a perimeter anymore)
              for (auto nextPos = position + sideDirection; region.positions.contains(nextPos) && !field.isAt(region.type, nextPos + direction); nextPos += sideDirection) {
                countedSides.emplace(nextPos, direction);
              }
            }
          }
        }
      }
    }


    cost += area * perimeter;
    discountCost += area * sides;
  }

  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part1: " << cost << "\n"; // 1431316
  std::cout << "Part2: " << discountCost << "\n";
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
