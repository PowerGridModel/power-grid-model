// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include "input_sanitization.hpp"
#include "safe_memory_handling.hpp"

#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/logger.h"

#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/common/exception.hpp>
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

inline PGM_Logger* make_logger(PGM_Idx type) {
    using namespace power_grid_model::common::logging;

    switch (type) {
    case PGM_text_logger:
        return create<PGM_Logger>(std::make_shared<MultiThreadedTextLogger>());
    case PGM_benchmark_logger:
        return create<PGM_Logger>(std::make_shared<MultiThreadedCalculationInfo>());
    default:
        throw power_grid_model::MissingCaseForEnumError{"make_logger", type};
    }
}

template <typename Callback, typename UserData>
inline void logger_get_output(PGM_Logger const& pgm_logger, Callback callback, UserData user_data) {
    pgm_logger.logger->get_output([callback, user_data](std::string_view sv) {
        callback(sv.data(), safe_cast<PGM_Idx>(sv.size()),
                 user_data); // NOLINT(bugprone-suspicious-stringview-data-usage)
    });
}

inline void logger_clear(PGM_Logger const& pgm_logger) { pgm_logger.logger->clear(); }

} // namespace power_grid_model_c
