#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

#include <common/field.hpp>
#include <common/task.hpp>

int main()
{
  auto t1 = std::chrono::high_resolution_clock::now();
  Field field(task::input());


  // First collect all antenna positions grouped by their frequencies
  std::unordered_map<char/*freq*/, std::vector<Vector>/*positions*/> antennas;
  for (size_t offset = 0; offset < field.data.size(); ++offset) {
    char freq = field.data[offset];
    if (freq != '.') {
      antennas[freq].push_back(field.fromOffset(offset));
    }
  }

  // Not very efficient, but straight forward
  std::unordered_set<Vector> antiNodes;
  for (auto& freqEntry : antennas) {
    // For now just check all possible combinations
    for (auto pos1 : freqEntry.second) {
      for (auto pos2 : freqEntry.second) {
        if (pos1 != pos2) {
          auto nodePos = pos2 + (pos2 - pos1); // simply add the vector from 1->2 onto 2 to get to the antinode position
          if (field.validPosition(nodePos)) {
            antiNodes.insert(nodePos);
          }
        }
      }
    }
  }

  // Part2: 
  std::unordered_set<Vector> allAntiNodes;
  for (auto& freqEntry : antennas) {
    // For now just check all possible combinations
    for (auto pos1 : freqEntry.second) {
      for (auto pos2 : freqEntry.second) {
        if (pos1 != pos2) {
          // Apparently the field is designed in a way that we don't need to normalize the 
          // direction vector to the minimum valid distance... (i.e. it doesn't contain constellations like: "..a....a..")
          auto direction = pos2 - pos1; // 1->2
          auto nodePos = pos1;
          for (auto nodePos = pos1; field.validPosition(nodePos); nodePos += direction) {
            allAntiNodes.insert(nodePos);
          }
        }
      }
    }
  }



  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part1: " << antiNodes.size() << "\n";
  std::cout << "Part2: " << allAntiNodes.size() << "\n";
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
