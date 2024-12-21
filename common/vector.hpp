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

  // Converts a direction vector to a character representing this direction or '0'´ for no direction.
  char toChar() const {
    if (*this == VectorT::Zero) return '0';
    if (*this == VectorT::Left) return '<';
    if (*this == VectorT::Up) return '^';
    if (*this == VectorT::Right) return '>';
    if (*this == VectorT::Down) return 'v';
    throw std::exception("not a direction vector");
    return '?';
  }

  // Converts a direction char into a vector (inverse of 'toChar()'). Will throw an exception if an unsupported direction is passed.
  static VectorT fromChar(char ch) {
    switch (ch) {
      case '0': return VectorT::Zero;
      case '<': return VectorT::Left;
      case '^': return VectorT::Up;
      case '>': return VectorT::Right;
      case 'v': return VectorT::Down;
    }
    throw std::exception("not a valid direction char");
    return VectorT::Zero;
  }

  // Calculate the number of single steps needed to reach other from this vector
  auto stepDistance(const VectorT& other) const {
    return std::abs(x - other.x) + std::abs(y - other.y);
  }

  // Apply given functor to each component and return the result
  template<typename MapFn>
  VectorT apply(MapFn mapFn) const { return VectorT(mapFn(x), mapFn(y)); }

  // Performs a component wise compare and returns a result vector with -1,0,1 for each component
  VectorT compare(const VectorT& other) const { return (*this - other).apply([](int value) { return std::clamp(value, -1, 1); }); }

  bool operator==(const VectorT& other) const = default;
  bool operator!=(const VectorT& other) const = default;

  // Vector ordering is defined in row major order, which makes it equal to offset ordering
  // This ordering is only defined to enable sorting vectors and using them in std::set and std::map and has no further semantic meaning
  std::strong_ordering operator<=>(const VectorT& other) const {
    auto result = (y <=> other.y);
    if (result == std::strong_ordering::equal) {
      result = x <=> other.x;
    }
    return result;
  }

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
