#pragma once

#include <xhash>
#include <iostream>
#include <algorithm> // std::clamp


namespace {
  template<typename T>
  T pMod(T a, T b) {
    auto result = a % b;
    return result < 0 ? result + b : result;
  }
}


template<typename T>
struct VectorT {
  VectorT(T column = 0, T row = 0) : column(column), row(row) {}

  static const VectorT Zero;
  static const VectorT Up;
  static const VectorT Right;
  static const VectorT Down;
  static const VectorT Left;

  static const VectorT AllDirections[4];

  VectorT operator+(const VectorT& other) const { return VectorT(column + other.column, row + other.row); }
  VectorT& operator+=(const VectorT& other) {
    column += other.column;
    row += other.row;
    return *this;
  }

  VectorT operator-(const VectorT& other) const { return VectorT(column - other.column, row - other.row); }
  VectorT& operator-=(const VectorT& other) {
    column -= other.column;
    row -= other.row;
    return *this;
  }

  // Important: the following operation performs integer division!
  VectorT operator/(T divisor) const { return VectorT(column / divisor, row / divisor); }
  VectorT& operator/(T divisor) {
    column /= divisor;
    row /= divisor;
    return *this;
  }
  VectorT operator*(T factor) const { return VectorT(column * factor, row * factor); }
  VectorT& operator*=(T factor) {
    column *= factor;
    row *= factor;
    return *this;
  }

  VectorT operator%(T divisor) const { return VectorT(column % divisor, row % divisor); }
  VectorT& operator%=(T divisor) {
    column %= divisor;
    row %= divisor;
    return *this;
  }

  // element wise modulo
  VectorT operator%(const VectorT& other) const { return VectorT(column % other.column, row % other.row); }
  VectorT& operator%=(const VectorT& other) {
    column %= other.column;
    row %= other.row;
    return *this;
  }

  // positive modulo
  VectorT pMod(T divisor) const { return VectorT(::pMod(column, divisor), ::pMod(row, divisor)); }

  // element wise positive modulo
  VectorT pMod(const VectorT& other) const { return VectorT(::pMod(column, other.column), ::pMod(row, other.row)); }


  VectorT rotateCW() const {
    return VectorT(-row, column); //rotated by 90° clockwise
  }

  VectorT rotateCCW() const {
    return VectorT(row, -column); //rotate by 90° counter clockwise
  }

  // Apply given functor to each component and return the result
  template<typename MapFn>
  VectorT apply(MapFn mapFn) const { return VectorT(mapFn(column), mapFn(row)); }

  // Performs a component wise compare and returns a result vector with -1,0,1 for each component
  VectorT compare(const VectorT& other) const { return (*this - other).apply([](int value) { return std::clamp(value, -1, 1); }); }


  // returns a strong ordering value pased on component wise comparison where the column is compared first and only if the column
  // is equal the row is compared. This establishes the column as a primary and more important dimension, which is why this isn't the default comparison.
  std::strong_ordering compWiseOrdering(const VectorT& other) const {
    auto result = (column <=> other.column);
    if (result == std::strong_ordering::equal) {
      result = row <=> other.row;
    }
    return result;
  }

  bool operator==(const VectorT& other) const = default;
  bool operator!=(const VectorT& other) const = default;
  bool operator<(const VectorT& other) const { return column < other.column && row < other.row; }
  bool operator<=(const VectorT& other) const { return column <= other.column && row <= other.row; }
  bool operator>(const VectorT& other) const { return column > other.column && row > other.row; }
  bool operator>=(const VectorT& other) const { return column >= other.column && row >= other.row; }

  T column, row;
};

template<typename T>
VectorT<T> operator*(T factor, const VectorT<T>& vector) {
  return vector * factor;
}

template<typename T> const VectorT<T> VectorT<T>::Zero(0, 0);
template<typename T> const VectorT<T> VectorT<T>::Up(0, -1); // rows are incremented down
template<typename T> const VectorT<T> VectorT<T>::Right(1, 0);
template<typename T> const VectorT<T> VectorT<T>::Down(0, 1);
template<typename T> const VectorT<T> VectorT<T>::Left(-1, 0);

template<typename T> const VectorT<T> VectorT<T>::AllDirections[4] = { VectorT<T>::Up, VectorT<T>::Right, VectorT<T>::Down, VectorT<T>::Left };

namespace std {
  template<typename T>
  struct hash<VectorT<T>> {
    size_t operator()(const VectorT<T>& vec) const { return ((vec.row << 16) ^ vec.column); }
  };
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const VectorT<T>& v) {
  return out << "(" << v.column << "," << v.row << ")";
}

using Vector = VectorT<int>;
