#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <common/vector.hpp>
#include <common/hash.hpp>

// implement as C++ Random number engine
struct MonkeyRandomPriceEngine {
  // UniformRandomBitGenerator (https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator)
  using Self = MonkeyRandomPriceEngine;
  using result_type = uint8_t;
  using internal_type = uint32_t;
  static result_type min() { return 0; }
  static result_type max() { return 9; }

  // RandomNumberEngine (https://en.cppreference.com/w/cpp/named_req/RandomNumberEngine)
  MonkeyRandomPriceEngine(internal_type seed = 0) : secretNumber(seed) {}
  MonkeyRandomPriceEngine(const Self& other) = default;
  
  void seed(internal_type seed = 0) {
    secretNumber = seed;
  }
  
  result_type operator()() {
    secretNumber ^= secretNumber * 64;
    secretNumber %= 16777216;
    secretNumber ^= secretNumber / 32;
    secretNumber %= 16777216;
    secretNumber ^= secretNumber * 2048;
    secretNumber %= 16777216;
    return secretNumber % 10;
  }

  void discard(size_t n) {
    for (size_t i = 0; i < n; ++i) {
      operator()();
    }
  }

  bool operator==(const Self& other) const = default;
  bool operator!=(const Self& other) const = default;
  internal_type secretNumber;
};

std::ostream& operator<<(std::ostream& out, const MonkeyRandomPriceEngine& e) { return out << e.secretNumber; }
std::istream& operator>>(std::istream& in, MonkeyRandomPriceEngine& e) { return in >> e.secretNumber; }


static constexpr int STEPS_TO_RUN = 2000;


int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  std::ifstream input("input.txt");
  std::vector<MonkeyRandomPriceEngine> states = { std::istream_iterator<uint32_t>(input), std::istream_iterator<uint32_t>() };

  // For Part 2 we have 19^4 possible sequences = 130.321
  // Simply go through all numbers and count the total value we would get for this sequence

  std::unordered_map<uint32_t/*sequence*/, int/*total profit*/> profitMap;
  int64_t sum = 0;
  for (auto& generator : states) {
    std::unordered_set<uint32_t/*sequence*/> matchedSequences; // only the first occurrence of a sequence is sold

    // The sequence number will be calculated by calculating the price differnce as (newPrice + 10 - oldPrice), which will 
    // guarantee an always positive value for the difference. This unsigened diff will then be or'ed into the current sequence number
    // after shifting it by 8 bits left. That way we can define an easy and efficient calculation to only keep the last four numbers and
    // uniquely identifiy all sequences.
    uint32_t sequenceNumber = 0;
    auto lastPrice = generator();
    // generate first three deltas (to not add an if-condition into the loop running for 2000 steps)
    for (int i = 0; i < 3; ++i) {
      auto newPrice = generator();
      sequenceNumber = (sequenceNumber << 8) | (newPrice + 10 - lastPrice);
      lastPrice = newPrice;
    }

    // Now run the remaining number of steps
    for (int i = 0; i < STEPS_TO_RUN-4; ++i) {
      auto newPrice = generator();
      sequenceNumber = (sequenceNumber << 8) | (newPrice + 10 - lastPrice);
      lastPrice = newPrice;
      if (matchedSequences.insert(sequenceNumber).second) {
        // First time seeing this sequence number -> add price to profit map for this sequence number
        profitMap[sequenceNumber] += newPrice;
      }
    }

    sum += generator.secretNumber;
  }

  uint32_t maxSequence = 0;
  int maxProfit = 0;
  // Now simply find the entry with the highest profit
  for (auto [sequence, profit] : profitMap) {
    if (profit > maxProfit) {
      maxSequence = sequence;
      maxProfit = profit;
    }
    maxProfit = std::max(maxProfit, profit);
  }


  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << sum << "\n"; // 13004408787
  std::cout << "Part 2: " << maxProfit << "\n"; // 1455 (0,1,-3,4)
  std::cout << "MaxSeq: " << (static_cast<int>((maxSequence >> 24) & 0xFF) - 10)
                   << "," << (static_cast<int>((maxSequence >> 16) & 0xFF) - 10)
                   << "," << (static_cast<int>((maxSequence >> 8) & 0xFF) - 10)
                   << "," << (static_cast<int>((maxSequence >> 0) & 0xFF) - 10) << "\n";


  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
