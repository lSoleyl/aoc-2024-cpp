#include <iostream>
#include <fstream>
#include <ranges>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <queue>


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

template<typename Element>
struct Field {
  Field(std::istream&& source) : size(0, 0) {
    for (auto line : std::ranges::subrange(std::istream_iterator<std::string>(source), std::istream_iterator<std::string>())) {
      for (char ch : line) {
        data.emplace_back(ch); // Element must be constructible from a single char
      }
      size.column = line.length();
      ++size.row;
    }
  }

  bool validPosition(const Vector& pos) const { return pos >= Vector(0, 0) && pos < size; }
  Element& operator[](const Vector& pos) { return data[toOffset(pos)]; }
  int toOffset(const Vector& pos) const { return pos.row * size.column + pos.column; }
  Vector fromOffset(size_t offset) const { return Vector(offset % size.column, offset / size.column); }

  Vector size;
  std::vector<Element> data;
};

template<typename ElementType>
std::ostream& operator<<(std::ostream& out, Field<ElementType>& field) {
  for (int row = 0; row < field.size.row; ++row) {
    for (int column = 0; column < field.size.column; ++column) {
      out << field[Vector(column, row)] << ' ';
    }
    out << "\n";
  }
  return out;
}




struct MapElement {
  MapElement(char ch) : height(ch) {}

  int8_t height; // as ASCII char
  std::vector<int/*offset*/> reachableTops; // here we collect all reachable tops (with duplicates), which we will later filter out
};

std::ostream& operator<<(std::ostream& out, const MapElement& element) {
  out << std::setfill(' ') << std::setw(2) << element.reachableTops.size();
  return out;
}



int main() {
  auto t1 = std::chrono::high_resolution_clock::now();
  Field<MapElement> field(std::ifstream("input.txt"));

  const auto ALL_DIRECTIONS = { Vector(0,-1), Vector(1,0), Vector(0,1), Vector(-1,0) };

  // We collect the nodes to process in a queue to ensure we will first process all '9', then all '8', ... that way
  // we never have to update any trailhead values for other numbers than the current processed one

  std::queue<int/*offset*/> toProcess;
  // Part1: First collect all '9' elements into the toProcess queue and then process each one of them
  for (int offset = 0; offset < field.data.size(); ++offset) {
    if (field.data[offset].height == '9') { // since we must reach a '9' we are only interested in paths, which end at '9'
      field.data[offset].reachableTops.push_back(offset); // initialize: only this top is reachable
      toProcess.push(offset);
    }
  }

  // Now keep processing until all reachable nodes have been processed
  while (!toProcess.empty()) {
    auto nodePosition = field.fromOffset(toProcess.front());
    toProcess.pop();

    // Expand the current node by checking all directions
    auto& currentNode = field[nodePosition];
    for (auto direction : ALL_DIRECTIONS) {
      auto nextPosition = nodePosition + direction;
      if (field.validPosition(nextPosition)) {
        // We can expand in that direction
        auto& nextNode = field[nextPosition];
        if (nextNode.height == currentNode.height-1) {
          // a valid trail continuation
          if (!nextNode.reachableTops.empty()) {
            // already expanded there by another node
            nextNode.reachableTops.insert(nextNode.reachableTops.end(), currentNode.reachableTops.begin(), currentNode.reachableTops.end());
          } else {
            // node not yet expanded and into processing queue
            nextNode.reachableTops = currentNode.reachableTops;
            toProcess.push(field.toOffset(nextPosition));
          }
        }
      }
    }
  }



  // Now we simply need to sum up the trailHeads values in all '0' nodes
  int allHeadSum = 0;
  int uniqueHeadSum = 0;
  for (auto& node : field.data) {
    if (node.height == '0') {
      // Now we can determine the reachable heads with duplicates and without.
      allHeadSum += node.reachableTops.size();
      uniqueHeadSum += (node.reachableTops | std::ranges::to<std::set>()).size();
    }
  }
  
  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part1: " << uniqueHeadSum << "\n"; // 617 
  std::cout << "Part2: " << allHeadSum << "\n"; // 1477
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
