#include <fstream>
#include <iostream>
#include <chrono>
#include <set>
#include <vector>
#include <ranges>
#include <regex>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <optional>

#include <common/stream.hpp>
#include <common/hash.hpp>

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

  Wire* getOtherInput(Wire* input) const { return input == inputA ? inputB : inputA; }

  Wire* inputA;
  Wire* inputB;
  Operation op;
  Wire* output; // used for easier navigation/modification in Part2
};

bool Wire::calcValue() { return input->getValue(); }

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

  /** A tag, which will be assigned to wires similar to symbols. They are derived from the gate 
   *  connections and the predefined input and output wires
   */
  struct Tag {
    friend std::ostream& operator<<(std::ostream&, Tag);

    enum Type : uint8_t {
      Invalid, // invalid tag returned by get for not set tag
      X, Y, // Input bit
      R, // r[i] = x[i] XOR y[i] - Only for i > 0
      Z, // z[0] = x[0] XOR y[0] - Output bit
      // z[i] = r[i] XOR c[i-1]
      S, // s[i] = x[i] AND y[i]
      T, // t[i] = r[i] AND c[i-1]
      C  // c[i] = s[i] OR t[i] - Carry bit
    };
    
    explicit operator bool() const { return type != Tag::Invalid; }
    bool operator==(const Tag& other) const = default;
    bool operator!=(const Tag& other) const = default;
    auto operator<=>(const Tag& other) const = default;

    Type type;
    uint8_t bit;
  };


  struct TagResult {
    TagResult(std::unordered_map<std::string/*id*/, Wire>& wireMap) {
      for (auto& [id, wire] : wireMap) {
        untagged.insert(&wire);
      }
    }

    std::map<Tag, Wire*> fromTag; // cannot use unordered, because we cannot define std::hash<Tag> before it is being needed
    std::unordered_map<Wire*, Tag> tags;
    std::unordered_set<Wire*> untagged;
    std::unordered_set<Wire*> conflictingTags; // Two different tags assigned to the same wire
    std::unordered_set<Wire*> duplicateTags; // Same tag assigned to multiple wires

    void set(Wire& wire, Tag::Type type, uint8_t bit) { set(&wire, Tag{ type, bit }); }
    void set(Wire* wire, Tag::Type type, uint8_t bit) { set(wire, Tag{ type, bit }); }
    void set(Wire& wire, Tag tag) { set(&wire, tag); }

    void set(Wire* wire, Tag tag) {
      auto pos = tags.find(wire);
      if (pos != tags.end()) {
        if (pos->second != tag) {
          // already tagged with a different tag
          bool inserted = conflictingTags.insert(wire).second;
          inserted |= conflictingTags.insert(pos->first).second;
          if (inserted) { // first time reporting this combo
            std::cout << wire->id << " duplicate tags: " << pos->second << " & " << tag << "\n";
          }
          return;
        }
        // Otherwise we are reassigning the same tag, which is allowed
      }

      auto rPos = fromTag.find(tag);
      if (rPos != fromTag.end()) {
        if (rPos->second != wire) {
          // tag already assigend to a different wire
          bool inserted = duplicateTags.insert(wire).second;
          inserted |= duplicateTags.insert(rPos->second).second;
          if (inserted) { // first time reporting this combination
            std::cout << wire->id << " and " << rPos->second->id << " both use same tag: " << tag << "\n";
          }
          return;
        }
        // Otherwise we are reassigning the same tag to the same wire, which is allowed
      }
        
      tags[wire] = tag;
      fromTag[tag] = wire;
    }

    Tag get(Wire& wire) const {
      auto pos = tags.find(&wire);
      return (pos != tags.end()) ? pos->second : Tag{ Tag::Invalid, 0 };
    }

    Wire* get(Tag tag) const {
      // A bit inefficient, but I can optimize it by adding a reverse map if necessary      
      auto pos = fromTag.find(tag);
      return (pos != fromTag.end()) ? pos->second : nullptr;
    }

    Wire* get(Tag::Type type, uint8_t bit) const {
      // A bit inefficient, but I can optimize it by adding a reverse map if necessary      
      auto pos = fromTag.find(Tag{type, bit});
      return (pos != fromTag.end()) ? pos->second : nullptr;
    }

  };

  /** Tag all wires by propagating information from the input through the gates to the output
   *  conflicting tags 
   * 
   *  Tagging is performed with the following processing scheme in mind: (special treatment for the first and last bit are needed)
   *   z00 = x00 XOR y00
   *   c00 = x00 AND y00
   *   
   *   r01 = x01 XOR y01
   *   z01 = c00 XOR r01
   *   s01 = x01 AND y01
   *   t01 = c00 AND r01
   *   c01 = s01 OR t01
   *   ...
   *   z41 = c40 = s40 OR t40
   */
  TagResult tagWires() {
    TagResult tags(wires);
    
    uint8_t nOutputBits = 0;
    uint8_t nInputBits = 0;

    // First tag input/output wires
    for (uint8_t bit = 0;; ++bit) {
      auto pos = wires.find(std::format("z{:02}", bit));
      if (pos == wires.end()) {
        nOutputBits = bit;
        nInputBits = bit - 1;
        break; // no more output bits
      }
      tags.set(pos->second, Tag::Z, bit);

      pos = wires.find(std::format("x{:02}", bit));
      if (pos != wires.end()) {
        tags.set(pos->second, Tag::X, bit);
      }
      
      pos = wires.find(std::format("y{:02}", bit));
      if (pos != wires.end()) {
        tags.set(pos->second, Tag::Y, bit);
      }
    }

    // To propagate all symbols in a single forward pass, we will construct a map, which 
    // maps each wire to the gates it is used as input in
    std::unordered_map<Wire*, std::vector<Gate*>> inputMap;
    for (auto& gate : gates) {
      inputMap[gate.inputA].push_back(&gate);
      inputMap[gate.inputB].push_back(&gate);
    }


    // Now propagate all inputs bit by bit
    // Special case for bit 0
    auto x00 = tags.get({ Tag::X, 0 });
    for (auto gate : inputMap[x00]) {
      if (gate->op == XOR) { // since inputs are never swapped, this must be the z00 = x00 XOR y00
        tags.set(gate->output, Tag::Z, 0);
      } else if (gate->op == AND) { // must be c00 = x00 AND y00
        tags.set(gate->output, Tag::C, 0);
      }
    }

    // Now propagate the tags bit by bit except for the last output bit, which needs special treatment
    for (uint8_t bit = 1; bit < nInputBits; ++bit) {
      // First R and S tags, which are trivial
      auto x01 = tags.get(Tag::X, bit);
      for (auto gate : inputMap[x01]) {
        if (gate->op == XOR) { // since inputs are never swapped, this must be the r[i] = x[i] XOR y[i]
          tags.set(gate->output, Tag::R, bit);
        } else if (gate->op == AND) { // s[i] = x[i] AND y[i]
          tags.set(gate->output, Tag::S, bit);
        }
      }

      // Now find z[i] and t[i]
      auto c00 = tags.get(Tag::C, bit - 1);
      auto r01 = tags.get(Tag::R, bit);

      for (auto wire : {c00, r01}) { // check through both wires, in case one of them is unknown (missing tag)
        if (wire) { 
          for (auto gate : inputMap[wire]) {
            if (gate->op == XOR) { // this should be z[i] = c[i-1] XOR r[i]
              tags.set(gate->output, Tag::Z, bit);
              // We should also tag the other input accordingly, otherwise we will miss some wrongly connected outputs
              tags.set(gate->getOtherInput(wire), (wire == c00) ? Tag{Tag::R, bit} : Tag{Tag::C, static_cast<uint8_t>(bit-1)});
            } else if (gate->op == AND) { // this shoud be t[i] = c[i-1] AND r[i] 
              tags.set(gate->output, Tag::T, bit);
              tags.set(gate->getOtherInput(wire), (wire == c00) ? Tag{ Tag::R, bit } : Tag{ Tag::C, static_cast<uint8_t>(bit - 1) });
            }
          }
        }
      }

      // Finally tag the carry
      auto s01 = tags.get(Tag::S, bit);
      auto t01 = tags.get(Tag::T, bit);

      for (auto wire : { s01, t01 }) { // check through both wires, in case one of them is unknown (missing tag)
        if (wire) {
          for (auto gate : inputMap[wire]) {
            if (gate->op == OR) { // this should be c[i] = s[i] OR t[i]
              if (bit == nInputBits - 1) {
                // For the last bit we must set z[i+1] = c[i] = s[i] OR t[i]
                tags.set(gate->output, Tag::Z, bit+1);
                tags.set(gate->getOtherInput(wire), (wire == s01) ? Tag{ Tag::T, bit } : Tag{ Tag::S, bit });
              } else {
                tags.set(gate->output, Tag::C, bit);
                tags.set(gate->getOtherInput(wire), (wire == s01) ? Tag{ Tag::T, bit } : Tag{ Tag::S, bit });
              }
            }
          }
        }
      }
    }

    return tags;
  }


  // Part 2: I thought, I would have to iteratively tagWires() and then, swap some of them until we 
  //         can tag all wires correctly, but the initial tagWires run simply returns all 8 wires, which are wrong... wow!
  // 
  std::set<std::string> fixAdder() {
    auto result = tagWires();
    auto wrongWires = result.conflictingTags | std::views::transform([](Wire* w) { return w->id; }) | std::ranges::to<std::set>();
    wrongWires.insert_range(result.duplicateTags | std::views::transform([](Wire* w) { return w->id; }) | std::ranges::to<std::set>());
    return wrongWires;
  }



  std::unordered_map<std::string/*id*/, Wire> wires;
  std::list<Gate> gates;
};

