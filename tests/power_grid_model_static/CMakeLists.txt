# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_library(power_grid_model_static STATIC
  "src/main_model_wrapper.cpp"
)

target_link_libraries(power_grid_model_static PUBLIC
  power_grid_model
)

target_include_directories(power_grid_model_static PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
