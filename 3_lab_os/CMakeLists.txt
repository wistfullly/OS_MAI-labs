cmake_minimum_required(VERSION 3.10)
project(lab03)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(child ${SOURCE_DIR}/child.cpp)
add_executable(main ${SOURCE_DIR}/main.cpp)
