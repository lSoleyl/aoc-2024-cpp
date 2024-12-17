// 17.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

struct ProgramState {
  ProgramState(std::istream&& input) {
    std::string dummy;
    input >> dummy >> dummy >> reg.A;
    input >> dummy >> dummy >> reg.B;
    input >> dummy >> dummy >> reg.C;
    input >> dummy;

    int number;
    char separator;
    input >> number;
    code.push_back(number);
    while (input >> separator >> number) {
      code.push_back(number);
    }
  }

  int run(int maxSteps = std::numeric_limits<int>::max()) {
    output.clear();
    reg.IP = 0;

    int step = 0;
    while (this->step() && step < maxSteps) {
      ++step;
    }
    return step;
  }


  /** Return false if end of programm has been reached
   */
  bool step() {
    if ((reg.IP+1) >= code.size()) {
      return false;
    }

    auto instruction = code[reg.IP++];
    auto operand = code[reg.IP++];

    switch (instruction) {
      case 0: adv(operand); break;
      case 1: bxl(operand); break;
      case 2: bst(operand); break;
      case 3: jnz(operand); break;
      case 4: bxc(operand); break;
      case 5: out(operand); break;
      case 6: bdv(operand); break;
      case 7: cdv(operand); break;
    }

    return true;
  }


  void adv(int operand) {
    reg.A = reg.A / (1 << combo(operand));
  }

  void bdv(int operand) {
    reg.B = reg.A / (1 << combo(operand));
  }

  void cdv(int operand) {
    reg.C = reg.A / (1 << combo(operand));
  }

  void bxl(int operand) {
    reg.B ^= operand;
  }

  void bxc(int operand) {
    reg.B ^= reg.C;
  }

  void bst(int operand) {
    reg.B = combo(operand) % 8;
  }

  void jnz(int operand) {
    if (reg.A) {
      reg.IP = operand;
    }
  }

  void out(int operand) {
    output.push_back(combo(operand) % 8);
  }

  // resolves a combo operand to a literal one
  int combo(int operand) const {
    if (operand <= 3) {
      return operand;
    }
    switch (operand) {
      case 4: return reg.A;
      case 5: return reg.B;
      case 6: return reg.C;
    }
    throw std::exception("malformed program!");
  }



  struct Register {
    int A = 0, B = 0, C = 0, IP = 0;
  } reg;

  std::vector<int> code;
  std::vector<int> output;
};

std::ostream& operator<<(std::ostream& out, const std::vector<int>& vec) {
  for (auto it = vec.begin(), end = vec.end(); it != end; ) {
    out << *it;

    if (++it != end) {
      out << ',';
    }
  }
  return out;
}



int main()
{
  auto t1 = std::chrono::high_resolution_clock::now();
  ProgramState program(std::ifstream("input.txt"));
  program.run();
  auto part1Output = program.output;


  





  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part 1: " << part1Output << "\n"; // 4,1,5,3,1,5,3,5,7
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
