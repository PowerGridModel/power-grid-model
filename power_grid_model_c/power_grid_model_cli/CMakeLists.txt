# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(power-grid-model main.cpp)

target_link_libraries(power-grid-model PRIVATE power_grid_model_c)
target_link_libraries(power-grid-model PRIVATE power_grid_model_cpp)

set_property(TARGET power-grid-model PROPERTY INSTALL_RPATH_USE_LINK_PATH FALSE)
if(APPLE)
  set_property(TARGET power-grid-model PROPERTY INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(UNIX) # Linux, BSD (not Windows/macOS)
  set_property(TARGET power-grid-model PROPERTY INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()


target_compile_definitions(
    power-grid-model
    PRIVATE PGM_ENABLE_EXPERIMENTAL
)

install(
    TARGETS power-grid-model
    EXPORT power_grid_modelTargets
    COMPONENT power_grid_model
)
