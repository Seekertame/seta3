cmake_minimum_required(VERSION 3.28)
project(set5_a3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(set5_a3
        main.cpp
        infrastructure/RandomStreamGen.cpp
        infrastructure/HashFuncGen.cpp
        hll/HyperLogLog.cpp
)

target_include_directories(set5_a3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
