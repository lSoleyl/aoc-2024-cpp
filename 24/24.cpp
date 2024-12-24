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

struct Gate;

struct Wire final : public Component {
  Wire(std::string id, std::optional<bool> initialValue = {}) : Component(initialValue), id(std::move(id)) {}

  // The wire has the value of its input (which must be defined if the wire has no initial value)
  virtual bool calcValue() override;

  Gate* input = nullptr;
  std::string id;
};


using Operation = bool (*)(bool, bool);

Operation AND = [](bool a, bool b) { return a && b; };
Operation OR = [](bool a, bool b) { return a || b; };
Operation XOR = [](bool a, bool b) { return a != b; }; // a^b returns an int, not a bool

struct Gate final : public Component {
  Gate(Wire& inputA, Operation op, Wire& inputB, Wire& output) : inputA(&inputA), inputB(&inputB), op(op), output(&output) {}

  virtual bool calcValue() override { return op(inputA->getValue(), inputB->getValue()); }

  bool isInput(Wire& wire) const { return &wire == inputA || &wire == inputB; }
  void sortInputs() { if (inputA->id > inputB->id) { std::swap(inputA, inputB); } }

  const char* opId() const {
    if (op == AND) {
      return "AND";
    } else if (op == OR) {
      return "OR";
    } else if (op == XOR) {
      return "XOR";
    }
    return "???";
  }

  Wire* inputA;
  Wire* inputB;
  Operation op;
  Wire* output; // used for easier navigation/modification in Part2
};

bool Wire::calcValue() { return input->getValue(); }




std::unordered_set<std::string> printedDefinitions;

std::ostream& operator<<(std::ostream& out, const Gate& gate);
std::ostream& operator<<(std::ostream& out, const Wire& wire) {
  if (printedDefinitions.insert(wire.id).second && wire.input) {
    // first time printing definition
    // -> first print all the inputs (if not already done)
    out << *wire.input->inputA; // print definition of the input wires to the gate first
    out << *wire.input->inputB; // print definition of the input wires to the gate first
    // now print the definition for this wire
    out << wire.id << " = " << wire.input->inputA->id << ' ' << wire.input->opId() << ' ' << wire.input->inputB->id << "\n";
  }

  #if 0
  if (wire.input) {
    out << '(' << wire.id << " = " << *wire.input << ')';
  } else {
    out << wire.id;
  }
  #endif
  return out;
}


std::ostream& operator<<(std::ostream& out, const Gate& gate) {
  return out << gate.inputA << gate.opId() << gate.inputB;
}


