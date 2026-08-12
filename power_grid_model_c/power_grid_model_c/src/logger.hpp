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
// The underlying logger implementation is shared with any handle it is registered to (see
// MultiThreadedCompositeLogger), so destroying this wrapper while still registered does not
// invalidate the implementation: it stays alive as long as any registration references it.
struct PGM_Logger {
    std::shared_ptr<power_grid_model::common::logging::MultiThreadedLogger> logger;
};

namespace power_grid_model_c {

inline PGM_Logger* make_logger(PGM_LoggerType type) {
    using namespace power_grid_model::common::logging;

    switch (type) {
    case PGM_do_nothing_logger:
        return new PGM_Logger{std::make_shared<NoMultiThreadedLogger>()}; // NOSONAR(S5025)
    case PGM_text_logger:
        return new PGM_Logger{std::make_shared<MultiThreadedTextLogger>()}; // NOSONAR(S5025)
    case PGM_benchmark_logger:
        return new PGM_Logger{std::make_shared<MultiThreadedCalculationInfo>()}; // NOSONAR(S5025)
    default:
        throw IllegalOperationError{std::format("Unknown logger type: {}", static_cast<int>(type))};
    }
}

inline void logger_get_output(PGM_Logger& pgm_logger, PGM_LogOutputCallback callback, void* user_data) {
    pgm_logger.logger->get_output([&](std::string_view sv) {
        callback(sv.data(), safe_cast<PGM_Idx>(sv.size()), user_data);
    });
}

inline void logger_clear(PGM_Logger& pgm_logger) {
    pgm_logger.logger->clear();
}

} // namespace power_grid_model_c
