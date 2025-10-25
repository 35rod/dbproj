cmake_minimum_required(VERSION 3.10)
project(DB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(main src/main.cpp src/PersistentObject.cpp src/utils.cpp)

target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/include)