cmake_minimum_required(VERSION 2.8)
project(JsmnTest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
include_directories(${CMAKE_SOURCE_DIR}/jsmn)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(prototype prototype/test.cpp)
add_executable(jsmnutils_test test/test1.cpp)

add_library(jsmn jsmn/jsmn.c)
add_library(jsmnutils src/jsmnutils.cpp)

target_link_libraries(prototype jsmn)
target_link_libraries(jsmnutils jsmn)
target_link_libraries(jsmnutils_test jsmnutils)
