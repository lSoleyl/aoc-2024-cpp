
#include <iostream>
#include <fstream>
#include <ranges>
#include <algorithm>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>

bool isValidSequence(const std::vector<int>& numbers) {
  auto checkView = numbers
    | std::views::adjacent_transform<2>([](int a, int b) { return a - b; })
    | std::views::adjacent_transform<2>([](int a, int b) {
    return (a * b) > 0  // both changes are > 0 and are in the same direction
      && std::abs(a) <= 3 // first change is between 1 and 3
      && std::abs(b) <= 3 // second change is between 1 and 3
      ;
  });
    
  // A valid number sequence will produce a view of only true values
  return std::ranges::all_of(checkView, [](bool value) { return value; });
}


bool hasValidSubSequence(const std::vector<int>& numbers) {
  for (int i = 0; i < numbers.size(); ++i) {
    auto copy = numbers;
    copy.erase(copy.begin() + i); // remove one element
    if (isValidSequence(copy)) {
      return true; // this subsequence is valid -> okay
    }
  }

  return false;
}

int main()
{
  std::ifstream file("input.txt");
  std::string line;
  int validSequences = 0;
  int validSubSequences = 0;
  while (std::getline(file, line)) {
    // Convert into list of numbers
    std::stringstream ss(line);
    std::vector<int> sequence{std::istream_iterator<int>(ss), std::istream_iterator<int>()};
    if (isValidSequence(sequence)) {
      ++validSequences;
    } else if (hasValidSubSequence(sequence)) {
      ++validSubSequences;
    }
  }

  std::cout << "Part1: " << validSequences << "\n";
  std::cout << "Part2: " << validSequences + validSubSequences << "\n";
}