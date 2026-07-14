// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include "input_sanitization.hpp"

#include "power_grid_model_c/basics.h"

#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/multi_threaded_logging.hpp>
#include <power_grid_model/common/text_logger.hpp>

#include <memory>
#include <string>

// The PGM_Logger struct is the C API wrapper for a polymorphic multi-threaded logger.
// It is heap-allocated by PGM_create_logger and freed by PGM_destroy_logger.
struct PGM_Logger {
    PGM_LoggerType type;
    std::unique_ptr<power_grid_model::common::logging::MultiThreadedLogger> logger;
    std::string output_buffer; // stable string storage returned by PGM_logger_get_output
};

namespace power_grid_model_c {

inline PGM_Logger* make_logger(PGM_LoggerType type) {
    using namespace power_grid_model::common::logging;

    switch (type) {
    case PGM_do_nothing_logger:
        return new PGM_Logger{type, std::make_unique<NoMultiThreadedLogger>(), {}}; // NOSONAR(S5025)
    case PGM_text_logger:
        return new PGM_Logger{type, std::make_unique<MultiThreadedTextLogger>(), {}}; // NOSONAR(S5025)
    case PGM_benchmark_logger:
        return new PGM_Logger{type, std::make_unique<MultiThreadedCalculationInfo>(), {}}; // NOSONAR(S5025)
    default:
        throw IllegalOperationError{std::format("Unknown logger type: {}", static_cast<int>(type))};
    }
}

inline char const* logger_get_output(PGM_Logger& pgm_logger) {
    using namespace power_grid_model::common::logging;

    switch (pgm_logger.type) {
    case PGM_text_logger:
        pgm_logger.output_buffer =
            static_cast<MultiThreadedTextLogger&>(*pgm_logger.logger).report();
        break;
    case PGM_benchmark_logger:
        pgm_logger.output_buffer =
            static_cast<MultiThreadedCalculationInfo&>(*pgm_logger.logger).string_report();
        break;
    default: // PGM_do_nothing_logger and any future no-op types
        pgm_logger.output_buffer.clear();
        break;
    }
    return pgm_logger.output_buffer.c_str();
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
