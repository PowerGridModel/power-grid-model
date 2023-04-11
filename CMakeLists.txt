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

# cmake output folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


find_package(Eigen3 CONFIG REQUIRED)
find_package(Boost REQUIRED)

add_library(power_grid_model INTERFACE)
target_link_libraries(power_grid_model INTERFACE Eigen3::Eigen)
include_directories(${Boost_INCLUDE_DIRS})
# only use MPL version of eigen
target_compile_definitions(power_grid_model INTERFACE EIGEN_MPL2_ONLY=1)
# header only library
target_include_directories(power_grid_model INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(NOT WIN32)
  # thread
  find_package(Threads REQUIRED)
  target_link_libraries(power_grid_model INTERFACE Threads::Threads)
endif()

# add C library
add_subdirectory(power_grid_model_c)

# get tests
add_subdirectory(tests/cpp_unit_tests)

# get benchmark
add_subdirectory(tests/benchmark_cpp)

# get c api example
add_subdirectory(power_grid_model_c_example)
