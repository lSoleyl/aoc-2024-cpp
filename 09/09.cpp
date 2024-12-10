#include <iostream>
#include <fstream>
#include <ranges>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <algorithm>
#include <string_view>
#include <execution>
#include <chrono>

struct Block {
  Block() : id(Block::FREE), size(0) {}
  Block(int id, int size) : id(id), size(size) {}

  int id; // max id is 9999, so we just as well use an int16_t
  int size;
  
  static const int FREE = -1;

  bool isFree() const { return id == Block::FREE;  }
};



auto nextFreeBlock(std::list<Block>::iterator pos, std::list<Block>::iterator end) {
  if (pos == end) {
    return end;
  }

  if (pos->isFree()) {
    return pos; // current position is at a free block
  }

  return std::find_if(pos, end, [](const Block& block) { return block.isFree(); });
}



int main()
{
  auto t1 = std::chrono::high_resolution_clock::now();

  std::list<Block> blocks;
  {
    int nextBlockId = 0;
    bool freeBlock = false;
    std::ifstream file("input.txt");
    for (char sizeCh; file >> sizeCh; freeBlock = !freeBlock) {
      int blockSize = sizeCh - '0'; // 0-9
      if (freeBlock) {
        if (blockSize > 0) { // don't insert empty blocks
          if (blocks.back().isFree()) {
            // Simply grow the last free block to avoid the overhead of merging them later 
            blocks.back().size += blockSize;
          } else {
            blocks.emplace_back(Block::FREE, blockSize);
          }
        }
      } else {
        if (blockSize > 0) { // don't insert empty blocks
          blocks.emplace_back(nextBlockId, blockSize);
        }
        ++nextBlockId;
      }
    }
  }


  auto blocksCopy = blocks; // copy instead of re-reading for part2


  // Part 1: 
  // Now compact the data by finding the first free block and fill it with the data from the last data block
  auto freeBlockPos = blocks.begin();

  while ((freeBlockPos = nextFreeBlock(freeBlockPos, blocks.end())) != blocks.end()) {
    auto dataBlockPos = --(blocks.end());
    while (dataBlockPos->isFree()) {
      // delete trailing free blocks
      auto eraseBlockPos = dataBlockPos;
      --dataBlockPos;
      blocks.erase(eraseBlockPos);
    }

    auto movedData = std::min(freeBlockPos->size, dataBlockPos->size);
    if (movedData < freeBlockPos->size) {
      // We must insert a new data block before the free block. The current data block is completely moved.
      freeBlockPos->size -= movedData;
      blocks.insert(freeBlockPos, *dataBlockPos);
      blocks.erase(dataBlockPos);
    } else {
      // Data is partially or completely moved, the free block is completely consumed by the data
      // We only need to copy id, as the size of the free block cannot grow here
      freeBlockPos->id = dataBlockPos->id; 
      if (dataBlockPos->size > movedData) {
        // data block shrinks by the moved data
        dataBlockPos->size -= movedData;
      } else {
        // data block has been completely moved
        blocks.erase(dataBlockPos);
      }
    }
  }

  // Now count up the checksum
  int64_t checksum = 0;
  int offset = 0;
  for (auto& block : blocks) {
    // There are no free blocks left in the list, so no problem here
    for (auto endOffset = offset + block.size; offset != endOffset; ++offset) {
      // idk why the number is considered huge... it easily fits into 64-Bit int
      checksum += offset * block.id;
    }
  }









  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part1: " << checksum << "\n"; // 6216544403458
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
