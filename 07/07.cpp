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


struct Sequence {
  int64_t result;
  std::vector<int64_t> numbers;


  bool valid() const {
    // First check whether we can reach that number by multiplying all values
    // Commented out, because it doesn't really speed up anything here
    // auto maxResult = std::ranges::fold_left(numbers, int64_t(0), [](int64_t sum, int64_t number) { return sum == 0 ? number : std::max(sum*number, sum+number); });
    // if (maxResult < result) {
    //   // The number is too big to be reachable
    //   return false;
    // }

    // For now just brute force all combinations... but we can search smarter by starting with + if the number is small and with * if the number is big
    // No sequence is longer than 12 numbers, so we need at most 2^12 attempts (=4096) per sequence.
    int maxAttempts = 1 << numbers.size() - 1; // = 2^(num-operators)
    for (int operatorMask = 0; operatorMask < maxAttempts; ++operatorMask) {
      // the bitmask will determine, which operators we are flipping
      int64_t currentResult = numbers[0];

      // Calculate the result according to our current 
      for (int numIdx = 1; numIdx < numbers.size(); ++numIdx) {
        if (((1 << (numIdx-1)) & operatorMask) != 0) {
          currentResult *= numbers[numIdx];
        } else {
          currentResult += numbers[numIdx];
        }
      }
         
      if (currentResult == result) {
        return true;
      }

      // try next operator combination
    }




    return false;
  }
};


int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  std::ifstream file("input.txt");
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



  auto t2 = std::chrono::high_resolution_clock::now();


  std::cout << "Part1: " << result << " (correct = " << correctSequences << ")\n"; // 6231007345478 (correct = 274)
  std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}