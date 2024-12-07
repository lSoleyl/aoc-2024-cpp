
#include <iostream>
#include <fstream>
#include <ranges>
#include <valarray>
#include <vector>
#include <iterator>

struct Vector {
  Vector(int column = 0, int row = 0) : column(column), row(row) {}

  Vector operator+(const Vector& other) const { return Vector(column + other.column, row + other.row); }
  Vector& operator+=(const Vector& other) {
    column += other.column;
    row += other.row;
    return *this;
  }

  Vector operator-(const Vector& other) const { return Vector(column - other.column, row - other.row); }
  Vector& operator-=(const Vector& other) {
    column -= other.column;
    row -= other.row;
    return *this;
  }

  Vector operator*(int factor) const { return Vector(column * factor, row * factor); }

  Vector rotateCCW() const {
    return Vector(row, -column); //rotated by 90° clockwise
  }
  

  bool operator==(const Vector& other) const = default;
  bool operator!=(const Vector& other) const = default;
  bool operator<(const Vector& other) const { return column < other.column && row < other.row; }
  bool operator<=(const Vector& other) const { return column <= other.column && row <= other.row; }
  bool operator>(const Vector& other) const { return column > other.column && row > other.row;  }
  bool operator>=(const Vector& other) const { return column >= other.column && row >= other.row;  }

  int column, row;
};

struct Field {
  Field(std::istream&& source) : size(0,0) {
    for (auto line : std::ranges::subrange(std::istream_iterator<std::string>(source), std::istream_iterator<std::string>())) {
      data += line;
      size.column = line.length();
      ++size.row;
    }
  }

  struct iterator {
    using element_type = char;
    using reference = char&;
    using pointer = char*;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using difference_type = ptrdiff_t;

    iterator() : field(nullptr) {}
    iterator(Field& field, const Vector& pos, const Vector& direction = Vector(0,0)) : field(&field), pos(pos), direction(direction) {}

    struct sentinel {};

    bool operator==(const iterator& other) const { return pos == other.pos; }
    bool operator!=(const iterator& other) const { return pos != other.pos; }

    // We must use negate the condition due to the partial ordering of vectors
    bool operator==(sentinel) const { return !valid(); }
    bool valid() const { return pos >= Vector(0, 0) && pos < field->size; }

    iterator& operator++() { pos += direction; return *this; }
    iterator operator++(int) { auto copy = *this; ++(*this); return copy; }
    char& operator*() const { return field->data[pos.row * field->size.column + pos.column]; }

    Vector pos, direction;
    Field* field;
  };

  Vector fromOffset(size_t offset) const { return Vector(offset % size.column, offset / size.column); }

  auto rangeFromPositionAndDirection(const Vector& position, const Vector& direction) {
    iterator begin(*this, position, direction);
    return std::ranges::subrange(begin, iterator::sentinel());
  }

  bool startsWithStringAtPositionAndDirection(const Vector& position, const Vector& direction, const std::string& matchString) {
    return std::ranges::mismatch(matchString, rangeFromPositionAndDirection(position, direction)).in1 == matchString.end();
  }

  bool crossMatchesAtCenter(size_t centerOffset, const std::string& matchString) {
    // F... I thought that vertical and horizontal also count <.<
    const auto center = fromOffset(centerOffset);

    int matchingStrings = 0;
    Vector direction (1, 1); // UP-RIGHT
    for (int i = 0; i < 4; ++i) { // check all four rotations
      if (startsWithStringAtPositionAndDirection(center + direction, direction * -1, matchString)) {
        ++matchingStrings;
      }
      direction = direction.rotateCCW();
    }

    return matchingStrings == 2; // "MAS" must match in exactly two perpendicular directions
  }

  Vector size;
  std::string data;
};





int main()
{
  Field field(std::ifstream("input.txt"));
  
  const auto allDirections = {
    Vector(0,1), Vector(1,1), Vector(1,0), Vector(1,-1),
    Vector(0,-1), Vector(-1,-1), Vector(-1, 0), Vector(-1, 1)
  };
  
  // Part 1
  std::string searchString("XMAS");
  int matches = 0;

  // Find all letters 'X'
  for (size_t offset = field.data.find('X'); offset != std::string::npos; offset = field.data.find('X', offset+1)) {
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
  for (size_t offset = field.data.find('A'); offset != std::string::npos; offset = field.data.find('A', offset + 1)) {
    if (field.crossMatchesAtCenter(offset, searchString)) {
      ++crossMatches;
    }
  }

  std::cout << "Result2: " << crossMatches << "\n"; // 2048
}