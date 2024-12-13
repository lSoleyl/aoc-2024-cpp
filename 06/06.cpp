#include <iostream>
#include <fstream>
#include <unordered_set>
#include <algorithm>
#include <execution>
#include <chrono>

#include <common/field.hpp>
#include <common/hash.hpp>


template<bool storeVisited>
struct State {
  State(Field& field, const Vector& position, const Vector& direction) : field(&field), position(position), direction(direction), obstaclePos(-1,-1) {}
  State(const State<!storeVisited>& other) : field(other.field), position(other.position), direction(other.direction), obstaclePos(other.obstaclePos),
    visited(other.visited), visitedObstacles(other.visitedObstacles), inLoop(other.inLoop) {}

  enum class Result { LEFT_FIELD, IN_LOOP };
  enum class Action { STEP, ROTATE };


  Result run() {
    do {
      nextStep();
    } while (!leftField() && !stuckInLoop());

    return leftField() ? Result::LEFT_FIELD : Result::IN_LOOP;
  }

  
  void nextStep() {
    if constexpr (storeVisited) { 
      // only needed for first run -> so just don't do it for the obstacle checks as this is actually quite expensive
      visited.insert(position);
    }

    auto nextPosition = position + direction;
    if (field->validPosition(nextPosition) && ((*field)[nextPosition] == '#' || nextPosition == obstaclePos)) {
      // obstacle in front -> rotate
      if (!visitedObstacles.emplace(position, direction).second) {
        inLoop = true;
      }
      direction = direction.rotateCW();
    } else {
      // continue moving in that direction
      position = nextPosition;
    }
  }

  bool leftField() const { return !field->validPosition(position); }
  bool stuckInLoop() const { return inLoop; }

  Field* field;
  std::unordered_set<Vector> visited;
  std::unordered_set<std::pair<Vector/*pos*/, Vector/*direction*/>> visitedObstacles;
  Vector position;
  Vector direction;
  Vector obstaclePos; // additional obstacle (used to check multiple potential obstacles in parallel without modifying the field)
  bool inLoop = false;
};

int main() {
  auto t1 = std::chrono::high_resolution_clock::now();

  Field field(std::ifstream("input.txt"));
  
  auto startPosition = field.fromOffset(field.findOffset('^'));

  
  const State<false> startState(field, startPosition, Vector(0, -1)); // start direction is UP as rows increment downward
  State<true> firstRun(startState);
  firstRun.run();
  
  // Part2 : At each visited position [except for the start position] try adding an obstacle and check whether this causes a loop
  //         My first attempt at putting these obstacles in the way as we walk had the flaw that putting the obstacle late into the
  //         journey could prevent the guard from even reaching that point.
  //         This is pretty brute force, but I cannot think of a better solution for now
  std::unordered_set<Vector> possibleLoops;
  std::mutex m;


  // Parellelized solution as original takes 5.2 seconds, parallel one takes 1.1 seconds (or 780ms without debugger)
  // Reduced further down to 2,5s or 0,5s (parallel) by only storing the obstacle orientations for loop detection
  // Further optimized down to 195ms or 45ms (parallel) by not storing visited states at all when performing obstacle loop checks
  std::for_each(std::execution::par, firstRun.visited.begin(), firstRun.visited.end(), [&](Vector position) {
    if (position != startPosition) {
      State<false> state(startState);
      state.obstaclePos = position;
      if (state.run() == State<false>::Result::IN_LOOP) {
        std::lock_guard<std::mutex> guard(m); // synchronize access to set!
        possibleLoops.insert(position);
      }
    }
  });

  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part 1: " << firstRun.visited.size() << "\n"; // 4752
  std::cout << "Part 2: " << possibleLoops.size() << "\n"; // 1719

  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
