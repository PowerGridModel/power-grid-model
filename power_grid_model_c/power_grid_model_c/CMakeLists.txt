# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

add_library(power_grid_model_c SHARED "power_grid_model_c.cpp")

target_include_directories(power_grid_model_c PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(power_grid_model_c PRIVATE power_grid_model)

set_target_properties(power_grid_model_c PROPERTIES
  VERSION ${PGM_VERSION}
  SOVERSION ${PGM_VERSION}
  PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_c.h"
)

install(TARGETS power_grid_model_c
  EXPORT power_grid_modelTargets
  COMPONENT power_grid_model
  PUBLIC_HEADER
)
