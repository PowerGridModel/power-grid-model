# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

set(PROJECT_SOURCES
    "test_entry_point.cpp"
    "test_all_components.cpp"
    "test_common.cpp"
    "test_counting_iterator.cpp"
    "test_exceptions.cpp"
    "test_component_list.cpp"
    "test_component_input.cpp"
    "test_component_output.cpp"
    "test_component_update.cpp"
    "test_three_phase_tensor.cpp"
    "test_statistics.cpp"
    "test_node.cpp"
    "test_asym_line.cpp"
    "test_line.cpp"
    "test_generic_branch.cpp"
    "test_link.cpp"
    "test_load_gen.cpp"
    "test_source.cpp"
    "test_shunt.cpp"
    "test_transformer.cpp"
    "test_sparse_lu_solver.cpp"
    "test_y_bus.cpp"
    "test_measured_values.cpp"
    "test_observability.cpp"
    "test_topology.cpp"
    "test_sparse_ordering.cpp"
    "test_grouped_index_vector.cpp"
    "test_container.cpp"
    "test_index_mapping.cpp"
    "test_meta_data_generation.cpp"
    "test_voltage_sensor.cpp"
    "test_power_sensor.cpp"
    "test_three_winding_transformer.cpp"
    "test_fault.cpp"
    "test_dataset.cpp"
    "test_deserializer.cpp"
    "test_serializer.cpp"
    "test_typing.cpp"
    "test_transformer_tap_regulator.cpp"
    "test_optimizer.cpp"
    "test_tap_position_optimizer.cpp"
    "test_main_core_output.cpp"
    "test_math_solver_pf_linear.cpp"
    "test_math_solver_pf_newton_raphson.cpp"
    "test_math_solver_pf_iterative_current.cpp"
    "test_math_solver_se_newton_raphson.cpp"
    "test_math_solver_se_iterative_linear.cpp"
    "test_math_solver_sc.cpp"
    "test_current_sensor.cpp"
)

add_executable(power_grid_model_unit_tests ${PROJECT_SOURCES})

target_link_libraries(power_grid_model_unit_tests
    PRIVATE
        power_grid_model
        doctest::doctest
        nlohmann_json nlohmann_json::nlohmann_json
)

doctest_discover_tests(power_grid_model_unit_tests)
