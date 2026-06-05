# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_subdirectory(common)
add_subdirectory(auxiliary)
add_subdirectory(component)
add_subdirectory(main_core)
add_subdirectory(math_solver)
add_subdirectory(optimizer)
add_subdirectory(logging)

add_executable(
    power_grid_model_unit_tests
    "test_entry_point.cpp"
    "test_topology.cpp"
    "test_sparse_ordering.cpp"
    "test_container.cpp"
    "test_index_mapping.cpp"
    "test_job_dispatch.cpp"
    "test_calculation_preparation.cpp"
    "test_link_solver.cpp"
    "test_supernodes.cpp"
)

target_link_libraries(
    power_grid_model_unit_tests
    PRIVATE power_grid_model doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests)
