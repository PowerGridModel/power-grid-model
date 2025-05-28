# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# CMakeList.txt : CMake project for power_grid_model, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.23)

include("cmake/pgm_version.cmake")

project (power_grid_model VERSION ${PGM_VERSION})

# The following option is used to skip all the builds.
# This is needed to run Python packaging in conda-forge.
# We have separate cmake build and install in conda-forge.
# So this option bypasses the build step from py-build-cmake.
option(PGM_SKIP_BUILD "Skip all the builds" OFF)
if (PGM_SKIP_BUILD)
  message(STATUS "Skip all the builds")
  return()
endif()

option(PGM_ENABLE_DEV_BUILD "Enable developer build (e.g.: tests)" OFF)

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

include(GNUInstallDirs)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0") 
#  libboost-headers packaged by conda does not come with BoostConfig.cmake
cmake_policy(SET CMP0167 OLD)  
endif()

# If cmake is invoked from py-build-cmake,
# we find include path through command
if (DEFINED PY_BUILD_CMAKE_VERSION)
  execute_process(
    COMMAND pgm-build-dependencies
    OUTPUT_VARIABLE PY_BUILD_CMAKE_DEP_INCLUDE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  find_package(Boost REQUIRED)
  find_package(Eigen3 CONFIG REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)
  find_package(msgpack-cxx REQUIRED)
endif()

if(NOT WIN32)
  # thread
  find_package(Threads REQUIRED)
endif()

# If cmake is invoked from py-build-cmake,
# we set the install path of the cmake targets
if (DEFINED PY_BUILD_CMAKE_VERSION)
    message(STATUS "Configuring installation paths for py-build-cmake")
    set(PGM_WHEEL_DIR "${PY_BUILD_CMAKE_PROJECT_NAME}-${PY_BUILD_CMAKE_PROJECT_VERSION}.data/data")
    set(PGM_SHARE_DIR "${PGM_WHEEL_DIR}/share")

    # In Windows, the convention is to use the Library folder
    if (WIN32)
      set(CMAKE_INSTALL_LIBDIR "${PGM_WHEEL_DIR}/Library/lib")
      set(CMAKE_INSTALL_INCLUDEDIR "${PGM_WHEEL_DIR}/Library/include")
      set(CMAKE_INSTALL_BINDIR "${PGM_WHEEL_DIR}/Library/bin")
    else()
      set(CMAKE_INSTALL_LIBDIR "${PGM_WHEEL_DIR}/lib")
      set(CMAKE_INSTALL_INCLUDEDIR "${PGM_WHEEL_DIR}/include")
      set(CMAKE_INSTALL_BINDIR "${PGM_WHEEL_DIR}/bin")
    endif()
else()
    set(PGM_SHARE_DIR "share")
endif()

# add C library
add_subdirectory("power_grid_model_c")

# dev build
if(${PGM_ENABLE_DEV_BUILD})
  include(CTest)
  enable_testing()
  # get tests
  add_subdirectory("tests")
  # get c api example
  add_subdirectory("power_grid_model_c_example")
endif()



# export the power grid model
include("cmake/export_power_grid_model.cmake")
