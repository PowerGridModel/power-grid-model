# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(
    power_grid_model_unit_tests_common
    "../test_entry_point.cpp"
    "test_common.cpp"
    "test_exceptions.cpp"
    "test_component_list.cpp"
    "test_grouped_index_vector.cpp"
    "test_counting_iterator.cpp"
    "test_statistics.cpp"
    "test_three_phase_tensor.cpp"
    "test_typing.cpp"
    "test_iterator_facade.cpp"
)

target_link_libraries(
    power_grid_model_unit_tests_common
    PRIVATE power_grid_model doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_common)
