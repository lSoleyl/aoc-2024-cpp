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

  struct iterator {
    using element_type = char;
    using reference = char&;
    using pointer = char*;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using difference_type = ptrdiff_t;

    iterator() : field(nullptr) {}
    iterator(Field& field, const Vector& pos, const Vector& direction = Vector(0, 0)) : field(&field), pos(pos), direction(direction) {}

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

struct State {
  State(Field& field, const Vector& position, const Vector& direction) : field(&field), position(position), direction(direction), obstacle(nullptr) {}
  enum class Result { LEFT_FIELD, IN_LOOP };
  enum class Action { STEP, ROTATE };




  Result run() {
    do {
      nextStep();
    } while (!leftField() && !stuckInLoop());

    return leftField() ? Result::LEFT_FIELD : Result::IN_LOOP;
  }

  
  void nextStep() {
    visited.insert(position);
    visitedOrientations.emplace(position, direction);

    auto nextPosition = position + direction;
    if (field->validPosition(nextPosition) && ((*field)[nextPosition] == '#' || (obstacle && nextPosition == *obstacle))) {
      // obstacle in front -> rotate
      direction = direction.rotateCW();
    } else {
      // continue moving in that direction
      position = nextPosition;
    }
  }

  bool leftField() const { return !field->validPosition(position); }
  bool stuckInLoop() const { return visitedOrientations.contains(std::make_pair(position, direction)); }

  Field* field;
  std::unordered_set<Vector> visited;
  std::unordered_set<std::pair<Vector/*pos*/, Vector/*direction*/>> visitedOrientations;
  Vector position;
  Vector direction;
  Vector* obstacle; // additional obstacle (used to check multiple potential obstacles in parallel without modifying the field)
};

#include <chrono>

int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  Field field(std::ifstream("input.txt"));
  
  auto startPosition = field.fromOffset(field.data.find('^'));

  
  const State startState(field, startPosition, Vector(0, -1)); // start direction is UP as rows increment downward
  State firstRun(startState);
  firstRun.run();
  
  // Part2 : At each visited position [except for the start position] try adding an obstacle and check whether this causes a loop
  //         My first attempt at putting these obstacles in the way as we walk had the flaw that putting the obstacle late into the
  //         journey could prevent the guard from even reaching that point.
  //         This is pretty brute force, but I cannot think of a better solution for now
  std::unordered_set<Vector> possibleLoops;
  std::mutex m;


  // Parellelized solution as original takes 5.2 seconds, parallel one takes 1.1 seconds (or 780ms without debugger)
  std::for_each(std::execution::par, firstRun.visited.begin(), firstRun.visited.end(), [&](Vector position) {
    State state(startState);
    state.obstacle = &position;
    if (state.run() == State::Result::IN_LOOP) {
      std::lock_guard<std::mutex> guard(m); // synchronize access to set!
      possibleLoops.insert(position);
    }
  });

  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part 1: " << firstRun.visited.size() << "\n"; // 4752
  std::cout << "Part 2: " << possibleLoops.size() << "\n"; // 1838

  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}