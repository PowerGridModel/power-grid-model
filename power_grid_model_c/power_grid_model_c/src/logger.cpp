// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS

#include "logger.hpp"
#include "handle.hpp"
#include "input_sanitization.hpp"
#include "logger_fwd.hpp"
#include "safe_memory_handling.hpp"

#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/logger.h"

#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/common/composite_logging.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/text_logger.hpp>

#include <memory>

// The PGM_Logger struct is the C API wrapper for a polymorphic multi-threaded logger.
// It is heap-allocated by PGM_create_logger and freed by PGM_destroy_logger.
// The underlying logger implementation is shared with any handle it is registered to (see
// MultiThreadedCompositeLogger), so destroying this wrapper while still registered does not
// invalidate the implementation: it stays alive as long as any registration references it.
struct PGM_Logger {
    std::shared_ptr<power_grid_model_c::MultiThreadedLogger> logger;
};

namespace {
using power_grid_model_c::call_with_catch;
using power_grid_model_c::create;
using power_grid_model_c::destroy;
using power_grid_model_c::safe_cast;
using power_grid_model_c::safe_ptr;
using power_grid_model_c::safe_ptr_get;

using power_grid_model::common::logging::MultiThreadedCompositeLogger;
using power_grid_model_c::HandleLogger;
} // namespace

namespace power_grid_model_c {
[[nodiscard]] MultiThreadedLogger& get_logger(HandleLogger& handle_logger) { return safe_ptr_get(handle_logger.get()); }

[[nodiscard]] HandleLogger make_handle_logger() {
    return HandleLogger{create<MultiThreadedCompositeLogger>(), [](MultiThreadedLogger* logger) { destroy(logger); }};
}
} // namespace power_grid_model_c

namespace {
MultiThreadedCompositeLogger& extract_handle_logger(HandleLogger& handle_logger) {
    return dynamic_cast<MultiThreadedCompositeLogger&>(power_grid_model_c::get_logger(handle_logger));
}

PGM_Logger* make_logger(PGM_Idx type) {
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
void logger_get_output(PGM_Logger const& pgm_logger, Callback callback, UserData user_data) {
    pgm_logger.logger->get_output([callback, user_data](std::string_view sv) {
        callback(sv.data(), // NOLINT(bugprone-suspicious-stringview-data-usage) // false positive: size() is provided
                 safe_cast<PGM_Idx>(sv.size()), user_data);
    });
}

void logger_clear(PGM_Logger const& pgm_logger) { pgm_logger.logger->clear(); }
} // namespace

PGM_Logger* PGM_create_logger(PGM_Handle* handle, PGM_Idx logger_type) {
    return call_with_catch(handle, [logger_type] { return make_logger(logger_type); });
}

void PGM_destroy_logger(PGM_Logger* logger) { destroy(logger); }

void PGM_register_logger(PGM_Handle* handle, PGM_Logger* logger) {
    call_with_catch(handle, [handle, logger] {
        extract_handle_logger(safe_ptr_get(handle).logger).add(safe_ptr_get(logger).logger);
    });
}

void PGM_unregister_logger(PGM_Handle* handle, PGM_Logger* logger) {
    call_with_catch(handle, [handle, logger] {
        extract_handle_logger(safe_ptr_get(handle).logger).remove(safe_ptr_get(logger).logger.get());
    });
}

void PGM_unregister_all_loggers(PGM_Handle* handle) {
    call_with_catch(handle, [handle] { extract_handle_logger(safe_ptr_get(handle).logger).reset(); });
}

void PGM_logger_get_output(PGM_Handle* handle, PGM_Logger* logger, PGM_LogOutputCallback callback, // NOSONAR(S5205)
                           void* user_data) {
    call_with_catch(handle, [logger, callback, user_data] {
        logger_get_output(safe_ptr_get(logger), safe_ptr(callback), user_data);
    });
}

void PGM_logger_clear(PGM_Handle* handle, PGM_Logger* logger) {
    call_with_catch(handle, [logger] { logger_clear(safe_ptr_get(logger)); });
}
