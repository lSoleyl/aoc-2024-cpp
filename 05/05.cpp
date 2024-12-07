
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

std::unordered_map<int, std::unordered_set<int>> beforeSet, afterSet;

bool isValidSequence(const std::vector<int>& numbers) {
  for (auto pos = numbers.begin(), end = numbers.end(); pos != end; ++pos) {
    // make sure that no numbers before pos are in the afterSet
    if (std::any_of(numbers.begin(), pos, [&](int number) { return afterSet[*pos].contains(number); })) {
      return false;
    }

    // make sure that no numbers after pos are in the before set
    if (std::any_of(pos + 1, numbers.end(), [&](int number) { return beforeSet[*pos].contains(number); })) {
      return false;
    }
  }

  return true;
}




int main()
{
  

  std::ifstream file("input.txt");
  std::string line;

  // Read conditions
  while (std::getline(file, line) && !line.empty()) {    
    std::stringstream ss(line);
    int before, after;
    char split;
    ss >> before >> split >> after;
    afterSet[before].insert(after);
    beforeSet[after].insert(before);
  }

  int sum = 0;
  std::regex comma(",");
  std::vector<std::vector<int>> incorrectSequences;
  while (std::getline(file, line)) {
    auto numbers = std::ranges::to<std::vector>(std::ranges::subrange(std::sregex_token_iterator(line.begin(), line.end(), comma, -1), std::sregex_token_iterator())
                                               | std::views::transform([](const std::string& str) { return std::stoi(str); }));
    
    if (isValidSequence(numbers)) {
      sum += numbers[numbers.size()/ 2]; // add the middle page number
    } else {
      incorrectSequences.push_back(std::move(numbers)); // save the incorrect sequence for later fixing (Part2)
    }
  }
  
  std::cout << "Part1: " << sum << "\n"; // 4996


  // Part2 
  int fixedSum = 0;
  for (auto numbers : incorrectSequences) {
    while (!isValidSequence(numbers)) { // loop in case a single iteration won't fix the sequence
      for (auto pos = numbers.begin(), end = numbers.end(); pos != end; ++pos) {
        // find a number preceding *pos, which requires *pos to be before that number
        auto beforePos = std::find_if(numbers.begin(), pos, [&](int number) { return beforeSet[number].contains(*pos); });
        if (beforePos != pos) {
          // move pos before that number
          std::rotate(beforePos, pos, pos + 1);
        }

        // we can continue iteration, because iterators following pos are not touched
      }
    }

    fixedSum += numbers[numbers.size() / 2]; // add the middle number of the fixed sequence
  }


  std::cout << "Part2: " << fixedSum << "\n";
}
