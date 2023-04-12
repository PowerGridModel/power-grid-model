# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

# CMakeList.txt : CMake project for power_grid_model, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.9)

include("cmake/pgm_version.cmake")

project (power_grid_model VERSION ${PGM_VERSION})

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

include(CTest)
enable_testing()

find_package(Eigen3 CONFIG REQUIRED)
find_package(Boost REQUIRED)

if(WIN32)
  add_compile_options(/utf-8 /WX /bigobj /FC)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-copy>)  # bug in boost
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
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  add_compile_options(-Wno-maybe-uninitialized)  # bug in boost
endif()

# add C library
add_subdirectory("power_grid_model_c")

# get tests
add_subdirectory("tests")

# get c api example
add_subdirectory("power_grid_model_c_example")

install(EXPORT "PGMTargets"
  DESTINATION "lib/cmake/PGM"
  NAMESPACE PGM::
  COMPONENT PGM
)

include(CMakePackageConfigHelpers)
configure_package_config_file("cmake/PGMConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/PGM/PGMConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/PGM"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/PGM/PGMConfigVersion.cmake"
  VERSION ${PGM_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/PGM/PGMConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/PGM/PGMConfigVersion.cmake"
  DESTINATION "lib/cmake/PGM"
)