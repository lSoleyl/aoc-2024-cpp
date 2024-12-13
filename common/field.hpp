#pragma once

#include <ranges>
#include <iostream>
#include <vector>
#include "vector.hpp"

template<typename Element>
struct FieldT {
  FieldT(std::istream&& source) : size(0, 0) {
    for (auto line : std::views::istream<std::string>(source)) {
      for (char ch : line) {
        data.emplace_back(ch); // Element must be constructible from a single char
      }
      size.column = line.length();
      ++size.row;
    }
  }

  template<typename Self>
  auto& operator[](this Self&& self, const Vector& pos) { return self.data[self.toOffset(pos)]; }
  bool validPosition(const Vector& pos) const { return pos >= Vector(0, 0) && pos < size; }
  int toOffset(const Vector& pos) const { return pos.row * size.column + pos.column; }
  Vector fromOffset(size_t offset) const { return Vector(offset % size.column, offset / size.column); }
  size_t findOffset(const Element& element, size_t startOffset = 0) const {
    auto pos = std::find(data.begin() + startOffset, data.end(), element);
    return pos != data.end() ? std::distance(data.begin(), pos) : std::numeric_limits<size_t>::max();
  }


  struct iterator {
    using element_type = Element;
    using reference = Element&;
    using pointer = Element*;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using difference_type = ptrdiff_t;

    iterator() : field(nullptr) {}
    iterator(FieldT& field, const Vector& pos, const Vector& direction = Vector(0, 0)) : field(&field), pos(pos), direction(direction) {}

    struct sentinel {};

    bool operator==(const iterator& other) const { return pos == other.pos; }
    bool operator!=(const iterator& other) const { return pos != other.pos; }

    // We must use negate the condition due to the partial ordering of vectors
    bool operator==(sentinel) const { return !valid(); }
    bool valid() const { return pos >= Vector(0, 0) && pos < field->size; }

    iterator& operator++() { pos += direction; return *this; }
    iterator operator++(int) { auto copy = *this; ++(*this); return copy; }
    Element& operator*() const { return field->data[pos.row * field->size.column + pos.column]; }

    Vector pos, direction;
    FieldT* field;
  };

  auto rangeFromPositionAndDirection(const Vector& position, const Vector& direction) {
    iterator begin(*this, position, direction);
    return std::ranges::subrange(begin, iterator::sentinel());
  }

  Vector size;
  std::vector<Element> data;
};

using Field = FieldT<char>;


template<typename Element>
std::ostream& operator<<(std::ostream& out, FieldT<Element>& field) {
  for (int row = 0; row < field.size.row; ++row) {
    for (int column = 0; column < field.size.column; ++column) {
      out << field[Vector(column, row)];
    }
    out << "\n";
  }
  return out;
}