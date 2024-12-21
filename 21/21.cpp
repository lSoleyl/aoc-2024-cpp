#include <fstream>
#include <iostream>
#include <chrono>
#include <ranges>
#include <algorithm>
#include <numeric>
#include <vector>
#include <unordered_map>

#include <common/vector.hpp>
#include <common/hash.hpp>


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
using CostMap = std::unordered_map<std::pair<Vector/*from*/, Vector/*to*/>, int64_t/*buttonpresses*/>;


struct KeypadState {
  KeypadState(const Keypad& keypad) : keypad(keypad), forbidden(keypad.at(' ')) {
    // For the very first keypad each button is one button press
    for (auto& [fromButton, fromPos] : keypad) {
      if (fromButton != ' ') {
        for (auto& [toButton, toPos] : keypad) {
          if (toButton != ' ') {
            costMap.insert({ {fromPos, toPos}, 1});
          }
        }
      }
    }
  }

  KeypadState(const Keypad& keypad, const KeypadState& previousKeypad) : keypad(keypad), forbidden(keypad.at(' ')), prevKeypad(&previousKeypad) {
    deriveCostMap();
  }


  // This will build up the precalculated cost map for each button
  void deriveCostMap() {
    auto lowerLevelStartPos = prevKeypad->keypad.at('A');
    auto& lowerLevelCostMap = prevKeypad->costMap;

    for (auto& [fromKey, fromPos] : keypad) {
      if (fromKey != ' ') {
        for (auto& [toKey, toPos] : keypad) {
          if (toKey != ' ') {
            costMap.insert({{ fromPos, toPos }, getLowestPathCost(fromPos, toPos, lowerLevelStartPos)});
          }
        }
      }
    }
  }


  int64_t getLowestPathCost(Vector from, Vector to, Vector posLowerLevel) const {
    if (from == to) {
      // The costs are the costs to press the 'A' button on the lower level
      return prevKeypad->costMap.at({ posLowerLevel, prevKeypad->keypad.at('A') });
    }

    auto minCosts = std::numeric_limits<int64_t>::max();

    // Try to go in each direction, which reduces the step distance to the target
    //for (auto direction : Vector::AllDirections()) {
    for (auto direction : {Vector::Right, Vector::Up, Vector::Down, Vector::Left }) {
      auto nextPos = from + direction;
      if (nextPos.stepDistance(to) < from.stepDistance(to) && nextPos != forbidden) {
        // This step brings us closer to the target
        auto nextLowerLevelPos = prevKeypad->keypad.at(direction.toChar()); // where we move on the next lower level
        auto stepEntry = prevKeypad->costMap.at({posLowerLevel, nextLowerLevelPos }); // get cost move performing this move on the lower level
        auto costEntry = getLowestPathCost(nextPos, to, nextLowerLevelPos);
        if (stepEntry + costEntry < minCosts) {
          minCosts = stepEntry + costEntry;
        }
      }
    }

    return minCosts;
  }



  int64_t getSequenceCost(const std::string& sequence) const {

    int64_t buttonPresses = 0;

    auto position = keypad.at('A'); // we start input of each sequence with the robot arm at 'A'
    for (auto button : sequence) {
      auto buttonPos = keypad.at(button);
      buttonPresses += costMap.at({ position, buttonPos });
      position = buttonPos; // update position
    }

    // Calculate sequence cost as the numeric value times the number of button presses required
    return std::stoi(sequence) * buttonPresses;
  }



  const Keypad& keypad;
  const Vector forbidden;
  const KeypadState* prevKeypad = nullptr;
  CostMap costMap; // transitive button press cost map
};


struct RobotControl {
  RobotControl(std::istream&& input) : sequences(std::istream_iterator<std::string>(input), std::istream_iterator<std::string>()) {}

  auto countSequenceComplexities() const {
    
    // List of control pads in reverse order
    std::list<KeypadState> controlPads = { controls }; // user controlled
    controlPads.push_front({ controls, controlPads.front() }); // robot controlled
    controlPads.push_front({ controls, controlPads.front() }); // robot controlled
    
    KeypadState numberPad = { numbers, controlPads.front() }; // robot controlled (don't calculate the costs here)

    int64_t totalComplexity = 0;

    // Now go through each sequence
    for (auto& sequence : sequences) {
      totalComplexity += numberPad.getSequenceCost(sequence);
    }

    return totalComplexity;
  }

  auto countSequenceComplexities2() const {

    // List of control pads in reverse order
    std::list<KeypadState> controlPads = { controls }; // user controlled

    for (int i = 0; i < 25; ++i) {
      controlPads.push_front({ controls, controlPads.front() }); // robot controlled
    }
    
    KeypadState numberPad = { numbers, controlPads.front() }; // robot controlled (don't calculate the costs here)

    int64_t totalComplexity = 0;

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

  RobotControl control(std::ifstream("input.txt"));
  auto complexities = control.countSequenceComplexities();
  auto complexities2 = control.countSequenceComplexities2();

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << complexities << "\n"; // 270084
  std::cout << "Part 2: " << complexities2 << "\n"; // 329431019997766
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
