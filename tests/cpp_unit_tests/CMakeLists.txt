# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

project (power_grid_model_unit_tests)

find_package(doctest REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(PROJECT_SOURCES
	test_power_grid_model.cpp
	test_main_model_se.cpp
	test_main_model.cpp
	test_main_model_static.cpp
	test_three_phase_tensor.cpp
	test_node.cpp
	test_line.cpp
	test_link.cpp
	test_load_gen.cpp
	test_source.cpp
	test_shunt.cpp
	test_transformer.cpp
	test_sparse_lu_solver.cpp
	test_y_bus.cpp
	test_math_solver.cpp
	test_topology.cpp
	test_container.cpp
	test_sparse_mapping.cpp
	test_meta_data_generation.cpp
	test_voltage_sensor.cpp
	test_power_sensor.cpp
	test_validation.cpp
	test_three_winding_transformer.cpp
	test_c_api_meta_data.cpp
	test_c_api_model.cpp
)

add_executable(power_grid_model_unit_tests ${PROJECT_SOURCES})

target_link_libraries(power_grid_model_unit_tests
	PRIVATE power_grid_model
	PRIVATE power_grid_model_c
	PRIVATE doctest::doctest
	PRIVATE nlohmann_json nlohmann_json::nlohmann_json
)
target_compile_definitions(power_grid_model_unit_tests PRIVATE POWER_GRID_MODEL_VALIDATION_TEST_DATA_PATH="${CMAKE_SOURCE_DIR}/tests/data")

add_test(PGMUnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/power_grid_model_unit_tests)
