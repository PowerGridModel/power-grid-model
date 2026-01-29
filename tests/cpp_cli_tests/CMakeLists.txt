# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

set(PROJECT_SOURCES "test_entry_point.cpp")

add_executable(power_grid_model_cli_tests ${PROJECT_SOURCES})

target_link_libraries(
    power_grid_model_cli_tests
    PRIVATE
        power_grid_model_cpp
        doctest::doctest
        nlohmann_json
        nlohmann_json::nlohmann_json
)
target_compile_definitions(
    power_grid_model_cli_tests
    PRIVATE
        POWER_GRID_MODEL_CLI_EXECUTABLE="$<TARGET_FILE:power_grid_model_cli>"
)
add_dependencies(power_grid_model_cli_tests power_grid_model_cli)

doctest_discover_tests(power_grid_model_cli_tests)
