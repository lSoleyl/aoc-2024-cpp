#include <iostream>
#include <fstream>
#include <ranges>
#include <vector>
#include <iterator>

#include <common/field.hpp>

struct XField : public Field {
  XField(std::istream&& source) : Field(std::move(source)) {}

  bool startsWithStringAtPositionAndDirection(const Vector& position, const Vector& direction, const std::string& matchString) {
    return std::ranges::mismatch(matchString, rangeFromPositionAndDirection(position, direction)).in1 == matchString.end();
  }

  bool crossMatchesAtCenter(size_t centerOffset, const std::string& matchString) {
    // F... I thought that vertical and horizontal also count <.<
    const auto center = fromOffset(centerOffset);

    int matchingStrings = 0;
    Vector direction = Vector::UpRight;
    for (int i = 0; i < 4; ++i) { // check all four rotations
      if (startsWithStringAtPositionAndDirection(center + direction, direction * -1, matchString)) {
        ++matchingStrings;
      }
      direction = direction.rotateCW();
    }

    return matchingStrings == 2; // "MAS" must match in exactly two perpendicular directions
  }
};


int main()
{
  XField field(std::ifstream("input.txt"));
  
  const auto allDirections = {
    Vector(0,1), Vector(1,1), Vector(1,0), Vector(1,-1),
    Vector(0,-1), Vector(-1,-1), Vector(-1, 0), Vector(-1, 1)
  };
  
  // Part 1
  std::string searchString("XMAS");
  int matches = 0;

  // Find all letters 'X'
  for (size_t offset = field.findOffset('X'); offset != std::numeric_limits<size_t>::max(); offset = field.findOffset('X', offset+1)) {
    // Compare to search string in all directions
    for (auto& direction : allDirections) {
      if (field.startsWithStringAtPositionAndDirection(field.fromOffset(offset), direction, searchString)) {
        ++matches;
      }
    }
  }

  std::cout << "Result1: " << matches << "\n"; // 2685


  // Part 2
  searchString = "MAS";

  int crossMatches = 0;
  // Find all letters 'A'
  for (size_t offset = field.findOffset('A'); offset != std::numeric_limits<size_t>::max(); offset = field.findOffset('A', offset + 1)) {
    if (field.crossMatchesAtCenter(offset, searchString)) {
      ++crossMatches;
    }
  }

  std::cout << "Result2: " << crossMatches << "\n"; // 2048
}