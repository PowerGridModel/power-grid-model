# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

# CMakeList.txt : CMake project for power_grid_model, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.9)
project (power_grid_model)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Eigen3 CONFIG REQUIRED)
find_package(Boost REQUIRED)

add_library(power_grid_model INTERFACE)
target_link_libraries(power_grid_model INTERFACE Eigen3::Eigen)
include_directories(${Boost_INCLUDE_DIRS})
# only use MPL version of eigen
target_compile_definitions(power_grid_model INTERFACE EIGEN_MPL2_ONLY=1)
# header only library
target_include_directories(power_grid_model INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(WIN32)
  add_compile_options(/utf-8 /WX /bigobj /FC)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
  add_compile_options(-Wno-deprecated-copy)  # bug in boost
  add_compile_options(-Wno-deprecated-declarations)  # bug in boost
  # test coverage for debug build in linux, if specified
  if(UNIX AND 
     NOT APPLE AND 
    (CMAKE_BUILD_TYPE STREQUAL "Debug") AND 
    (DEFINED POWER_GRID_MODEL_COVERAGE) AND 
    (POWER_GRID_MODEL_COVERAGE EQUAL 1))
	  add_compile_options(-fprofile-arcs -ftest-coverage)
	  add_link_options(-fprofile-arcs)
  endif()
  # add address sanitizer for Linux/Mac build if specified
  if( 
    (DEFINED POWER_GRID_MODEL_SANITIZER) AND 
    (POWER_GRID_MODEL_SANITIZER EQUAL 1))
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
  endif()
  # thread
  find_package(Threads REQUIRED)
  target_link_libraries(power_grid_model INTERFACE Threads::Threads)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  add_compile_options(-Wno-maybe-uninitialized)  # bug in boost
endif()

# add C library
add_subdirectory(power_grid_model_c)

# get tests
add_subdirectory(tests/cpp_unit_tests)

# get benchmark
add_subdirectory(tests/benchmark_cpp)

# get c api example
add_subdirectory(power_grid_model_c_example)
