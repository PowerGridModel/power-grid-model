// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logging_impl.hpp"

#include <cstddef>
#include <functional>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

namespace power_grid_model {
namespace common::logging {
class CalculationInfo : public NoLogger {
  public:
    using Report = std::map<LogEvent, double>;
    using const_iterator = Report::const_iterator;

    void log(LogEvent tag, double value) override { log_impl(tag, value); }
    void log(LogEvent tag, Idx value) override { log_impl(tag, static_cast<double>(value)); }

  private:
    Report data_;

    void log_impl(LogEvent tag, double value) {
        using enum LogEvent;

        switch (tag) {
        case total:
        case build_model:
        case total_single_calculation_in_thread:
        case total_batch_calculation_in_thread:
        case copy_model:
        case update_model:
        case restore_model:
        case scenario_exception:
        case recover_from_bad:
        case prepare:
        case create_math_solver:
        case math_calculation:
        case math_solver:
        case initialize_calculation:
        case preprocess_measured_value:
        case prepare_matrix:
        case prepare_matrix_including_prefactorization:
        case prepare_matrices:
        case initialize_voltages:
        case calculate_rhs:
        case prepare_lhs_rhs:
        case solve_sparse_linear_equation:
        case solve_sparse_linear_equation_prefactorized:
        case iterate_unknown:
        case calculate_math_result:
        case produce_output:
            return accumulate_log(tag, value);
        case iterative_pf_solver_max_num_iter:
        case max_num_iter:
            return maximize_log(tag, value);
        default:
            return;
        }
    }
    void accumulate_log(LogEvent tag, double value) { data_[tag] += value; }
    void maximize_log(LogEvent tag, double value) {
        if (auto& stored_value = data_[tag]; value > stored_value) {
            stored_value = value;
        }
    }

  public:
    Report const& report() const { return data_; }
};
} // namespace common::logging

using common::logging::CalculationInfo;
} // namespace power_grid_model
