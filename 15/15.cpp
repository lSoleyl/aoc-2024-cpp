#include <iostream>
#include <fstream>
#include <chrono>
#include <ranges>
#include <map>

#include <common/field.hpp>
#include <common/hash.hpp>



struct Warehouse : public Field {
  Warehouse(int width, int height): Field(width, height, '.') {} // constructor used by wide field
  Warehouse(std::istream&& input) : Field(input) {
    for (std::string line; std::getline(input, line);) {
      instructions += line;
    }

    // lookup robot position only once and update it afterwards
    robotPos = fromOffset(findOffset('@'));
  }


  void runInstructions() {
    for (auto instruction : instructions) {
      auto direction = instructionTranslation[instruction];
      if (tryMove(robotPos, direction)) {
        // move successful -> update robotPos
        robotPos += direction;
      }
    }
  }

  /** Attempt to move a robot or box from position in given direction.
   *  Operation will fail if attempting to move into a wall. If this operation fails,
   *  no move will have been performed. If it succeeds, all required moves will have been performed.
   */
  virtual bool tryMove(Vector fromPos, Vector direction, bool checkOnly = false) {
    Vector toPos = fromPos + direction;

    auto& atDestination = (*this)[toPos];
    if (atDestination == '#') {
      return false; // cannot move into wall
    } else if (atDestination == 'O' && !tryMove(toPos, direction)) {
      return false; // failed to move box, which had to be moved
    }

    // Otherwise move is possible...
    if (!checkOnly) {
      std::swap(atDestination, (*this)[fromPos]);
    }
    return true;
  }


  std::string instructions;
  Vector robotPos;

  std::map<char, Vector> instructionTranslation = {
    {'^', Vector::Up},
    {'<', Vector::Left},
    {'v', Vector::Down},
    {'>', Vector::Right}
  };
};


std::map<char, std::array<char, 2>> wideningMap = {
  {'#', {'#', '#'}},
  {'O', {'[', ']'}},
  {'.', {'.', '.'}},
  {'@', {'@', '.'}}
};



struct WarehouseWide : public Warehouse {
  WarehouseWide(const Warehouse& other) : Warehouse(other.size.column*2, other.size.row) {
    data.clear();
    data.reserve(size.row * size.column);

    // now copy the field data
    for (auto element : other.data) {
      data.insert_range(data.end(), wideningMap[element]);
    }

    instructions = other.instructions;

    // lookup robot position only once and update it afterwards
    robotPos = fromOffset(findOffset('@'));
  }

  virtual bool tryMove(Vector fromPos, Vector direction, bool checkOnly = false) override {
    Vector toPos = fromPos + direction;

    auto& atDestination = (*this)[toPos];
    if (atDestination == '#') {
      return false; // cannot move into wall
    } 

    // we need to treat horizontal and vertical movement differently...
    // because boxes are wider than they are tall
    bool horizontal = (direction.column != 0);

    if (horizontal) {
      // In horizontal direction we only need to try to move both box parts separately
      if ((atDestination == '[' || atDestination == ']') && !tryMove(toPos, direction, checkOnly)) {
        // Moving the box failed -> cannot perform move
        return false;
      }
    } else {
      // Vertical movement must differentiate, because we must move both box parts at the same time.
      // Here we must always call tryMove with the checkOnly flag as we do NOT want to perform any moves on the right branch 
      // if the left branch prevents the move
      if (atDestination == '[') {
        if (tryMove(toPos, direction, true) && tryMove(toPos + Vector::Right, direction, true)) { 
          // Move of both branches is possible -> perform the actual move (unless we are in a checkonly branch)
          if (!checkOnly) {
            tryMove(toPos, direction);
            tryMove(toPos + Vector::Right, direction);
          }
        } else {
          // Movement of at least one branch is not possible so the entire move is not possible
          return false;
        }
      } else if (atDestination == ']') {
        if (tryMove(toPos, direction, true) && tryMove(toPos + Vector::Left, direction, true)) {
          // Move of both branches is possible -> perform the actual move (unless we are in a checkonly branch)
          if (!checkOnly) {
            tryMove(toPos, direction);
            tryMove(toPos + Vector::Left, direction);
          }
        } else {
          // Movement of at least one branch is not possible so the entire move is not possible
          return false;
        }
      }
    }

    // Move is possible...
    if (!checkOnly) {
      std::swap(atDestination, (*this)[fromPos]);
    }
    return true;
  }
};



int main() {
  auto t1 = std::chrono::high_resolution_clock::now();
  Warehouse warehouse(std::ifstream("input.txt"));
  WarehouseWide wideHouse(warehouse);

  // Part 1:
  warehouse.runInstructions();
  
  // Now collect all pox positions
  int boxPosSum = 0;
  for (auto offset = warehouse.findOffset('O'); offset != std::numeric_limits<size_t>::max(); offset = warehouse.findOffset('O', offset+1)) {
    auto pos = warehouse.fromOffset(offset);
    boxPosSum += pos.row * 100 + pos.column;
  }


  // Part 2:
  wideHouse.runInstructions();

  // Now collect all pox positions
  int boxPosSum2 = 0;
  for (auto offset = wideHouse.findOffset('['); offset != std::numeric_limits<size_t>::max(); offset = wideHouse.findOffset('[', offset + 1)) {
    auto pos = wideHouse.fromOffset(offset);
    boxPosSum2 += pos.row * 100 + pos.column;
  }

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << boxPosSum << "\n"; // 1463715
  std::cout << "Part 2: " << boxPosSum2 << "\n"; // 1481392
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
  
  std::cout << warehouse;
}
