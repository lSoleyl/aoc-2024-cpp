#include <iostream>
#include <fstream>
#include <list>
#include <chrono>
#include <ranges>
#include <atomic>
#include <unordered_map>

#include <common/field.hpp>

int main() {
  auto t1 = std::chrono::high_resolution_clock::now();
  


  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
