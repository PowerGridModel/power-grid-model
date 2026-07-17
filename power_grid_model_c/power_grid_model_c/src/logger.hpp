// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include "input_sanitization.hpp"

#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/logger.h"

#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/multi_threaded_logging.hpp>
#include <power_grid_model/common/text_logger.hpp>

#include <memory>

// The PGM_Logger struct is the C API wrapper for a polymorphic multi-threaded logger.
// It is heap-allocated by PGM_create_logger and freed by PGM_destroy_logger.
struct PGM_Logger {
    PGM_LoggerType type;
    std::unique_ptr<power_grid_model::common::logging::MultiThreadedLogger> logger;
};

namespace power_grid_model_c {

inline PGM_Logger* make_logger(PGM_LoggerType type) {
    using namespace power_grid_model::common::logging;

    switch (type) {
    case PGM_do_nothing_logger:
        return new PGM_Logger{type, std::make_unique<NoMultiThreadedLogger>()}; // NOSONAR(S5025)
    case PGM_text_logger:
        return new PGM_Logger{type, std::make_unique<MultiThreadedTextLogger>()}; // NOSONAR(S5025)
    case PGM_benchmark_logger:
        return new PGM_Logger{type, std::make_unique<MultiThreadedCalculationInfo>()}; // NOSONAR(S5025)
    default:
        throw IllegalOperationError{std::format("Unknown logger type: {}", static_cast<int>(type))};
    }
}

inline void logger_get_output(PGM_Logger& pgm_logger, PGM_LogOutputCallback callback, void* user_data) {
    using namespace power_grid_model::common::logging;

    switch (pgm_logger.type) {
    case PGM_text_logger: {
        auto const view = static_cast<MultiThreadedTextLogger const&>(*pgm_logger.logger).report_view();
        callback(view.data(), static_cast<PGM_Idx>(view.size()), user_data);
        break;
    }
    case PGM_benchmark_logger: {
        auto const report = static_cast<MultiThreadedCalculationInfo const&>(*pgm_logger.logger).string_report();
        callback(report.data(), static_cast<PGM_Idx>(report.size()), user_data);
        break;
    }
    default: // PGM_do_nothing_logger and any future no-op types
        callback("", 0, user_data);
        break;
    }
}

inline void logger_clear(PGM_Logger& pgm_logger) {
    using namespace power_grid_model::common::logging;

    switch (pgm_logger.type) {
    case PGM_text_logger:
        static_cast<MultiThreadedTextLogger&>(*pgm_logger.logger).clear();
        break;
    case PGM_benchmark_logger:
        static_cast<MultiThreadedCalculationInfo&>(*pgm_logger.logger).clear();
        break;
    default: // PGM_do_nothing_logger: no-op
        break;
    }
}

} // namespace power_grid_model_c
