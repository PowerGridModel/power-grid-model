# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

set(PROJECT_SOURCES 
	benchmark.cpp
)

add_executable(power_grid_model_benchmark_cpp EXCLUDE_FROM_ALL ${PROJECT_SOURCES})
target_link_libraries(power_grid_model_benchmark_cpp 
	PRIVATE power_grid_model 
)
target_compile_definitions(power_grid_model_benchmark_cpp PRIVATE "POWER_GRID_MODEL_CPP_BENCHMARK=1")
# set_target_properties(power_grid_model_benchmark_cpp PROPERTIES LINK_WHAT_YOU_USE TRUE)
