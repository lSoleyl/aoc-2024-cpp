#include <fstream>
#include <iostream>
#include <chrono>
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


// We can statically precalculate the cost of moving the cursor from a given position `from` to any
// other position on the number pad `to` and then pressing 'A'. All we need to calculate that is the cost map of
// the next lower level, because after entering any button on the top level (the number pad) all lower level pads
// MUST be positioned at the 'A' button.
using CostMap = std::unordered_map<std::pair<Vector/*from*/, Vector/*to*/>, int64_t/*buttonpresses*/>;


struct KeypadState {
  KeypadState(const Keypad& keypad) : keypad(keypad), forbidden(keypad.at(' ')) {
    // For the very first keypad each button is one button press
    for (auto& [fromButton, fromPos] : keypad) {
      if (fromButton != ' ') {
        for (auto& [toButton, toPos] : keypad) {
          if (toButton != ' ') {
            // pressing any button on the very first (user controlled) control panel has a cost of 1 button press
            costMap.insert({ {fromPos, toPos}, 1}); 
          }
        }
      }
    }
  }

  KeypadState(const Keypad& keypad, const KeypadState& previousKeypad) : keypad(keypad), forbidden(keypad.at(' ')), prevKeypad(&previousKeypad) {
    deriveCostMap();
  }


  // This will build up the precalculated cost map for each combination of button positions (from,to)
  void deriveCostMap() {
    // The lower level input always starts at the 'A' button
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
    for (auto direction : Vector::AllSimpleDirections()) {
      auto nextPos = from + direction;
      if (nextPos.stepDistance(to) < from.stepDistance(to) && nextPos != forbidden) {
        // This step brings us closer to the target
        auto nextLowerLevelPos = prevKeypad->keypad.at(direction.toChar()); // where we move on the next lower level
        auto stepCost = prevKeypad->costMap.at({posLowerLevel, nextLowerLevelPos }); // get cost of performing this move on the lower level
        auto restPathCost = getLowestPathCost(nextPos, to, nextLowerLevelPos); // cost of the rest of the path to 'to'
        minCosts = std::min(minCosts, stepCost + restPathCost);
      }
    }

    return minCosts;
  }



  int64_t getSequenceCost(const std::string& sequence) const {
    int64_t buttonPresses = 0;

    auto position = keypad.at('A'); // we start input of each sequence with the robot arm at 'A'
    for (auto button : sequence) {
      auto buttonPos = keypad.at(button);
      buttonPresses += costMap.at({ position, buttonPos }); // add costs to get from position to buttonPos
      position = buttonPos; // update position
    }

    // Calculate sequence cost as the numeric value times the number of button presses required (std::stoi() will correctly ignore the trailing 'A')
    return std::stoi(sequence) * buttonPresses;
  }



  const Keypad& keypad;
  const Vector forbidden;
  const KeypadState* prevKeypad = nullptr;
  CostMap costMap; // transitive button press cost map
};


struct RobotControl {
  RobotControl(std::istream&& input) : sequences(std::istream_iterator<std::string>(input), std::istream_iterator<std::string>()) {}

  auto countSequenceComplexities(int directionalRobotKeypads) const {

    // List of control pads in reverse order
    std::list<KeypadState> controlPads = { controls }; // user controlled

    for (int i = 0; i < directionalRobotKeypads; ++i) {
      controlPads.push_front({ controls, controlPads.front() }); // robot controlled
    }
    
    KeypadState numberPad = { numbers, controlPads.front() }; // robot controlled number pad

    // Now go through each sequence and add up the complexity costs
    int64_t totalComplexity = 0;
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
  auto complexities = control.countSequenceComplexities(2);
  auto complexities2 = control.countSequenceComplexities(25);

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << complexities << "\n"; // 270084
  std::cout << "Part 2: " << complexities2 << "\n"; // 329431019997766
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
