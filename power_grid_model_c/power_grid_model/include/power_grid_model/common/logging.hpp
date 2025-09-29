// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <memory>
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

class Logger {
  public:
    virtual void log(LogEvent tag) = 0;
    virtual void log(LogEvent tag, std::string_view message) = 0;
    virtual void log(LogEvent tag, double value) = 0;
    virtual void log(LogEvent tag, Idx value) = 0;

    Logger(Logger&&) noexcept = default;
    Logger& operator=(Logger&&) noexcept = default;
    virtual ~Logger() = default;

  protected:
    Logger() = default;
    Logger(Logger const&) = default;
    Logger& operator=(Logger const&) = default;
};

struct MultiThreadedLogger : public Logger {
    virtual std::unique_ptr<Logger> create_child() = 0;
};

} // namespace common::logging

using common::logging::LogEvent;
using common::logging::Logger;

} // namespace power_grid_model
