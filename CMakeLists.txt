cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_compile_options(-Wall -O3)

project(lumos VERSION 1.0)

file(GLOB_RECURSE SOURCE_FILES src/*.cc)
add_executable(lumos ${SOURCE_FILES})

include(cmake/CPM.cmake)
CPMAddPackage("gh:fmtlib/fmt#9.0.0")
CPMAddPackage("gh:g-truc/glm#0.9.9.8")
CPMAddPackage("gh:SFML/SFML#2.6.x")

target_include_directories(lumos PRIVATE ${glm_SOURCE_DIR})
target_include_directories(lumos PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(lumos PRIVATE ${CMAKE_SOURCE_DIR}/vendor)

target_compile_features(lumos PRIVATE cxx_std_17)
target_link_libraries(lumos fmt sfml-system sfml-graphics sfml-window)
