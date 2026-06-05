# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(
    power_grid_model_unit_tests_auxiliary
    "../test_entry_point.cpp"
    "test_component_input.cpp"
    "test_component_output.cpp"
    "test_component_update.cpp"
    "test_meta_data_generation.cpp"
    "test_dataset.cpp"
    "test_deserializer.cpp"
    "test_serializer.cpp"
)

target_link_libraries(
    power_grid_model_unit_tests_auxiliary
    PRIVATE power_grid_model doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_auxiliary)
