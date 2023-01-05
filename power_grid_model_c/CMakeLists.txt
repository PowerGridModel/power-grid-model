# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

project (power_grid_model_c)

add_library(power_grid_model_c power_grid_model_c.cpp)

target_link_libraries(power_grid_model_c
	PRIVATE power_grid_model
)

target_compile_definitions(power_grid_model_c PRIVATE POWER_GRID_MODEL_BUILD_IN_PROGRESS)

if (BUILD_SHARED_LIBS STREQUAL "ON")
    target_compile_definitions(power_grid_model_c PRIVATE POWER_GRID_MODEL_DLL_EXPORTS)
else()
    target_compile_definitions(power_grid_model_c PRIVATE POWER_GRID_MODEL_STATIC)
endif()