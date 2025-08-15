// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <string_view>

namespace power_grid_model {
namespace common::logging {

enum class LoggingTag : Idx {
    unknown = -1,
    log = 0,
    timer = 1,
    total_single_calculation_in_thread = 0100,
    total_batch_calculation_in_thread = 0200,
    copy_model = 1100,
    update_model = 1200,
    restore_model = 1201,
    scenario_exception = 1300,
    recover_from_bad = 1400,
    prepare = 2100,
    math_calculation = 2200,
    iterative_pf_solver_max_num_iter = 2226,
    ilse_max_num_iter = 2228,
    nrse_max_num_iter = 2229, // used to be duplicate number
    produce_output = 3000,
};

constexpr auto to_string(LoggingTag tag) {
    using enum LoggingTag;
    using namespace std::string_literals;

    switch (tag) {
    case log:
        return "log"s;
    case timer:
        return "timer"s;
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
    case math_calculation:
        return "Math calculation"s;
    case iterative_pf_solver_max_num_iter:
        // return "Iterative PF solver max num iter"s;
        return "Max number of iterations"s;
    case ilse_max_num_iter:
        // return "ILSE max num iter"s;
        return "Max number of iterations"s;
    case nrse_max_num_iter:
        // return "NRSE max num iter"s;
        return "Max number of iterations"s;
    case produce_output:
        return "Produce output"s;
    default:
        return "unknown"s;
    }
}

struct Logger {
    virtual void log(LoggingTag tag, std::string_view message) = 0;
    virtual void log(LoggingTag tag, double value) = 0;
    virtual void log(LoggingTag tag, Idx value) = 0;

    virtual ~Logger() = default;
};

struct LogDispatch : public Logger {
    virtual void registrar(Logger* logger) = 0;
    virtual void deregistrar(Logger* logger) = 0;
};

} // namespace common::logging

using common::logging::LoggingTag;

} // namespace power_grid_model
