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
  VectorT(T x = 0, T y = 0) : x(x), y(y) {}

  // We sadly cannot assign the values here or use constexpr, because the class is "incomplete" at this time
  static const VectorT Zero;
  static const VectorT Up;
  static const VectorT Right;
  static const VectorT Down;
  static const VectorT Left;

  static const std::initializer_list<const VectorT>& AllDirections() {
    static const std::initializer_list<const VectorT> directions = { VectorT::Up, VectorT::Right, VectorT::Down, VectorT::Left };
    return directions;
  }

  VectorT operator+(const VectorT& other) const { return VectorT(x + other.x, y + other.y); }
  VectorT& operator+=(const VectorT& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  VectorT operator-(const VectorT& other) const { return VectorT(x - other.x, y - other.y); }
  VectorT& operator-=(const VectorT& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  // Important: the following operation performs integer division!
  VectorT operator/(T divisor) const { return VectorT(x / divisor, y / divisor); }
  VectorT& operator/(T divisor) {
    x /= divisor;
    y /= divisor;
    return *this;
  }
  VectorT operator*(T factor) const { return VectorT(x * factor, y * factor); }
  VectorT& operator*=(T factor) {
    x *= factor;
    y *= factor;
    return *this;
  }

  VectorT operator%(T divisor) const { return VectorT(x % divisor, y % divisor); }
  VectorT& operator%=(T divisor) {
    x %= divisor;
    y %= divisor;
    return *this;
  }

  // element wise modulo
  VectorT operator%(const VectorT& other) const { return VectorT(x % other.x, y % other.y); }
  VectorT& operator%=(const VectorT& other) {
    x %= other.x;
    y %= other.y;
    return *this;
  }

  // positive modulo
  VectorT pMod(T divisor) const { return VectorT(::pMod(x, divisor), ::pMod(y, divisor)); }

  // element wise positive modulo
  VectorT pMod(const VectorT& other) const { return VectorT(::pMod(x, other.x), ::pMod(y, other.y)); }


  VectorT rotateCW() const {
    return VectorT(-y, x); //rotated by 90° clockwise
  }

  VectorT rotateCCW() const {
    return VectorT(y, -x); //rotate by 90° counter clockwise
  }

  // Apply given functor to each component and return the result
  template<typename MapFn>
  VectorT apply(MapFn mapFn) const { return VectorT(mapFn(x), mapFn(y)); }

  // Performs a component wise compare and returns a result vector with -1,0,1 for each component
  VectorT compare(const VectorT& other) const { return (*this - other).apply([](int value) { return std::clamp(value, -1, 1); }); }


  // returns a strong ordering value pased on component wise comparison where the column is compared first and only if the column
  // is equal the row is compared. This establishes the column as a primary and more important dimension, which is why this isn't the default comparison.
  std::strong_ordering compWiseOrdering(const VectorT& other) const {
    auto result = (x <=> other.x);
    if (result == std::strong_ordering::equal) {
      result = y <=> other.y;
    }
    return result;
  }

  bool operator==(const VectorT& other) const = default;
  bool operator!=(const VectorT& other) const = default;
  bool operator<(const VectorT& other) const { return x < other.x && y < other.y; }
  bool operator<=(const VectorT& other) const { return x <= other.x && y <= other.y; }
  bool operator>(const VectorT& other) const { return x > other.x && y > other.y; }
  bool operator>=(const VectorT& other) const { return x >= other.x && y >= other.y; }

  T x, y;
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

namespace std {
  template<typename T>
  struct hash<VectorT<T>> {
    size_t operator()(const VectorT<T>& vec) const { return ((vec.y << 16) ^ vec.x); }
  };
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const VectorT<T>& v) {
  return out << "(" << v.x << "," << v.y << ")";
}

using Vector = VectorT<int>;
