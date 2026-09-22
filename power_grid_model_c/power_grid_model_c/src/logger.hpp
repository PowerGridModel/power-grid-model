// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logger_fwd.hpp"

#include <power_grid_model/common/exception.hpp>

#include <memory>

namespace power_grid_model_c {
[[nodiscard]] HandleLogger make_handle_logger();

template <typename Module, typename Logger>
concept logger_scoped_module_c = requires(Module& module, Logger& logger) {
    module.set_logger(logger);
    { module.reset_logger() } noexcept;
    { module.logger_empty() } noexcept;
};

template <typename Module, typename Logger>
    requires logger_scoped_module_c<Module, Logger>
class ScopedModuleLogger {
  public:
    ScopedModuleLogger(Module& module, Logger& logger) : module_{module}, logger_{logger} {
        if (!module_.logger_empty()) {
            throw power_grid_model::UnreachableHit{"ScopedModuleLogger", "module has no logger set"};
        }
        module_.set_logger(logger_);
    }
    ScopedModuleLogger(ScopedModuleLogger const&) = delete;
    ScopedModuleLogger& operator=(ScopedModuleLogger const&) = delete;
    ScopedModuleLogger(ScopedModuleLogger&&) = delete;
    ScopedModuleLogger& operator=(ScopedModuleLogger&&) = delete;
    ~ScopedModuleLogger() noexcept { module_.reset_logger(); }

  private:
    Module& module_;
    Logger& logger_;
};
} // namespace power_grid_model_c
