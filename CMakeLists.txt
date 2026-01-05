cmake_minimum_required(VERSION 3.10)
project(simplified_uninit LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# No external dependencies - pure standard library
add_executable(simplified_uninit simplified_uninit.cpp)
