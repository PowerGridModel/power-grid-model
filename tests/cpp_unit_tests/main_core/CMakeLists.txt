# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(
    power_grid_model_unit_tests_main_core
    "../test_entry_point.cpp"
    "test_main_core_output.cpp"
    "test_main_model_type.cpp"
)

target_link_libraries(
    power_grid_model_unit_tests_main_core
    PRIVATE power_grid_model doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_main_core)
