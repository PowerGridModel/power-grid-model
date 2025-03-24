# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

set(PROJECT_SOURCES
    "test_entry_point.cpp"
    "test_validation.cpp"
)

add_executable(power_grid_model_validation_tests ${PROJECT_SOURCES})

target_link_libraries(power_grid_model_validation_tests
    PRIVATE
        power_grid_model_cpp
        doctest::doctest
        nlohmann_json nlohmann_json::nlohmann_json
)
target_compile_definitions(power_grid_model_validation_tests
	PRIVATE POWER_GRID_MODEL_VALIDATION_TEST_DATA_DIR="${PROJECT_SOURCE_DIR}/tests/data"
)

doctest_discover_tests(power_grid_model_validation_tests)
