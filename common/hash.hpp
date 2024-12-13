#pragma once

#include <xhash>

// hash_combine as is used by boost
template <class T>
void hash_combine(std::size_t& hash, const T& v) {
  std::hash<T> hasher;
  hash ^= hasher(v) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

// Generic hasher function for arbitrary number of parameters
template<typename... T>
size_t hash_all(const T&... param) {
  size_t hash = 0;
  (hash_combine(hash, param), ...);
  return hash;
}

// Generic hash definition for pair
namespace std {
  template<typename A, typename B>
  struct hash<std::pair<A,B>> {
    size_t operator()(const std::pair<A, B>& pair) const {
      return hash_all(pair.first, pair.second);
    }
  };
}

