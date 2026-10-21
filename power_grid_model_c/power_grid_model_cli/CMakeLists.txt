# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(power_grid_model_cli main.cpp)

target_link_libraries(power_grid_model_cli PRIVATE power_grid_model_cli_backend)

set_target_properties(
    power_grid_model_cli
    PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH FALSE
        OUTPUT_NAME "power-grid-model"
)
if(APPLE)
  set_target_property(power_grid_model_cli PROPERTIES INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(UNIX) # Linux, BSD (not Windows/macOS)
  set_target_properties(power_grid_model_cli PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

install(
    TARGETS power_grid_model_cli
    EXPORT power_grid_modelTargets
    COMPONENT power_grid_model
)
