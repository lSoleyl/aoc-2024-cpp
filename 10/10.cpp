#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <queue>
#include <ranges>
#include <set>

#include <common/field.hpp>
#include <common/task.hpp>

struct MapElement {
  MapElement(char ch) : height(ch) {}

  int8_t height; // as ASCII char
  std::vector<int/*offset*/> reachableTops; // here we collect all reachable tops (with duplicates), which we will later filter out
};

std::ostream& operator<<(std::ostream& out, const MapElement& element) {
  out << std::setfill(' ') << std::setw(2) << element.reachableTops.size() << ' ';
  return out;
}



int main() {
  auto t1 = std::chrono::high_resolution_clock::now();
  FieldT<MapElement> field(task::input());

  // We collect the nodes to process in a queue to ensure we will first process all '9', then all '8', ... that way
  // we never have to update any trailhead values for other numbers than the current processed one

  std::queue<int/*offset*/> toProcess;
  // Part1: First collect all '9' elements into the toProcess queue and then process each one of them
  for (int offset = 0; offset < field.data.size(); ++offset) {
    if (field.data[offset].height == '9') { // since we must reach a '9' we are only interested in paths, which end at '9'
      field.data[offset].reachableTops.push_back(offset); // initialize: only this top is reachable
      toProcess.push(offset);
    }
  }

  // Now keep processing until all reachable nodes have been processed
  while (!toProcess.empty()) {
    auto nodePosition = field.fromOffset(toProcess.front());
    toProcess.pop();

    // Expand the current node by checking all directions
    auto& currentNode = field[nodePosition];
    for (auto direction : Vector::AllSimpleDirections()) {
      auto nextPosition = nodePosition + direction;
      if (field.validPosition(nextPosition)) {
        // We can expand in that direction
        auto& nextNode = field[nextPosition];
        if (nextNode.height == currentNode.height-1) {
          // a valid trail continuation
          if (!nextNode.reachableTops.empty()) {
            // already expanded there by another node
            nextNode.reachableTops.insert(nextNode.reachableTops.end(), currentNode.reachableTops.begin(), currentNode.reachableTops.end());
          } else {
            // node not yet expanded and into processing queue
            nextNode.reachableTops = currentNode.reachableTops;
            toProcess.push(field.toOffset(nextPosition));
          }
        }
      }
    }
  }



  // Now we simply need to sum up the trailHeads values in all '0' nodes
  int allHeadSum = 0;
  int uniqueHeadSum = 0;
  for (auto& node : field.data) {
    if (node.height == '0') {
      // Now we can determine the reachable heads with duplicates and without.
      allHeadSum += node.reachableTops.size();
      uniqueHeadSum += (node.reachableTops | std::ranges::to<std::set>()).size();
    }
  }
  
  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part1: " << uniqueHeadSum << "\n";
  std::cout << "Part2: " << allHeadSum << "\n";
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
