#include <fstream>
#include <iostream>
#include <chrono>
#include <ranges>
#include <algorithm>
#include <numeric>
#include <vector>
#include <unordered_map>

#include <common/vector.hpp>



using Keypad = std::unordered_map<char/*key*/, Vector/*pos*/>;

Keypad numbers = {
  { '7', Vector(0,0) }, { '8', Vector(1,0) }, { '9', Vector(2,0) },
  { '4', Vector(0,1) }, { '5', Vector(1,1) }, { '6', Vector(2,1) },
  { '1', Vector(0,2) }, { '2', Vector(1,2) }, { '3', Vector(2,2) },
  { ' ', Vector(0,3) }, { '0', Vector(1,3) }, { 'A', Vector(2,3)}
};

Keypad controls = {
  { ' ', Vector(0,0) }, { '^', Vector(1,0) }, {'A', Vector(2,0)},
  { '<', Vector(0,1) }, { 'v', Vector(1,1) }, { '>', Vector(2,1) }
};


// We can statically precalculate the cost of pressing a button in a later number pad, because
// we can use the fact that the earlier pads will always confirm each press with an 'A' and thus start and end at an 'A'
using CostMap = std::unordered_map<char/*button*/, std::pair<int/*buttonPresses*/, std::string/*buttonSequence*/>>;





struct KeypadState {
  KeypadState(const Keypad& keypad) : keypad(keypad), forbidden(keypad.at(' ')) {
    // For the very first keypad each button is one button press
    for (auto& [button, pos] : keypad) {
      if (button != ' ') {
        costMap.insert({ button, { 1, std::string(1, button) }});
        inverseKeypad.insert({ pos, button });
      }
    }
  }

  KeypadState(const Keypad& keypad, const KeypadState& previousKeypad) : keypad(keypad), forbidden(keypad.at(' ')), prevKeypad(&previousKeypad) {
    deriveCostMap(previousKeypad.costMap);
    for (auto& [button, pos] : keypad) {
      if (button != ' ') {
        inverseKeypad.insert({ pos, button });
      }
    }
  }


  // This will build up the precalculated cost map for each button
  void deriveCostMap(const CostMap& earlierKeypadCostMap) {
    const auto startPosition = keypad.at('A');
    for (auto& [key, position] : keypad) {
      if (key == 'A') {
        // For the start button we don't need to navigate there and back, so it is only the press itself
        costMap[key] = earlierKeypadCostMap.at('A');

      } else if (key != ' ') {
        // We need to navigate to the position, confirm it by pressing 'A', then navigate back to 'A' and confirm it by pressing 'A'
        auto part1 = getLowestPathCost(startPosition, position, earlierKeypadCostMap);
        // Now we need to navigate back to the 'A' button to confirm this button press
        auto part2 = getLowestPathCost(position, startPosition, earlierKeypadCostMap);
        auto aCost = earlierKeypadCostMap.at('A');
        costMap[key] = { part1.first + aCost.first + part2.first + aCost.first, part1.second + aCost.second + part2.second + aCost.second };
      }
    }
  }


  std::pair<int, std::string> getLowestPathCost(Vector from, Vector to, const CostMap& costs) const {
    if (from == to) {
      return { 0, "" }; // on the last level we don't need to press 'A', so don't include it here, we will include it into the cost map though
    }

    std::pair<int, std::string> minCosts = { std::numeric_limits<int>::max(), "???" };

    // Try to go in each direction, which reduces the step distance to the target
    //for (auto direction : Vector::AllDirections()) {
    for (auto direction : {Vector::Right, Vector::Up, Vector::Down, Vector::Left }) {
      auto nextPos = from + direction;
      if (nextPos.stepDistance(to) < from.stepDistance(to) && nextPos != forbidden) {
        // This step brings us closer to the target
        auto stepEntry = costs.at(direction.toChar());
        auto costEntry = getLowestPathCost(nextPos, to, costs);
        if (stepEntry.first + costEntry.first < minCosts.first) {
          minCosts = { stepEntry.first + costEntry.first, stepEntry.second + costEntry.second };
        }
      }
    }

    return minCosts;
  }



  int getSequenceCost(const std::string& sequence) const {

    int buttonPresses = 0;

    std::cout << sequence << " : ";

    auto position = keypad.at('A'); // we start input of each sequence with the robot arm at 'A'
    for (auto button : sequence) {
      auto buttonPos = keypad.at(button);

      auto costEntry = getLowestPathCost(position, buttonPos, costMap);
      std::cout << costEntry.second << ' ';

      buttonPresses += costEntry.first;
      position = buttonPos; // update position
    }

    std::cout << "\n";

    // Calculate sequence cost as the numeric value times the number of button presses required
    return std::stoi(sequence) * buttonPresses;
  }



  const Keypad& keypad;
  std::unordered_map<Vector/*pos*/, char/*button*/> inverseKeypad;
  const Vector forbidden;
  const KeypadState* prevKeypad = nullptr;
  CostMap costMap; // transitive button press cost map
};


struct RobotControl {
  RobotControl(std::istream&& input) : sequences(std::istream_iterator<std::string>(input), std::istream_iterator<std::string>()) {}

  int countSequenceComplexities() const {
    
    // List of control pads in reverse order
    std::list<KeypadState> controlPads = { controls }; // user controlled
    controlPads.push_front({ controls, controlPads.front() }); // robot controlled
    //controlPads.push_front({ controls, controlPads.front() }); // robot controlled
    
    KeypadState numberPad = { numbers }; // robot controlled (don't calculate the costs here)
    numberPad.costMap = controlPads.front().costMap; // the number pad uses the cost map of the last control pad


    int totalComplexity = 0;

    // Now go through each sequence
    for (auto& sequence : sequences) {
      totalComplexity += numberPad.getSequenceCost(sequence);
    }

    return totalComplexity;
  }

  std::vector<std::string> sequences;
};


int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  RobotControl control(std::ifstream("sample.txt"));
  auto complexities = control.countSequenceComplexities();

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << complexities << "\n";
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
