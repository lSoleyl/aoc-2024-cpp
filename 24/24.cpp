#include <fstream>
#include <iostream>
#include <chrono>
#include <set>
#include <vector>
#include <ranges>
#include <regex>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <optional>

#include <common/stream.hpp>

struct Component {
  Component(std::optional<bool> value = {}) : value(value) {}
  bool getValue() {
    // Calculate the value of each component at most once
    if (value.has_value()) {
      return *value;
    }

    value = calcValue();
    return *value;
  }

protected:
  virtual bool calcValue() = 0;
  std::optional<bool> value;
};


struct Wire : public Component {
  Wire(std::string id, std::optional<bool> initialValue = {}) : Component(initialValue), id(std::move(id)) {}

  // The wire has the value of its input (which must be defined if the wire has no initial value)
  virtual bool calcValue() override { return input->getValue(); }

  Component* input = nullptr;
  std::string id;
};


using Operation = bool (*)(bool, bool);

struct Gate : public Component {
  Gate(Component& inputA, Operation op, Component& inputB) : inputA(inputA), inputB(inputB), op(op) {}

  virtual bool calcValue() override { return op(inputA.getValue(), inputB.getValue()); }

  Component& inputA;
  Component& inputB;
  Operation op;
};


const std::unordered_map<std::string/*id*/, Operation> opMap = {
  { "AND", [](bool a, bool b) { return a && b; } },
  { "OR",  [](bool a, bool b) { return a || b; } },
  { "XOR", [](bool a, bool b) { return a != b; } } // a^b returns an int, not a bool
};

std::regex wireRegex("^([a-z0-9]+): (0|1)$");
std::regex gateRegex("^([a-z0-9]+) (AND|OR|XOR) ([a-z0-9]+) -> ([a-z0-9]+)$");

struct Network {
  Network(std::istream&& input) {
    // input wires
    for (auto line : stream::lines(input)) {
      if (line.empty()) {
        break; // end of input wires
      }

      std::smatch wireMatch;
      std::regex_match(line, wireMatch, wireRegex);
      auto id = wireMatch[1].str();
      wires.emplace(id, Wire{id, wireMatch[2].str() == "1"}); // create wire with initial value
    }

    // gates and intermediate wires
    for (auto line : stream::lines(input)) {
      if (line.empty()) {
        break; // end of gates
      }

      std::smatch gateMatch;
      std::regex_match(line, gateMatch, gateRegex);
      auto& inputA = wires.emplace(gateMatch[1].str(), gateMatch[1].str()).first->second;
      auto op = opMap.at(gateMatch[2].str());
      auto& inputB = wires.emplace(gateMatch[3].str(), gateMatch[3].str()).first->second;
      auto& output = wires.emplace(gateMatch[4].str(), gateMatch[4].str()).first->second;
      // Create the gate and set is as input of the output wire
      output.input = &gates.emplace_back(inputA, op, inputB);
    }
  }


  int64_t calculateOutput() {
    int64_t result = 0;
    for (int bit = 0;; ++bit) {
      auto pos = wires.find(std::format("z{:02}", bit));
      if (pos == wires.end()) { 
        break; // no more output bits
      }

      auto& wire = pos->second;
      result |= static_cast<int64_t>(wire.getValue()) << bit;
    }

    return result;
  }



  std::unordered_map<std::string/*id*/, Wire> wires;
  std::list<Gate> gates;
};



int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  Network network(std::ifstream("input.txt"));
  auto output = network.calculateOutput();

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << output << "\n"; // 51657025112326
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
