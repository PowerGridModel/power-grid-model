# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

# The detailed headers
add_library(power_grid_model_c_headers INTERFACE)

target_include_directories(power_grid_model_c_headers INTERFACE
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
)

set_target_properties(power_grid_model_c_headers PROPERTIES
  VERSION ${PGM_VERSION}
  SOVERSION ${PGM_VERSION}
  PUBLIC_HEADER "${PGM_PUBLIC_HEADERS}"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
  INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
)

install(TARGETS power_grid_model_c_headers
  EXPORT power_grid_modelTargets
  COMPONENT power_grid_model
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/power_grid_model_c"
)

# The actual implementation
add_library(power_grid_model_c SHARED 
  "src/buffer.cpp"
  "src/handle.cpp"
  "src/meta_data.cpp"
  "src/model.cpp"
  "src/options.cpp"
  "src/dataset_definitions.cpp"
)

target_include_directories(power_grid_model_c PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(power_grid_model_c
  PUBLIC power_grid_model_c_headers
  PRIVATE power_grid_model
)

set_target_properties(power_grid_model_c PROPERTIES
  VERSION ${PGM_VERSION}
  SOVERSION ${PGM_VERSION}
  PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c.h"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
  INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
)

install(TARGETS power_grid_model_c
  EXPORT power_grid_modelTargets
  COMPONENT power_grid_model
  PUBLIC_HEADER
)
