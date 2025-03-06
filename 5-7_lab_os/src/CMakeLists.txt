cmake_minimum_required(VERSION 3.10)
project(os_labs)
set(CUR_PR "lab5_7")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${CUR_PR}_lib lib.cpp)

add_executable(control control.cpp)
add_executable(computing computing.cpp)
target_include_directories(${CUR_PR}_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(control PRIVATE ${CUR_PR}_lib zmq)
target_link_libraries(computing PRIVATE ${CUR_PR}_lib zmq)