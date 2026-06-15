# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(
    power_grid_model_unit_tests_component
    "../test_entry_point.cpp"
    "test_all_components.cpp"
    "test_node.cpp"
    "test_asym_line.cpp"
    "test_line.cpp"
    "test_generic_branch.cpp"
    "test_link.cpp"
    "test_load_gen.cpp"
    "test_source.cpp"
    "test_shunt.cpp"
    "test_transformer.cpp"
    "test_voltage_sensor.cpp"
    "test_power_sensor.cpp"
    "test_three_winding_transformer.cpp"
    "test_fault.cpp"
    "test_transformer_tap_regulator.cpp"
    "test_current_sensor.cpp"
    "test_voltage_regulator.cpp"
)

target_link_libraries(
    power_grid_model_unit_tests_component
    PRIVATE power_grid_model doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_component)
