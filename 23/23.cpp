#include <fstream>
#include <iostream>
#include <chrono>
#include <set>
#include <vector>
#include <ranges>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <common/stream.hpp>

static const int COMPUTERS_PER_SET = 3;

struct Computer {
  Computer(std::string id) : id(std::move(id)) {}

  void collectGroups(std::set<std::set<Computer*>>& foundGroups, std::set<Computer*> currentGroup) {
    for (auto connection : connections) {
      // only follow the connection if it is not part of the current set of computers
      // AND all computers from the current group have a connection to that computer
      if (!currentGroup.contains(connection) && std::ranges::all_of(currentGroup, [=](Computer* other) { return other->connections.contains(connection); })) {
        auto extendedGroup = currentGroup;
        extendedGroup.insert(connection);
        if (extendedGroup.size() == COMPUTERS_PER_SET) {
          // found the target set size -> enter into result set
          foundGroups.emplace(std::move(extendedGroup));
        } else {
          // find next connection starting from the connected to computer
          connection->collectGroups(foundGroups, extendedGroup);
        }
      }
    }
  }

 
  std::string id;
  std::set<Computer*> connections;
};


struct Network {
  Network(std::istream&& input) {
    for (auto line : stream::lines(input)) {
      // Create computers if not already done
      auto& computerA = get(line.substr(0, 2));
      auto& computerB = get(line.substr(3, 2));
      // Connect both nodes
      computerA.connections.insert(&computerB);
      computerB.connections.insert(&computerA);
    }
  }

  Computer& get(const std::string& id) {
    auto pos = computers.find(id);
    if (pos == computers.end()) {
      pos = computers.emplace(id, id).first;
    }
    return pos->second;
  }

  int countComputerGroups() {
    std::set<std::set<Computer*>> foundGroups;

    for (auto& [id, computer] : computers) {
      if (id[0] == 't') { // only collect nodes starting with 't'
        computer.collectGroups(foundGroups, { &computer });
      }
    }

    return foundGroups.size();
  }





  std::unordered_map<std::string/*id*/, Computer> computers;
};




int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  Network network(std::ifstream("input.txt"));

  auto groupCount = network.countComputerGroups();



  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << groupCount << "\n"; // 1253
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
