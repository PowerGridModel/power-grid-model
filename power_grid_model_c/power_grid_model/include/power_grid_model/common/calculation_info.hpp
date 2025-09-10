// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "multi_threaded_logging.hpp"

#include <map>

namespace power_grid_model {
namespace common::logging {
class CalculationInfo : public Logger {
    using Data = std::map<LogEvent, double>;

  public:
    using Report = std::add_lvalue_reference_t<std::add_const_t<Data>>;
    static_assert(std::same_as<Report, Data const&>);

    CalculationInfo() = default;
    CalculationInfo(CalculationInfo const&) = default;
    CalculationInfo(CalculationInfo&&) noexcept = default;
    CalculationInfo& operator=(CalculationInfo const&) = default;
    CalculationInfo& operator=(CalculationInfo&&) noexcept = default;
    ~CalculationInfo() override = default;

    void log(LogEvent /*tag*/) override {
        // ignore all such events for now
    }
    void log(LogEvent /*tag*/, std::string_view /*message*/) override {
        // ignore all such events for now
    }
    void log(LogEvent tag, double value) override { log_impl(tag, value); }
    void log(LogEvent tag, Idx value) override { log_impl(tag, static_cast<double>(value)); }

  private:
    Data data_;

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
            accumulate_log(tag, value);
            return;
        case iterative_pf_solver_max_num_iter:
        case max_num_iter:
            maximize_log(tag, value);
            return;
        default:
            return;
        }
    }
    void accumulate_log(LogEvent tag, double value) { data_[tag] += value; }
    void maximize_log(LogEvent tag, double value) {
        auto& stored_value = data_[tag];
        stored_value = std::max(value, stored_value);
    }

  public:
    Report report() const { return data_; }
    void clear() { data_.clear(); }

    template <std::derived_from<Logger> T> T& merge_into(T& destination) const {
        if (&destination == this) {
            return destination; // nothing to do
        }
        for (const auto& [tag, value] : report()) {
            destination.log(tag, value);
        }
        return destination;
    }
};

class MultiThreadedCalculationInfo : public MultiThreadedLoggerImpl<CalculationInfo> {
  public:
    using MultiThreadedLoggerImpl<CalculationInfo>::MultiThreadedLoggerImpl;
    using Report = CalculationInfo::Report;

    Report report() const { return get().report(); }
    void clear() { get().clear(); }
};
} // namespace common::logging

using common::logging::CalculationInfo;
using common::logging::MultiThreadedCalculationInfo;
} // namespace power_grid_model
