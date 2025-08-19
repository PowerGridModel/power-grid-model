// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <string_view>

namespace power_grid_model {
namespace common::logging {

enum class LogEvent : int16_t {
    unknown = -1,
    total = 0000,       // TODO(mgovers): find other error code?
    build_model = 1000, // TODO(mgovers): find other error code?
    total_single_calculation_in_thread = 0100,
    total_batch_calculation_in_thread = 0200,
    copy_model = 1100,
    update_model = 1200,
    restore_model = 1201,
    scenario_exception = 1300,
    recover_from_bad = 1400,
    prepare = 2100,
    create_math_solver = 2210,
    math_calculation = 2200,
    math_solver = 2220,
    initialize_calculation = 2221,
    preprocess_measured_value = 2231, // TODO(mgovers): find other error code + make plural?
    prepare_matrix = 2222,
    prepare_matrix_including_prefactorization = 2232, // TODO(mgovers): find other error code
    prepare_matrices = 2242,                          // TODO(mgovers): find other error code
    initialize_voltages = 2223,
    calculate_rhs = 2224,
    prepare_lhs_rhs = 2244, // TODO(mgovers): find other error code
    solve_sparse_linear_equation = 2225,
    solve_sparse_linear_equation_prefactorized = 2235, // TODO(mgovers): find other error code
    iterate_unknown = 2226,
    calculate_math_result = 2227,
    produce_output = 3000,
    iterative_pf_solver_max_num_iter = 2246, // TODO(mgovers): find other error code
    max_num_iter = 2248,                     // TODO(mgovers): find other error code
};

constexpr std::string to_string(LogEvent tag) {
    using enum LogEvent;
    using namespace std::string_literals;

    switch (tag) {
    case total:
        return "Total"s;
    case build_model:
        return "Build model"s;
    case total_single_calculation_in_thread:
        return "Total single calculation in thread"s;
    case total_batch_calculation_in_thread:
        return "Total batch calculation in thread"s;
    case copy_model:
        return "Copy model"s;
    case update_model:
        return "Update model"s;
    case restore_model:
        return "Restore model"s;
    case scenario_exception:
        return "Scenario exception"s;
    case recover_from_bad:
        return "Recover from bad"s;
    case prepare:
        return "Prepare"s;
    case create_math_solver:
        return "Create math solver"s;
    case math_calculation:
        return "Math Calculation"s; // TODO(mgovers): make capitalization consistent
    case math_solver:
        return "Math solver"s;
    case initialize_calculation:
        return "Initialize calculation"s;
    case preprocess_measured_value:
        return "Pre-process measured value"s; // TODO(mgovers): make plural
    case prepare_matrix:
        return "Prepare matrix"s;
    case prepare_matrix_including_prefactorization:
        return "Prepare matrix, including pre-factorization"s;
    case prepare_matrices:
        return "Prepare the matrices"s; // TODO(mgovers): combine or properly split up?
    case initialize_voltages:
        return "Initialize voltages"s;
    case calculate_rhs:
        return "Calculate rhs"s; // TODO(mgovers): capitalize?
    case prepare_lhs_rhs:
        return "Prepare LHS rhs"s;
    case solve_sparse_linear_equation:
        return "Solve sparse linear equation"s;
    case solve_sparse_linear_equation_prefactorized:
        return "Solve sparse linear equation (pre-factorized)"s;
    case iterate_unknown:
        return "Iterate unknown"s;
    case calculate_math_result:
        return "Calculate math result"s;
    case produce_output:
        return "Produce output"s;
    case iterative_pf_solver_max_num_iter:
        // return "Max number of iterations"s; // TODO(mgovers): different messages?
        [[fallthrough]];
    case max_num_iter:
        return "Max number of iterations"s; // TODO(mgovers): different messages?
    case unknown:
        [[fallthrough]];
    default:
        return "unknown"s;
    }
}

} // namespace common::logging

using common::logging::LogEvent;

} // namespace power_grid_model
