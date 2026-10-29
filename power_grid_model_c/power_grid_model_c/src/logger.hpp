// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logger_fwd.hpp"

#include <power_grid_model/common/exception.hpp>

#include <memory>

namespace power_grid_model_c {
[[nodiscard]] HandleLogger make_handle_logger();

template <typename PGMModule, typename Logger>
concept logger_scoped_module_c = requires(PGMModule& pgm_module, Logger& logger) {
    pgm_module.set_logger(logger);
    { pgm_module.reset_logger() } noexcept;
    { pgm_module.logger_empty() } noexcept;
};

/** Temporarily sets a logger on a module and resets it when this guard is destroyed.
 * @tparam PGMModule The type of the module on which the logger will be set. Like Model, Dataset, etc.
 * @tparam Logger The logger to set / reset on the module
 *
 * The module must not already have a logger set; otherwise construction throws
 * `power_grid_model::UnreachableHit`.
 * This guard ensures that the logger is properly reset all the time.
 */
template <typename PGMModule, typename Logger>
    requires logger_scoped_module_c<PGMModule, Logger>
class ScopedModuleLogger {
  public:
    ScopedModuleLogger(PGMModule& pgm_module, Logger& logger) : pgm_module_{pgm_module}, logger_{logger} {
        if (!pgm_module_.logger_empty()) {
            throw power_grid_model::UnreachableHit{"ScopedModuleLogger", "module has no logger set"};
        }
        pgm_module_.set_logger(logger_);
    }
    ScopedModuleLogger(ScopedModuleLogger const&) = delete;
    ScopedModuleLogger& operator=(ScopedModuleLogger const&) = delete;
    ScopedModuleLogger(ScopedModuleLogger&&) = delete;
    ScopedModuleLogger& operator=(ScopedModuleLogger&&) = delete;
    ~ScopedModuleLogger() noexcept { pgm_module_.reset_logger(); }

  private:
    PGMModule& pgm_module_;
    Logger& logger_;
};
} // namespace power_grid_model_c
