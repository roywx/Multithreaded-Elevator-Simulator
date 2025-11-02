# Multithreaded-Elevator-Simulator

## Overview 
This is a basic multi elevator simulator that handles user input. It is written in C++ and uses BOOST for threading. 

*This project was created as a take-home assessment*

## How to Build
This project uses **CMake** and requires Boost

**macOS 
brew install boost

git clone https://github.com/roywx/Multithreaded-Elevator-Simulator
cd elevator
cmake . && make && ./elevator 

## How to interact

After building run the executable: ./elevator

User requests are in the format <source_floor><target_floor>
