#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <ranges>

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


  // Part 2 optimized - will be compiled directly into assembler code
  void fastRun(int A) {
    int B = 0, C = 0;
    output.clear();

    // disassembled code
    do {
      B = A % 8;			    // 2 4
      B = B ^ 1;			    // 1 1
      C = A / (1 << B);		// 7 5
      B = B ^ 5;			    // 1 5
      A = A / (1 << 3);		// 0 3
      B = B ^ C;			    // 4 3
      output.push_back(B % 8);	// 5 5
    } while (A && output.size() <= code.size()); // 0 0
  }


  // Part 2 with my own optimizations
  bool fastRunOpti(int64_t A) {
    int64_t B = 0, C = 0;

    auto codePos = code.begin();
    auto codeEnd = code.end();

    // disassembled code
    do {
      B = A & 0x7;  	    // 2 4
      B = B ^ 0x1;	      // 1 1 = (A & 0x7) ^ 0x1
      C = A >> B;	       	// 7 5 = A >> ((A & 0x7) ^ 0x1)
      B = B ^ 0x5;		    // 1 5 = (A & 0x7) ^ 0x4
      A = A >> 3;	       	// 0 3 
      B = B ^ C;			    // 4 3 = ((A & 0x7) ^ 0x4) ^ (A >> ((A & 0x7) ^ 0x1))
      if (*(codePos++) != (B & 0x7)) {
        return false; // output/code mismatch
      }
    } while (A && codePos != codeEnd); // 0 0

    return A == 0; // otherwise the output is longer
  }


  // Part 2 actually working solution of simply searching the required bits 
  //        by knowing what the output should be and assembling the number in 3-bit blocks
  int64_t reverseSearchHelp(int* pos, int* end, int64_t currentA) {
    if (pos == end) {
      return currentA;
    }

    // Try out all possible values for X = A & 0x7 (lower 3 bits)
    // We have output = ((X ^ 0x4) ^ (A >> (X ^ 0x1))) & 0x7
    for (int X = 0; X < 8; ++X) {
      // The question is now: which A can satisfy the following condition:
      // output = ((X ^ 0x4) ^ (A >> (X ^ 0x1))) & 0x7
      //        = ((X ^ 0x4) & 0x7)  ^  ((A >> (X ^ 0x1)) & 0x7)
      //        =  (X ^ 0x4) ^ ((A >> (X ^ 0x1)) & 0x7)
      // output ^ X ^ 0x4 = (A >> (X ^ 0x1)) & 0x7
      // requiredAX = (A >> (X ^ 0x1)) & 0x7
      auto requiredAX = *pos ^ X ^ 0x4;
      int64_t A = (currentA << 3 | X);
      
      // only follow the path if the output matches
      if (((A >> (X^0x1)) & 0x7) == requiredAX) {
        if (auto result = reverseSearchHelp(pos + 1, end, A)) {
          return result;
        }
      }
    }

    return 0; // not found
  }



  int64_t reverseSearch() {
    auto reverseCode = code;
    std::reverse(reverseCode.begin(), reverseCode.end());
    return reverseSearchHelp(reverseCode.data(), reverseCode.data() + reverseCode.size(), 0);
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

  // Part 1:
  ProgramState program(std::ifstream("input.txt"));
  auto registerCopy = program.reg;
  program.run();
  auto part1Output = program.output;


  // Part 2: 
  // Part 1 finished after 72 steps, so lets limit the program to 1000 steps and simply try all values

#if BRUTE_FORCE
  // The optimized brute force solution can check ~300.000.000 register values per second,
  // but this is still nowehere fast enough to find the solution in a reasonable time.
  // The value is where I got after letting this run for 1,5h, so a reasonable time estimate 
  // would be 150h of runtime to find the value.
  int64_t regAValue = 1460300000000; 
  int64_t printCounter = 0;
  while (program.output != program.code) {
    ++regAValue;
    ++printCounter;
    if (regAValue < 0) {
      throw std::exception("not found!");
    }

    if (program.fastRunOpti(regAValue)) {
      break; // found the value
    }

    if (printCounter == 100000000) {
      std::cout << regAValue << "\n";
      printCounter = 0;
    }
  }
#endif

  // Simply performing a recursive search by assembling the bits from the output in 
  // reverse order is actually the only way to solve this in a reasonable time
  auto A = program.reverseSearch();


  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part 1: " << part1Output << "\n"; // 4,1,5,3,1,5,3,5,7
  std::cout << "Part 2: " << A << "\n"; // 164542125272765
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
