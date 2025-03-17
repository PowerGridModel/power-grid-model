# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# C API library
add_library(power_grid_model_c SHARED
  "src/buffer.cpp"
  "src/handle.cpp"
  "src/meta_data.cpp"
  "src/model.cpp"
  "src/options.cpp"
  "src/dataset_definitions.cpp"
  "src/serialization.cpp"
  "src/dataset.cpp"
  "src/math_solver.cpp"
)

target_include_directories(power_grid_model_c PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

file(
  GLOB_RECURSE
  pgm_c_public_headers
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

target_link_libraries(power_grid_model_c
  PRIVATE power_grid_model
)

target_sources(power_grid_model_c PUBLIC 
  FILE_SET pgm_c_public_headers 
  TYPE HEADERS 
  BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/" 
  FILES "${pgm_c_public_headers}"
  )

set_target_properties(power_grid_model_c PROPERTIES
  VERSION ${PGM_VERSION}
  SOVERSION ${PGM_VERSION}
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
  INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
)

install(TARGETS power_grid_model_c
  EXPORT power_grid_modelTargets
  COMPONENT power_grid_model
  FILE_SET pgm_c_public_headers
)
