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
