#include <iostream>
#include <fstream>
#include <ranges>
#include <regex>
#include <string>
#include <vector>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <algorithm>
#include <string_view>
#include <execution>
#include <chrono>
#include <atomic>

#include <common/task.hpp>

using Operator = int64_t(*)(int64_t a, int64_t b);

int64_t add(int64_t a, int64_t b) { return a + b; }
int64_t mul(int64_t a, int64_t b) { return a * b; }
int64_t concat(int64_t a, int64_t b) { 
  // Determine the number of digits in b and just multiply a by 10^digits and add to b. This is by far more efficient than the string operation.
  // 1 = 0, 9 = 0...., 10 = 1, 99 = 1...., 100 = 2
  int bDigits = static_cast<int>(std::log10(b) + 1);
  return a * std::pow<int64_t>(10, bDigits) + b;
}

std::vector<Operator> operators = { add, mul };

struct Sequence {
  int64_t result;
  std::vector<int64_t> numbers;


  bool valid() const {
    // Simply brute force all operator combinations through recursively checking the sequence
    return matchesResult(numbers[0], numbers.begin() + 1, numbers.end());
  }


  bool matchesResult(int64_t currentResult, std::vector<int64_t>::const_iterator numbersIt, std::vector<int64_t>::const_iterator numbersEnd) const {
    if (numbersIt == numbersEnd) {
      return currentResult == result;
    }

    auto number = *numbersIt;
    ++numbersIt;
    for (auto op : operators) {
      auto nextResult = op(currentResult, number);
      if (matchesResult(nextResult, numbersIt, numbersEnd)) { // numbersIt already incremented
        return true;
      }
    }

    return false;
  }
};


int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  auto file = task::input();
  std::string line;
  std::vector<Sequence> sequences;
  while (std::getline(file, line)) {
    std::stringstream ss(line);
    Sequence sequence;
    char separator;
    ss >> sequence.result >> separator;
    while (ss) {
      int64_t number;
      ss >> number;
      if (ss) {
        sequence.numbers.push_back(number);
      }
    }
    sequences.push_back(std::move(sequence));
  }
  

  int64_t result = 0;
  int correctSequences = 0;

  for (auto& sequence : sequences) {
    if (sequence.valid()) {
      result += sequence.result;
      ++correctSequences;
    }
  }



  // Part2:
  // Now we simply add the third operator into the operator list and repeat
  
  operators.push_back(concat);

  std::atomic<int64_t> result2 = 0;
  std::atomic<int> correctSequences2 = 0;


  // Idk why, but when adding std::execution::par the loop takes 30 instead of 8 seconds!!! But how!?
  // Okay it was the inefficient implementation of concat(). By implementing concat() arithmetically we could
  // reduce execution time down to 176ms (single-thread) or 37ms (parallel)
  std::for_each(std::execution::par, sequences.begin(), sequences.end(), [&](const Sequence& sequence) {
    if (sequence.valid()) {
      result2 += sequence.result;
      ++correctSequences2;
    }
  });

  auto t2 = std::chrono::high_resolution_clock::now();


  std::cout << "Part1: " << result << " (correct = " << correctSequences << ")\n";
  std::cout << "Part2: " << result2 << " (correct = " << correctSequences2 << ")\n";
  std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}