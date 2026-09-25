// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

namespace power_grid_model {
namespace common::logging {
enum class LogEvent : int16_t {
    unknown = -1,
    critical = 0,
    error = 1,
    warning = 2,
    info = 3,
    debug = 4,

    // benchmark events, grouped into blocks with gaps left open, so a future addition (e.g. a
    // loop nested inside an existing loop) can pick a free code from the relevant block below
    // instead of hunting for one.

    // application scope (tests/benchmark_cpp)
    total = 100,

    // per-thread/per-scenario scope (job_dispatch.hpp)
    total_single_calculation_in_thread = 1000,
    total_batch_calculation_in_thread = 1010,
    copy_model = 1020,
    update_model = 1030,
    restore_model = 1031, // sub-step of update_model
    scenario_exception = 1040,
    recover_from_bad = 1050,

    // single calculation pipeline (main_model_impl.hpp)
    build_model = 2000,
    prepare = 2010,
    math_calculation = 2020,
    create_math_solver = 2021, // sub-step of math_calculation
    math_solver = 2022,        // sub-step of math_calculation; wraps the solver run below
    produce_output = 2030,

    // math solver run: one-shot steps (iterative_linear_se_solver.hpp, iterative_pf_solver.hpp,
    // newton_raphson_se_solver.hpp)
    initialize_calculation = 2100,
    preprocess_measured_value = 2101,
    prepare_matrix = 2102,
    prepare_matrix_including_prefactorization = 2103,
    prepare_matrices = 2104,
    initialize_voltages = 2105,
    calculate_rhs = 2106,
    prepare_lhs_rhs = 2107,
    solve_sparse_linear_equation = 2108,
    solve_sparse_linear_equation_prefactorized = 2109,
    calculate_math_result = 2110,
    // 2111-2119 reserved for future one-shot solver steps

    // math solver run: outer loop, one iteration of the solver's main loop
    iterate_unknown = 2120,
    max_num_iter = 2121,
    iterative_pf_solver_max_num_iter = 2122,
    // 2123-2139 reserved for future outer-loop steps

    // math solver run: inner loop, reserved for a loop nested inside iterate_unknown
    // 2140-2159 reserved for future inner-loop steps
};

template <typename Fn>
concept LazyLoggingFn = std::invocable<Fn> && std::convertible_to<std::invoke_result_t<Fn>, std::string> &&
                        (!std::convertible_to<Fn, std::string_view>) && functor_c<Fn>;

class Logger {
  public:
    virtual void log(LogEvent tag) = 0;
    virtual void log(LogEvent tag, std::string_view message) = 0;
    virtual void log(LogEvent tag, double value) = 0;
    virtual void log(LogEvent tag, Idx value) = 0;

    void log(std::string_view message) { log(LogEvent::unknown, message); }
    template <LazyLoggingFn Fn> void log(LogEvent tag, Fn fn) { log(tag, std::invoke(fn)); }
    template <LazyLoggingFn Fn> void log(Fn fn) { log(LogEvent::unknown, std::invoke(fn)); }

    Logger(Logger&&) noexcept = default;
    Logger& operator=(Logger&&) noexcept = default;
    virtual ~Logger() = default;

  protected:
    Logger() = default;
    Logger(Logger const&) = default;
    Logger& operator=(Logger const&) = default;
};

class MultiThreadedLogger : public Logger {
  public:
    virtual std::unique_ptr<Logger> create_child() { return nullptr; };

    // The function is called exactly once with a string_view valid only for the duration of the call.
    // Default: no op / delivers an empty view
    virtual void get_output(std::function<void(std::string_view)> const& callback) const { callback({}); }

    virtual void clear() {
        // Clear accumulated output. Default: no-op.
    }
};

} // namespace common::logging

using common::logging::LogEvent;
using common::logging::Logger;

} // namespace power_grid_model
