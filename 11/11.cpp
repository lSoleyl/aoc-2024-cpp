#include <iostream>
#include <fstream>
#include <list>
#include <chrono>
#include <ranges>
#include <execution>
#include <atomic>
#include <unordered_map>


namespace std {
  template<>
  struct hash<std::pair<int64_t, int>> {
    size_t operator()(const std::pair<int64_t, int>& entry) const {
      return (entry.first + entry.second) * (entry.second+1);
    }
  };
}


struct Stones : public std::list<int64_t> {
  Stones(std::istream&& input) {
    for (int64_t stone; input >> stone; ) {
      push_back(stone);
    }
  }
};

// Recursively calculates how many stones we will end up with after the given amount of steps.
// Uses an internal calculation cache to make the function actually computable.
int64_t stonesAfter(int64_t stone, int steps) {
  if (steps == 0) {
    return 1;
  }

  static std::unordered_map<std::pair<int64_t/*stone*/, int/*steps*/>, int64_t/*numStones*/> lookUpMap;
  std::pair<int64_t, int> lookupKey(stone, steps);
  auto pos = lookUpMap.find(lookupKey);
  if (pos != lookUpMap.end()) {
    // we already calculated this value -> return it
    return pos->second;
  }

  int64_t result;
  if (stone == 0) {
    result = stonesAfter(1, steps - 1);
  } else {
    int64_t nDigits = static_cast<int64_t>(std::log10(stone) + 1);
    if (nDigits % 2 == 0) {
      // Rule 2
      auto [left, right] = std::div(stone, std::pow(10, nDigits / 2));
      result = stonesAfter(left, steps - 1) + stonesAfter(right, steps - 1);
    } else {
      result = stonesAfter(stone * 2024, steps - 1);
    }
  }
  // store the result for future lookups
  lookUpMap.emplace(std::move(lookupKey), result);
  return result;
}


int main()
{
  auto t1 = std::chrono::high_resolution_clock::now();
  Stones stones(std::ifstream("input.txt"));
   
  // Part1:
  int64_t sum1 = 0;
  for (auto stone : stones) {
    sum1 += stonesAfter(stone, 25);
  }

  // Part2:
  int64_t sum2 = 0;
  for (auto stone : stones) {
    sum2 += stonesAfter(stone, 75);
  }
  
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part1: " << sum1 << "\n"; // 199753
  std::cout << "Part2: " << sum2 << "\n"; // 239413123020116
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";  
}
