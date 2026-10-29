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
    build_model = 1000,

    // per-thread/per-scenario scope (job_dispatch.hpp)
    total_single_calculation_in_thread = 1100,
    total_batch_calculation_in_thread = 1200,
    copy_model = 1220,
    update_model = 1230,
    restore_model = 1231, // sub-step of update_model
    scenario_exception = 1240,
    recover_from_bad = 1250,

    // calculation preparation
    prepare = 2000,

    // solving
    math_calculation = 3000,
    create_math_solver = 3100,
    math_solver = 3200,

    // math solver run: initialization
    solver_initialization = 3210,
    initialize_calculation = 3211,
    preprocess_measured_value = 3212,
    initialize_voltages = 3213,

    // math solver run: equation preparation
    matrix_equation_preparation = 3230,
    prepare_matrix = 3231,
    prepare_matrix_including_prefactorization = 3232,
    prepare_matrices = 3233,
    calculate_rhs = 3234,
    prepare_lhs_rhs = 3235,

    // math solver run: equation solving
    solve_equation = 3240,
    solve_sparse_linear_equation = 3241,
    solve_sparse_linear_equation_prefactorized = 3242,

    // math solver run: outer loop, one iteration of the solver's main loop
    solver_iteration = 3250,
    iterate_unknown = 3251,
    max_num_iter = 3252,
    iterative_pf_solver_max_num_iter = 3253,

    // math solver run: result calculation
    calculate_math_result = 3290,

    // output creationg
    produce_output = 4000,
};

template <typename Fn>
concept LazyLoggingFn = std::invocable<Fn> && std::convertible_to<std::invoke_result_t<Fn>, std::string> &&
                        (!std::convertible_to<Fn, std::string_view>) && functor_c<Fn>;

class Logger {
  public:
    // Returns false only if the event is guaranteed to be discarded, so that callers can skip constructing
    // potentially expensive messages. A false positive only costs performance; a false negative loses logs.
    [[nodiscard]] virtual bool should_log(LogEvent /*tag*/) const { return true; }

    virtual void log(LogEvent tag) = 0;
    virtual void log(LogEvent tag, std::string_view message) = 0;
    virtual void log(LogEvent tag, double value) = 0;
    virtual void log(LogEvent tag, Idx value) = 0;

    template <LazyLoggingFn Fn> void log(LogEvent tag, Fn fn) {
        if (should_log(tag)) {
            log(tag, std::invoke(fn));
        }
    }

    void log(std::string_view message) { log(LogEvent::unknown, message); }
    template <LazyLoggingFn Fn> void log(Fn fn) { log(LogEvent::unknown, fn); }

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

    virtual void clear_content() {
        // Clear accumulated content. Default: no-op.
    }
};

} // namespace common::logging

using common::logging::LogEvent;
using common::logging::Logger;

} // namespace power_grid_model
