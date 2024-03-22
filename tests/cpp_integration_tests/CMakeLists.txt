# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

set(PROJECT_SOURCES
    "test_entry_point.cpp"
    "test_main_model_sc.cpp"
    "test_main_model_se.cpp"
    "test_main_model.cpp"
    "test_main_model_static.cpp"
    "test_math_solver.cpp"
)

add_executable(power_grid_model_integration_tests ${PROJECT_SOURCES})

target_link_libraries(power_grid_model_integration_tests
    PRIVATE
        power_grid_model
        doctest::doctest
        nlohmann_json nlohmann_json::nlohmann_json
)

doctest_discover_tests(power_grid_model_integration_tests)
