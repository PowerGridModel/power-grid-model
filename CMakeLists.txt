# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# CMakeList.txt : CMake project for power_grid_model, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.23)

include("cmake/pgm_version.cmake")

project (power_grid_model VERSION ${PGM_VERSION})

set(CMAKE_CXX_STANDARD 20)
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

include(CTest)
enable_testing()

include(GNUInstallDirs)

find_package(Eigen3 CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(msgpack-cxx REQUIRED)

if(NOT WIN32)
  # thread
  find_package(Threads REQUIRED)
endif()

# add C library
add_subdirectory("power_grid_model_c")

# get tests
add_subdirectory("tests")

# get c api example
add_subdirectory("power_grid_model_c_example")

# export the power grid model
include("cmake/export_power_grid_model.cmake")
