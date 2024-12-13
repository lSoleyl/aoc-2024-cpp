#pragma once

#include <xhash>
#include <iostream>

template<typename T>
struct VectorT {
  VectorT(T column = 0, T row = 0) : column(column), row(row) {}

  static const VectorT ZERO;
  static const VectorT UP;
  static const VectorT RIGHT;
  static const VectorT DOWN;
  static const VectorT LEFT;

  static const VectorT ALL_DIRECTIONS[4];

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
  VectorT operator*(T factor) const { return VectorT(column * factor, row * factor); }

  VectorT rotateCW() const {
    return VectorT(-row, column); //rotated by 90° clockwise
  }

  VectorT rotateCCW() const {
    return VectorT(row, -column); //rotate by 90° counter clockwise
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

template<typename T> const VectorT<T> VectorT<T>::ZERO(0, 0);
template<typename T> const VectorT<T> VectorT<T>::UP(0, -1); // rows are incremented down
template<typename T> const VectorT<T> VectorT<T>::RIGHT(1, 0);
template<typename T> const VectorT<T> VectorT<T>::DOWN(0, 1);
template<typename T> const VectorT<T> VectorT<T>::LEFT(-1, 0);

template<typename T> const VectorT<T> VectorT<T>::ALL_DIRECTIONS[4] = { VectorT<T>::UP, VectorT<T>::RIGHT, VectorT<T>::DOWN, VectorT<T>::LEFT };

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
