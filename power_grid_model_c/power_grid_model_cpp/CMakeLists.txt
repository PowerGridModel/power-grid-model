# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# C++ API library
add_library(power_grid_model_cpp INTERFACE)

target_include_directories(power_grid_model_cpp INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

set(PGM_PUBLIC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_cpp/basics.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_cpp/buffer.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_cpp/dataset.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_cpp/handle.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_cpp/meta_data.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_cpp/model.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_cpp/options.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_cpp/serialization.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/power_grid_model_cpp.hpp"
)

set_target_properties(power_grid_model_cpp PROPERTIES
  VERSION ${PGM_VERSION}
  SOVERSION ${PGM_VERSION}
)

install(TARGETS power_grid_model_cpp
  EXPORT power_grid_modelCppTargets
  COMPONENT power_grid_model
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION include
  FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT power_grid_modelCppTargets
  FILE power_grid_modelCppTargets.cmake
  DESTINATION lib/cmake/power_grid_modelCpp
)