const std::unordered_map<std::string/*id*/, Operation> opMap = {
  { "AND", AND },
  { "OR",  OR },
  { "XOR", XOR } 
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
      // For better debugability order the ids alphabetically in the inputs
      auto idA = std::min(gateMatch[1].str(), gateMatch[3].str());
      auto idB = std::max(gateMatch[1].str(), gateMatch[3].str());

      auto& inputA = wires.emplace(idA, idA).first->second;
      auto op = opMap.at(gateMatch[2].str());
      auto& inputB = wires.emplace(idB, idB).first->second;
      auto& output = wires.emplace(gateMatch[4].str(), gateMatch[4].str()).first->second;
      // Create the gate and set is as input of the output wire
      output.input = &gates.emplace_back(inputA, op, inputB, output);
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

  enum class Role { InputA, InputB, Carry, Output };

  void fixAdder() {

    


    // From printing it out we have following construction used:
    // zN = xN ^ yN ^ cN-1
    // cN = (yN AND xN) OR (cN-1 AND (yN XOR xN))
    
    // The network is constructed as follows: (special treatment for the first and last bit are needed)
    // z00 = x00 XOR y00
    // c00 = x00 AND y00
    // 
    // z01 = c00 XOR r01
    // r01 = x01 XOR y01
    // s01 = x01 AND y01
    // t01 = c00 AND r01
    // c01 = s01 OR t01



    // ...
    // z41 = c40 = s40 OR t40

    // We should be able to construct a normalized network (with the above naming scheme to find wrong connections)

    std::unordered_set<Wire*> rWires, sWires, tWires;

    // First find a## wires
    for (auto& gate : gates) {
      if (gate.op == XOR && gate.inputA->id[0] == 'x' && gate.inputB->id[0] == 'y') { // r## = x## XOR y##
        if (gate.inputA->id != "x00") {
          if (gate.output->id[0] != 'z') { // z00 = x00 XOR y00 (and don't rename outputs)
            gate.output->id = "r" + gate.inputA->id.substr(1, 2);
            rWires.insert(gate.output);
          } else {
            std::cout << "[ERR] found " << gate.output->id << " should be " << "r" << gate.inputA->id.substr(1, 2) << "\n";
          }
        }
      }
    }

    // Find p## and c00 wires
    for (auto& gate : gates) {
      if (gate.op == AND && gate.inputA->id[0] == 'x' && gate.inputB->id[0] == 'y') { // s## = x## AND y##
        if (gate.inputA->id == "x00") { // c00 = x00 AND y00
          gate.output->id = "c00";
        } else if (gate.output->id[0] != 'z') { // cannot rename outputs
          gate.output->id = "s" + gate.inputA->id.substr(1, 2);
          sWires.insert(gate.output);
        } else {
          std::cout << "[ERR] found " << gate.output->id << " should be " << "s" << gate.inputA->id.substr(1, 2) << "\n";
        }
      }
    }

    std::ranges::for_each(gates, [](Gate& gate) { gate.sortInputs(); });

    // Find q## wires
    for (auto& gate : gates) {
      // We need to check for r## on both positions as the inputs are sorted alphabetically and we don't know the
      // name of the other input
      if (gate.op == AND && rWires.contains(gate.inputA)) { // t## = c[n-1] AND r## (but c[n-1] is not yet known)
        if (gate.output->id[0] != 'z') { // cannot rename outputs
          gate.output->id = "t" + gate.inputA->id.substr(1, 2);
          tWires.insert(gate.output);
        } else {
          std::cout << "[ERR] found " << gate.output->id << " should be " << "t" << gate.inputA->id.substr(1, 2) << "\n";
        }
      } else if (gate.op == AND && rWires.contains(gate.inputB)) { // t## = c[n-1] AND r## (but c[n-1] is not yet known)
        if (gate.output->id[0] != 'z') { // cannot rename outputs
          gate.output->id = "t" + gate.inputB->id.substr(1, 2);
          tWires.insert(gate.output);
        } else {
          std::cout << "[ERR] found " << gate.output->id << " should be " << "t" << gate.inputA->id.substr(1, 2) << "\n";
        }
      }
    }

    std::ranges::for_each(gates, [](Gate& gate) { gate.sortInputs(); });

    // Finally find c## wires
    for (auto& gate : gates) {
      if (gate.op == OR && sWires.contains(gate.inputA) && tWires.contains(gate.inputB)) { // c## = s## OR t##
        if (gate.output->id[0] != 'z') { // cannot rename output
          if (gate.inputA->id.substr(1,2) == gate.inputB->id.substr(1, 2)) { // ids should match
            gate.output->id = "c" + gate.inputA->id.substr(1, 2);
          } else {
            std::cout << "[ERR] found " << gate.output->id << " should be " << "c" << gate.inputA->id.substr(1, 2) << ", but input ids differ!\n";
          }
        } else {
          std::cout << "[ERR] found " << gate.output->id << " should be " << "c" << gate.inputA->id.substr(1, 2) << "\n";
        }
      }
    }

    std::cout << "\n\n";

    std::ranges::for_each(gates, [](Gate& gate) { gate.sortInputs(); });

    // first print out the network as it is
    for (int bit = 0;; ++bit) {
      auto pos = wires.find(std::format("z{:02}", bit));
      if (pos == wires.end()) {
        break; // no more output bits
      }

      auto& wire = pos->second;
      std::cout << wire;
    }

    std::cout << "\n\n";
    // print renaming map
    for (auto& [id, wire] : wires) {
      std::cout << id << " -> " << wire.id << "\n";
    }


  }



  std::unordered_map<std::string/*id*/, Wire> wires;
  std::list<Gate> gates;
};



int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  Network network(std::ifstream("input.txt"));
  auto output = network.calculateOutput();

  // Part 2: I didn't actually implement an algorithm to fix the adder automatically, which I still intend to do, but instead
  //         I normalized the wire names and gate input ordering, printed out the strucuture and then could (with some time) find,
  //         which wire has been swapped with which other wire.
  //         Interestingly the wires are never swapped between mutliple bits, which would make the search more complex, the swaps 
  //         always happen inside one single adder.
  network.fixAdder();

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Part 1: " << output << "\n"; // 51657025112326
  std::cout << "Part 2: " << "gbf,hdt,jgt,mht,nbf,z05,z09,z30" << "\n"; // gbf,hdt,jgt,mht,nbf,z05,z09,z30
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
