
#include <fstream>
#include <iostream>
#include <chrono>
#include <ranges>
#include <algorithm>
#include <numeric>
#include <unordered_map>

#include <common/stream.hpp>
#include <common/split.hpp>
#include <common/task.hpp>

struct Towels {
  Towels(std::ifstream&& input) {
    std::string line;
    std::getline(input, line);
    for (auto towel : common::split(line, ", ")) {
      towels.emplace_back(towel);
    }
    std::ranges::sort(towels);
    std::getline(input, line); // read empty line
    designs = { std::istream_iterator<std::string>(input), std::istream_iterator<std::string>() };
  }

  // Part 1: by erasing all impossible designs, we make the second part a bit cheaper
  int filterImpossibleDesigns() {
    std::erase_if(designs, [this](const std::string& design) { return !designPossible(design); });
    return designs.size(); // return the possible ones
  }

  bool designPossible(std::string_view design) const {
    // Find matching prefixes quickly using binary search for the first letter
    auto firstLetter = design[0];
    auto pos = std::ranges::lower_bound(towels, design.substr(0,1), {}, [](const std::string& towel) { return std::string_view(towel); });

    for (; pos != towels.end() && (*pos)[0] == firstLetter; ++pos) {
      if (*pos == design) {
        return true; // we completed the design
      }

      if (design.starts_with(*pos) && designPossible(design.substr(pos->size()))) {
        return true;
      }
    }
    
    return false;
  }

  // Part 2: count all possible ways to create the design
  int64_t countAllDesignOptions() {
    return std::ranges::fold_left(designs, int64_t(0), [this](int64_t acc, auto& design) { return acc + countDesignOptions(design); });
  }

  int64_t countDesignOptions(std::string_view design) const {
    // Memoization cache is direly needed for analyzing these long sequences
    static std::unordered_map<std::string_view, int64_t> cache;
    auto cachePos = cache.find(design);
    if (cachePos != cache.end()) {
      return cachePos->second;
    }

    // Find matching prefixes quickly using binary search for the first letter
    auto firstLetter = design[0];
    auto pos = std::ranges::lower_bound(towels, design.substr(0, 1), {}, [](const std::string& towel) { return std::string_view(towel); });

    int64_t options = 0;
    for (; pos != towels.end() && (*pos)[0] == firstLetter; ++pos) {
      if (*pos == design) {
        ++options; // we completed the design -> 1 possible option
      }

      if (design.starts_with(*pos)) {
        // Add all options to arrange the suffix to the count
        options += countDesignOptions(design.substr(pos->size()));
      }
    }

    cache.emplace(design, options);
    return options;
  }



  std::vector<std::string> towels;
  std::vector<std::string> designs;
};



int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  // Part 1:
  Towels towels(task::input());
  auto towelCount = towels.filterImpossibleDesigns();

  // Part 2:
  auto allOptions = towels.countAllDesignOptions();
 


  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << towelCount << "\n";
  std::cout << "Part 2: " << allOptions << "\n";
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
