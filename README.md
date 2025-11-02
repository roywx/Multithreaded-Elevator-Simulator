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

## Assumptions & Known Limitations
Scheduling is basic and requests are handled in a FIFO order. Elevators may pass by a requested floor in some cases.

Requests are also handled as a single input (source floor and target floor) rather than two seperate inputs.

Elevators continue in their current direction until all requested floors in that direction are served.

Movement is simulated via a sleep call, travel time is also constant.

If all elevators are busy in the wrong direction, requests are requeued with a short delay (busy wait).

Lacks more complex features (eg. emergency call).

