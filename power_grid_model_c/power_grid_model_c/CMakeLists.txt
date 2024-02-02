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
)

target_include_directories(power_grid_model_c PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

set(PGM_PUBLIC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c/basics.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c/buffer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c/handle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c/meta_data.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c/model.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c/options.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c/dataset_definitions.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c/serialization.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c/dataset.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c.h"
)

target_link_libraries(power_grid_model_c
  PRIVATE power_grid_model
)

target_sources(power_grid_model_c PUBLIC 
  FILE_SET pgm_public_headers 
  TYPE HEADERS 
  BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/" 
  FILES "${PGM_PUBLIC_HEADERS}"
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
  FILE_SET pgm_public_headers
)
