#pragma once

#include <ranges>
#include <iterator>
#include <iostream>
#include <string>
#include <vector>
#include "vector.hpp"

template<typename Element>
struct FieldT {
  FieldT(int width, int height, Element fill) : size(width, height) {
    data.resize(width * height, fill);
  }

  FieldT(std::istream&& source) : FieldT(source) {}
  FieldT(std::istream& source) : size(0, 0) {
    for (std::string line; std::getline(source, line);) {
      if (line.empty()) { // special case for Day 15 where the field is followed by a newline and instructions
        break;
      }
      for (char ch : line) {
        data.emplace_back(ch); // Element must be constructible from a single char
      }
      size.x = static_cast<int>(line.length());
      ++size.y;
    }
  }

  template<typename Self>
  auto& operator[](this Self&& self, const Vector& pos) { return self.data[self.toOffset(pos)]; }
  bool validPosition(const Vector& pos) const { return pos.x >= 0 && pos.y >= 0 && pos.x < size.x && pos.y < size.y; }
  bool isAt(const Element& element, const Vector& pos) const { return validPosition(pos) && (*this)[pos] == element; }

  int toOffset(const Vector& pos) const { return pos.y * size.x + pos.x; }
  Vector fromOffset(size_t offset) const { return Vector(static_cast<int>(offset) % size.x, static_cast<int>(offset) / size.x); }
  size_t findOffset(const Element& element, size_t startOffset = 0) const {
    auto pos = std::find(data.begin() + startOffset, data.end(), element);
    return pos != data.end() ? std::distance(data.begin(), pos) : std::numeric_limits<size_t>::max();
  }
  
  Vector topLeft() const { return Vector(0, 0); }
  Vector topRight() const { return Vector(size.x - 1, 0); }
  Vector bottomLeft() const { return Vector(0, size.y - 1); }
  Vector bottomRight() const { return size - Vector(1, 1); }


  struct iterator {
    using element_type = Element;
    using reference = Element&;
    using pointer = Element*;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = int;

    iterator() : field(nullptr) {}
    iterator(FieldT& field, const Vector& pos, const Vector& direction = Vector(0, 0)) : field(&field), pos(pos), direction(direction) {}

    // Sentinel type, which can alternatively be used in place of an end() iterator
    struct sentinel {};

    bool operator==(const iterator& other) const { return pos == other.pos; }
    bool operator!=(const iterator& other) const { return pos != other.pos; }

    // We must use negate the condition due to the partial ordering of vectors
    bool operator==(sentinel) const { return !valid(); }
    bool valid() const { return field->validPosition(pos); }

    iterator& operator++() { pos += direction; return *this; }
    iterator operator++(int) { auto copy = *this; ++(*this); return copy; }
    iterator& operator-() { pos -= direction; return *this; }
    iterator operator--(int) { auto copy = *this; --(*this); return copy; }
    Element& operator*() const { return (*field)[pos]; }

    iterator& operator+=(int offset) { pos += direction * offset; return *this; }
    iterator& operator-=(int offset) { pos -= direction * offset; return *this; }
    iterator operator+(int offset) const { auto copy = *this; copy += offset; return copy; }
    iterator operator-(int offset) const { auto copy = *this; copy -= offset; return copy; }
    
    // Determine the difference between two positions
    // Assumption: other iterator will pass through this position otherwise behavior is undefined
    difference_type operator-(const iterator& other) const { 
      auto delta = this->pos - other.pos;
      return other.direction.x != 0 ? delta.x / other.direction.x : delta.y / other.direction.y;
    }

    Element& operator[](int index) const { return (*field)[pos + (direction * index)]; }

    Vector pos, direction;
    FieldT* field;
  };

  auto rangeFromPositionAndDirection(const Vector& position, const Vector& direction) {
    iterator begin(*this, position, direction);

    int dx = std::max(direction.x > 0 ? (size.x - position.x) / direction.x :
                      direction.x < 0 ? (0 - position.x) / direction.x : std::numeric_limits<int>::max(), 0);

    int dy = std::max(direction.y > 0 ? (size.y - position.y) / direction.y :
                      direction.y < 0 ? (0 - position.y) / direction.y : std::numeric_limits<int>::max(), 0);

    auto distanceToEnd = std::min(dx, dy);
    return std::ranges::subrange(begin, begin + distanceToEnd);
  }

  // Return a range representing the i'th row (0-based) left to right
  auto row(int row) {
    iterator begin(*this, Vector(0, row), Vector::Right);
    return std::ranges::subrange(begin, begin + size.x);
  }

  // Returns a range representing the i'th column (0-based) top to bottom
  auto column(int column) {
    iterator begin(*this, Vector(column, 0), Vector::Down);
    return std::ranges::subrange(begin, begin + size.y);
  }

  Vector size;
  std::vector<Element> data;
};

using Field = FieldT<char>;


template<typename Element>
std::ostream& operator<<(std::ostream& out, FieldT<Element>& field) {
  for (int y = 0; y < field.size.y; ++y) {
    for (int x = 0; x < field.size.x; ++x) {
      out << field[Vector(x, y)];
    }
    out << "\n";
  }
  return out;
}