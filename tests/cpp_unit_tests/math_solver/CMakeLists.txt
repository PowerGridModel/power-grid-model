# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(
    power_grid_model_unit_tests_math_solver
    "../test_entry_point.cpp"
    "test_math_solver_pf_newton_raphson.cpp"
    "test_math_solver_se_newton_raphson.cpp"
    "test_math_solver_se_iterative_linear.cpp"
    "test_math_solver_pf_iterative_current.cpp"
    "test_math_solver_pf_linear.cpp"
    "test_math_solver_sc.cpp"
    "test_sparse_lu_solver.cpp"
    "test_observability.cpp"
    "test_measured_values.cpp"
    "test_y_bus.cpp"
)

target_link_libraries(
    power_grid_model_unit_tests_math_solver
    PRIVATE power_grid_model doctest::doctest
)

doctest_discover_tests(power_grid_model_unit_tests_math_solver)
