# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(power_grid_model_cli 
  main.cpp
  cli_options.cpp
  pgm_calculation.cpp
)

target_link_libraries(power_grid_model_cli PRIVATE power_grid_model_c)
target_link_libraries(power_grid_model_cli PRIVATE power_grid_model_cpp)
target_link_libraries(power_grid_model_cli PRIVATE CLI11::CLI11)

set_property(TARGET power_grid_model_cli PROPERTY INSTALL_RPATH_USE_LINK_PATH FALSE)
set_property(TARGET power_grid_model_cli PROPERTY OUTPUT_NAME "power-grid-model")
if(APPLE)
  set_property(TARGET power_grid_model_cli PROPERTY INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(UNIX) # Linux, BSD (not Windows/macOS)
  set_property(TARGET power_grid_model_cli PROPERTY INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()


target_compile_definitions(
    power_grid_model_cli
    PRIVATE PGM_ENABLE_EXPERIMENTAL
)

install(
    TARGETS power_grid_model_cli
    EXPORT power_grid_modelTargets
    COMPONENT power_grid_model
)
