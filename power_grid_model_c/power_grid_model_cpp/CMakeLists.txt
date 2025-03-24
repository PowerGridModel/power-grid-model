# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# C++ wrapper library around C API
add_library(power_grid_model_cpp INTERFACE)

target_include_directories(power_grid_model_cpp INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

file(
  GLOB_RECURSE
  pgm_cpp_public_headers
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

target_link_libraries(power_grid_model_cpp
	INTERFACE power_grid_model_c
)

target_sources(power_grid_model_cpp PUBLIC 
  FILE_SET pgm_cpp_public_headers 
  TYPE HEADERS 
  BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/" 
  FILES "${pgm_cpp_public_headers}"
)

set_target_properties(power_grid_model_cpp PROPERTIES
  VERSION ${PGM_VERSION}
  SOVERSION ${PGM_VERSION}
  CXX_STANDARD 17
)


install(TARGETS power_grid_model_cpp
  EXPORT power_grid_modelTargets
  COMPONENT power_grid_model
  FILE_SET pgm_cpp_public_headers
)
