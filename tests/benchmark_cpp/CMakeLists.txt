# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

project (power_grid_model_benchmark_cpp)

set(PROJECT_SOURCES 
	benchmark.cpp
)

add_executable(power_grid_model_benchmark_cpp ${PROJECT_SOURCES})
target_link_libraries(power_grid_model_benchmark_cpp 
	PRIVATE power_grid_model 
)
# set_target_properties(power_grid_model_benchmark_cpp PROPERTIES LINK_WHAT_YOU_USE TRUE)

