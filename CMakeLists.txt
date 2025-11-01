cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(elevator VERSION 1.0)

find_package(Boost REQUIRED COMPONENTS thread system)
include_directories(${Boost_INCLUDE_DIRS})


add_executable(elevator elevator.cpp)
target_link_libraries(elevator PRIVATE ${Boost_LIBRARIES})


