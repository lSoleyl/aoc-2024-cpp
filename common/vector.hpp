#pragma once

#include <xhash>
#include <iostream>


struct Vector {
  Vector(int column = 0, int row = 0) : column(column), row(row) {}

  static const Vector UP;
  static const Vector RIGHT;
  static const Vector DOWN;
  static const Vector LEFT;

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

  Vector rotateCCW() const {
    return Vector(row, -column); //rotate by 90° counter clockwise
  }


  bool operator==(const Vector& other) const = default;
  bool operator!=(const Vector& other) const = default;
  bool operator<(const Vector& other) const { return column < other.column && row < other.row; }
  bool operator<=(const Vector& other) const { return column <= other.column && row <= other.row; }
  bool operator>(const Vector& other) const { return column > other.column && row > other.row; }
  bool operator>=(const Vector& other) const { return column >= other.column && row >= other.row; }

  int column, row;
};


const Vector Vector::UP(0, -1); // rows are incremented down
const Vector Vector::RIGHT(1, 0);
const Vector Vector::DOWN(0, 1);
const Vector Vector::LEFT(-1, 0);


namespace std {
  template<>
  struct hash<Vector> {
    size_t operator()(const Vector& vec) const { return ((vec.row << 16) ^ vec.column); }
  };
}

std::ostream& operator<<(std::ostream& out, const Vector& v) {
  return out << "(" << v.column << "," << v.row << ")";
}
