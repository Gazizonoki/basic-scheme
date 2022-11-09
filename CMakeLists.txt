cmake_minimum_required(VERSION 3.13)

project(cpp-advanced-hse)

set(CMAKE_CXX_STANDARD 20)

add_library(scheme-lib object.cpp parser.cpp tokenizer.cpp scheme.cpp)
add_executable(scheme main.cpp)
target_link_libraries(scheme scheme-lib)

