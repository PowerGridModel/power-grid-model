# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

project (power_grid_model_c_api_tests)

set(PROJECT_SOURCES
	"test_c_api_meta_data.cpp"
	"test_c_api_model.cpp"
	"test_entry_point.cpp"
)

add_executable(power_grid_model_c_api_tests ${PROJECT_SOURCES})

target_link_libraries(power_grid_model_c_api_tests
	PRIVATE power_grid_model
	PRIVATE power_grid_model_c
	PRIVATE doctest::doctest
)

doctest_discover_tests(power_grid_model_c_api_tests)
