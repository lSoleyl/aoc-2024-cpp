
#include <iostream>
#include <fstream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <unordered_map>


int main()
{
  std::vector<int> vec1, vec2;
  std::unordered_map<int/*number*/, int/*occurrences*/> similarity;

  // Read in file and  populate both vectors
  {
    std::ifstream file("input.txt");
    while (file) {
      int a, b;
      file >> a >> b;
      if (!file.eof()) { // last read didn't encounter eof, so it is a complete line
        vec1.push_back(a);
        vec2.push_back(b);
        ++similarity[b];
      }
    }
  }

  // Sort both vectors
  std::sort(vec1.begin(), vec1.end());
  std::sort(vec2.begin(), vec2.end());

  // Now zip and calculate the required sum of differences
  auto sum = std::ranges::fold_left(std::views::zip(vec1, vec2), 0, [](int sum, std::tuple<int&,int&> tuple) {
    return sum + std::abs(std::get<0>(tuple) - std::get<1>(tuple));
  });
  
  std::cout << "Part1: "  << sum << "\n";

   
  auto similarityScore = std::ranges::fold_left(vec1, 0, [&similarity](int sum, int value) {
    return sum + (similarity[value] * value);
  });

  std::cout << "Part2: " << similarityScore << "\n";
}
// 2057374