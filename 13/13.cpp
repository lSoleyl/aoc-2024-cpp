#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <chrono>
#include <string>

#include <common/vector.hpp>

std::regex buttonRegex("^Button [AB]: X\\+([0-9]+), Y\\+([0-9]+)$");
std::regex prizeRegex("^Prize: X=([0-9]+), Y=([0-9]+)$");


struct Button : public VectorT<int64_t> {
  int cost;
};

struct Game {
  Button A, B; // A costs 3, B costs 1
  VectorT<int64_t> prize; 

  int64_t countCost(Vector position = Vector::Zero, int currentCost = 0, int currentButtons = 0) const {
    auto term1 = A.x * prize.y - A.y * prize.x;
    auto term2 = A.x * B.y - A.y * B.x;

    // by performing the integer division last we ensure that there is no hidden truncation mid way in the calculation
    auto b = term1 / term2;
    auto a = (prize.x - b * B.x) / A.x;

    return (a >= 0 && b >= 0 && a * A + b * B == prize) ? (a * A.cost + b * B.cost) : 0;
  }
};

struct Games : public std::vector<Game> {
  Games(std::istream&& stream) {
    std::string line;
    while (std::getline(stream, line)) {
      Game game;
      std::smatch results;
      std::regex_match(line, results, buttonRegex);
      game.A.x = std::stoi(results.str(1));
      game.A.y = std::stoi(results.str(2));
      game.A.cost = 3;
      
      std::getline(stream, line);
      std::regex_match(line, results, buttonRegex);
      game.B.x = std::stoi(results.str(1));
      game.B.y = std::stoi(results.str(2));
      game.B.cost = 1;

      std::getline(stream, line);
      std::regex_match(line, results, prizeRegex);
      game.prize.x = std::stoi(results.str(1));
      game.prize.y = std::stoi(results.str(2));
      push_back(game);

      std::getline(stream, line); // read empty follow line
    }
  }
};


int main()
{
  auto t1 = std::chrono::high_resolution_clock::now();
  Games games(std::ifstream("input.txt"));

  int64_t totalCoins = 0;
  int64_t correctedCoins = 0;
  // Part 1 & 2: 
  for (auto& game : games) {
    // Simply calculate the number of button presses... brute force seems to be actually more complicated than calculation
    totalCoins += game.countCost();
    // Correct the prize position for part 2
    game.prize += VectorT<int64_t>(10000000000000, 10000000000000);
    correctedCoins += game.countCost();
  }

  
  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part 1: " << totalCoins << "\n"; // 31589
  std::cout << "Part 2: " << correctedCoins << "\n"; // 98080815200063
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
