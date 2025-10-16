# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(power_grid_model_unit_tests_common 
    "test_entry_point.cpp"
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

target_link_libraries(power_grid_model_unit_tests_common
    PRIVATE
        power_grid_model
        doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_common)

add_executable(power_grid_model_unit_tests_auxiliary 
    "test_entry_point.cpp"
    "test_component_input.cpp"
    "test_component_output.cpp"
    "test_component_update.cpp"
    "test_meta_data_generation.cpp"
    "test_dataset.cpp"
    "test_deserializer.cpp"
    "test_serializer.cpp"
)

target_link_libraries(power_grid_model_unit_tests_auxiliary
    PRIVATE
        power_grid_model
        doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_auxiliary)

add_executable(power_grid_model_unit_tests_component
    "test_entry_point.cpp"
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
)

target_link_libraries(power_grid_model_unit_tests_component
    PRIVATE
        power_grid_model
        doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_component)

add_executable(power_grid_model_unit_tests_main_core
    "test_entry_point.cpp"
    "test_main_core_output.cpp"
    "test_main_model_type.cpp"
)

target_link_libraries(power_grid_model_unit_tests_main_core
    PRIVATE
        power_grid_model
        doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_main_core)

add_executable(power_grid_model_unit_tests_math_solver
    "test_entry_point.cpp"
    "test_sparse_lu_solver.cpp"
    "test_y_bus.cpp"
    "test_measured_values.cpp"
    "test_observability.cpp"
    "test_math_solver_pf_linear.cpp"
    "test_math_solver_pf_newton_raphson.cpp"
    "test_math_solver_pf_iterative_current.cpp"
    "test_math_solver_se_newton_raphson.cpp"
    "test_math_solver_se_iterative_linear.cpp"
    "test_math_solver_sc.cpp"
)

target_link_libraries(power_grid_model_unit_tests_math_solver
    PRIVATE
        power_grid_model
        doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_math_solver)

add_executable(power_grid_model_unit_tests_optimizer
    "test_entry_point.cpp"
    "test_optimizer.cpp"
    "test_tap_position_optimizer.cpp"
)

target_link_libraries(power_grid_model_unit_tests_optimizer
    PRIVATE
        power_grid_model
        doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_optimizer)

add_executable(power_grid_model_unit_tests
    "test_entry_point.cpp"
    "test_topology.cpp"
    "test_sparse_ordering.cpp"
    "test_container.cpp"
    "test_index_mapping.cpp"
    "test_job_dispatch.cpp"
)

target_link_libraries(power_grid_model_unit_tests
    PRIVATE
        power_grid_model
        doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests)
