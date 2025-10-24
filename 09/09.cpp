#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <algorithm>
#include <chrono>

#include <common/task.hpp>

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
    auto file = task::input();
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


  // Part 2: For the second part we need to invert the iteration process
  //         We start by finding the last not yet processed file block and then search for a free space to insert it into
  blocks = std::move(blocksCopy);
  
  // By terminating on blocks.begin() we may attempt to move already moved blocks, but since each block is moved to the first fitting position
  // there will be no free block where we could move the already moved block into.
  for (auto nextDataBlockPos = --(blocks.end()); nextDataBlockPos != blocks.begin(); --nextDataBlockPos) {
    auto& block = *nextDataBlockPos;
    if (!block.isFree()) {
      // skip free blocks, here we cannot delete them, because there is no guarantee that we would actually use them
      auto blockSize = nextDataBlockPos->size;
      auto freeBlockPos = std::find_if(blocks.begin(), nextDataBlockPos, [=](const Block& block) { return block.isFree() && block.size >= blockSize; });
      if (freeBlockPos != nextDataBlockPos) {
        // we can actually move the whole file
        if (freeBlockPos->size > blockSize) {
          // we must insert the data block before this free block and reduce the free block's size
          blocks.insert(freeBlockPos, *nextDataBlockPos);
          freeBlockPos->size -= blockSize;
        } else {
          // Free block size matches exactly
          *freeBlockPos = *nextDataBlockPos;
        }

        // we DO NOT erase the just moved block because there may be data following it and this would lead to wrong offsets
        nextDataBlockPos->id = Block::FREE; // the size doesn't change
      }
    }

    // continue with next block
  }


  // Now count up the checksum
  int64_t checksum2 = 0;
  offset = 0;
  for (auto& block : blocks) {
    // We must handle free blocks correctly
    if (block.isFree()) {
      offset += block.size;
    } else {
      for (auto endOffset = offset + block.size; offset != endOffset; ++offset) {
        // idk why the number is considered huge... it easily fits into 64-Bit int
        checksum2 += offset * block.id;
      }
    }
  }




  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout << "Part1: " << checksum << "\n";
  std::cout << "Part2: " << checksum2 << "\n";
  std::cout << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
}
