#include <regex>
#include <iostream>
#include <fstream>
#include <ranges>

int main()
{
  std::string content;

  {
    // Read full file into the content string
    std::ifstream file("input.txt", std::ios::binary);
    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    content.resize(fileSize);
    file.read(const_cast<char*>(content.data()), content.size());
  }


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
  
  std::cout << "Part1: " << result << "\n"; // 184122457
  std::cout << "Part2: " << (result - disabledSum) << "\n"; // 107862689
}
