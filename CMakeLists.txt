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
  if(UNIX AND NOT APPLE)
    list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .so.1)
    list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .so.2)
	# test coverage for debug build in linux
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	  add_compile_options(-fprofile-arcs -ftest-coverage)
	  add_link_options(-fprofile-arcs)
	endif()
  else()
    list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .1.dylib)
  endif()
  # thread
  find_package(Threads REQUIRED)
  target_link_libraries(power_grid_model INTERFACE Threads::Threads)
endif()


# sparse solver, default eigen
if (NOT DEFINED POWER_GRID_MODEL_SPARSE_SOLVER)
	set(POWER_GRID_MODEL_SPARSE_SOLVER "EIGEN")
endif()

# use mkl solver
if((POWER_GRID_MODEL_SPARSE_SOLVER STREQUAL "MKL") OR (POWER_GRID_MODEL_SPARSE_SOLVER STREQUAL "MKL_RUNTIME"))
	if(NOT DEFINED ENV{MKL_INCLUDE})
		message(FATAL_ERROR "MKL include dir not found!")
	endif()
	if(NOT DEFINED ENV{MKL_LIB})
		message(FATAL_ERROR "MKL lib dir not found!")
	endif()
	# mkl
	target_include_directories(power_grid_model INTERFACE $ENV{MKL_INCLUDE})
	list(APPEND CMAKE_PREFIX_PATH "$ENV{MKL_LIB}")
	find_library(MKL_RT mkl_rt)
	message("${MKL_RT}")
	# check load mkl at runtime or link-time
 	if(POWER_GRID_MODEL_SPARSE_SOLVER STREQUAL "MKL_RUNTIME")
		target_compile_definitions(power_grid_model INTERFACE POWER_GRID_MODEL_USE_MKL_AT_RUNTIME=1)
		target_link_libraries(power_grid_model INTERFACE ${CMAKE_DL_LIBS})
	else()
		target_compile_definitions(power_grid_model INTERFACE POWER_GRID_MODEL_USE_MKL=1)
		target_link_libraries(power_grid_model INTERFACE "${MKL_RT}")
	endif()
endif()


# get tests
add_subdirectory(tests/cpp_unit_tests)

# get benchmark
if(DEFINED POWER_GRID_MODEL_BUILD_BENCHMARK AND POWER_GRID_MODEL_BUILD_BENCHMARK EQUAL 1)
	add_subdirectory(tests/benchmark_cpp)
endif()

