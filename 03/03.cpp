#include <regex>
#include <iostream>
#include <fstream>
#include <ranges>

#include <common/task.hpp>

int main()
{
  // Read full input file into a string
  std::string content = task::inputString();


  bool enabled = true;
  int result = 0;
  int disabledSum = 0;
  std::regex pattern(R"!!((do(n't)?\(\))|mul\(([0-9]{1,3}),([0-9]{1,3})\))!!");
  for (auto match : std::ranges::subrange(std::sregex_iterator(content.begin(), content.end(), pattern), std::sregex_iterator())) {
    if (match[1].matched) {
      // instruction matched
      enabled = (match[1].str() == "do()");
    } else {
      // mul matched
      auto product = std::stoi(match[3]) * std::stoi(match[4]);
      result += product;
      if (!enabled) {
        disabledSum += product;
      }
    }
  }
  
  std::cout << "Part1: " << result << "\n";
  std::cout << "Part2: " << (result - disabledSum) << "\n";
}