std::ostream& operator<<(std::ostream& out, Network::Tag tag) {
  switch (tag.type) {
    case Network::Tag::X: out << 'x'; break;
    case Network::Tag::Y: out << 'y'; break;
    case Network::Tag::R: out << 'r'; break;
    case Network::Tag::Z: out << 'z'; break;
    case Network::Tag::S: out << 's'; break;
    case Network::Tag::T: out << 't'; break;
    case Network::Tag::C: out << 'c'; break;
  }
  return out << std::setfill('0') << std::setw(2) << static_cast<int>(tag.bit);
}



int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  Network network(std::ifstream("input.txt"));
  auto output = network.calculateOutput();

  // Part 2: I didn't actually implement an algorithm to fix the adder automatically, which I still intend to do, but instead
  //         I normalized the wire names and gate input ordering, printed out the strucuture and then could (with some time) find,
  //         which wire has been swapped with which other wire.
  //         Interestingly the wires are never swapped between mutliple bits, which would make the search more complex, the swaps 
  //         always happen inside one single adder.
  auto wrongWires = network.fixAdder();

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "\n\n";
  std::cout << "Part 1: " << output << "\n"; // 51657025112326
  std::cout << "Part 2: " << stream::join(wrongWires) << "\n"; // gbf,hdt,jgt,mht,nbf,z05,z09,z30
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
