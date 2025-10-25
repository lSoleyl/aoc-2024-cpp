# Advent of Code 2024 (C++)

This repository contains my C++ solutions to the Advent of Code 2024 challenges.
Since inputs/outputs should not be shared, they are in a password encrypted zip. 

If you want to try these solutions with your own inputs then you will need Visual Studio 2022 or newer and place your inputs into:
 * `/data/01/input.txt`
 * `/data/02/input.txt`
 * ...

Since the repository uses a common headers submodule, either clone it using:

    git clone --recurse-submodules https://github.com/lSoleyl/aoc-2024-cpp.git

or first clone and then initialize the submodule:

    git clone https://github.com/lSoleyl/aoc-2024-cpp.git
    cd aoc-2024-cpp
    git submodule update --init
