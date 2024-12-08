#include <iostream>
#include <fstream>
#include <ranges>
#include <regex>
#include <string>
#include <vector>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <algorithm>
#include <string_view>
#include <execution>
#include <chrono>


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

  Vector rotateCW() const {
    return Vector(-row, column); //rotated by 90° clockwise
  }


  bool operator==(const Vector& other) const = default;
  bool operator!=(const Vector& other) const = default;
  bool operator<(const Vector& other) const { return column < other.column && row < other.row; }
  bool operator<=(const Vector& other) const { return column <= other.column && row <= other.row; }
  bool operator>(const Vector& other) const { return column > other.column && row > other.row; }
  bool operator>=(const Vector& other) const { return column >= other.column && row >= other.row; }

  int column, row;
};

namespace std {
  template<>
  struct hash<Vector> {
    size_t operator()(const Vector& vec) const { return (vec.row << 16 | vec.column); }
  };

  template<>
  struct hash<std::pair<Vector, Vector>> {
    size_t operator()(const std::pair<Vector, Vector>& posDir) const { return std::hash<Vector>()(posDir.first) ^ std::hash<Vector>()(posDir.second); }
  };
}

struct Field {
  Field(std::istream&& source) : size(0, 0) {
    for (auto line : std::ranges::subrange(std::istream_iterator<std::string>(source), std::istream_iterator<std::string>())) {
      data += line;
      size.column = line.length();
      ++size.row;
    }
  }

  bool validPosition(const Vector& pos) const { return pos >= Vector(0, 0) && pos < size; }
  char& operator[](const Vector& pos) { return data[toOffset(pos)]; }
  int toOffset(const Vector& pos) const { return pos.row * size.column + pos.column; }
  Vector fromOffset(size_t offset) const { return Vector(offset % size.column, offset / size.column); }
  std::string_view row(int row) const {
    return std::string_view(data.data() + row * size.column, size.column);
  }

  Vector size;
  std::string data;
};

std::ostream& operator<<(std::ostream& out, const Field& field) {
  for (int row = 0; row < field.size.row; ++row) {
    out << field.row(row) << "\n";
  }
  return out;
}

int main()
{
  auto t1 = std::chrono::high_resolution_clock::now();
  Field field(std::ifstream("input.txt"));


  // First collect all antenna positions grouped by their frequencies
  std::unordered_map<char/*freq*/, std::vector<Vector>/*positions*/> antennas;
  for (size_t offset = 0; offset < field.data.size(); ++offset) {
    char freq = field.data[offset];
    if (freq != '.') {
      antennas[freq].push_back(field.fromOffset(offset));
    }
  }

  // Not very efficient, but straight forward
  std::unordered_set<Vector> antiNodes;
  for (auto& freqEntry : antennas) {
    // For now just check all possible combinations
    for (auto pos1 : freqEntry.second) {
      for (auto pos2 : freqEntry.second) {
        if (pos1 != pos2) {
          auto nodePos = pos2 + (pos2 - pos1); // simply add the vector from 1->2 onto 2 to get to the antinode position
          if (field.validPosition(nodePos)) {
            antiNodes.insert(nodePos);
          }
        }
      }
    }
  }

  // Part2: 
  std::unordered_set<Vector> allAntiNodes;
  for (auto& freqEntry : antennas) {
    // For now just check all possible combinations
    for (auto pos1 : freqEntry.second) {
      for (auto pos2 : freqEntry.second) {
        if (pos1 != pos2) {
          // Apparently the field is designed in a way that we don't need to normalize the 
          // direction vector to the minimum valid distance... (i.e. it doesn't contain constellations like: "..a....a..")
          auto direction = pos2 - pos1; // 1->2
          auto nodePos = pos1;
          for (auto nodePos = pos1; field.validPosition(nodePos); nodePos += direction) {
            allAntiNodes.insert(nodePos);
          }
        }
      }
    }
  }



  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part1: " << antiNodes.size() << "\n"; // 344
  std::cout << "Part2: " << allAntiNodes.size() << "\n";
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
