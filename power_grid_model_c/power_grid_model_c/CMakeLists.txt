# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

add_library(power_grid_model_c SHARED "power_grid_model_c.cpp")

target_include_directories(power_grid_model_c PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(power_grid_model_c PRIVATE power_grid_model)
