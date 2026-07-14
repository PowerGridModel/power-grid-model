// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS

#include "handle.hpp"
#include "logger.hpp"

#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/logger.h"

namespace {
using power_grid_model_c::call_with_catch;
using power_grid_model_c::logger_clear;
using power_grid_model_c::logger_get_output;
using power_grid_model_c::make_logger;
using power_grid_model_c::safe_enum;
using power_grid_model_c::safe_ptr;
using power_grid_model_c::safe_ptr_get;
} // namespace

PGM_Logger* PGM_create_logger(PGM_Handle* handle, PGM_Idx logger_type) {
    return call_with_catch(handle, [logger_type] {
        return make_logger(safe_enum<PGM_LoggerType>(logger_type));
    });
}

void PGM_destroy_logger(PGM_Logger* logger) {
    delete logger; // NOSONAR(S5025)
}

void PGM_register_logger(PGM_Handle* handle, PGM_Logger* logger) {
    call_with_catch(handle, [handle, logger] {
        safe_ptr_get(logger); // null check
        handle->loggers.push_back(logger);
    });
}

void PGM_unregister_logger(PGM_Handle* handle, PGM_Logger* logger) {
    call_with_catch(handle, [handle, logger] {
        auto& loggers = handle->loggers;
        auto it = std::ranges::find(loggers, logger);
        if (it != loggers.end()) {
            loggers.erase(it);
        }
        // TODO (nitbharambe): not found, no-op for now, but should we throw an error? (like in unregistering a dataset)
    });
}

char const* PGM_logger_get_output(PGM_Handle* handle, PGM_Logger* logger) {
    return call_with_catch(handle, [logger]() -> char const* {
        return logger_get_output(safe_ptr_get(logger));
    });
}

void PGM_logger_clear(PGM_Handle* handle, PGM_Logger* logger) {
    call_with_catch(handle, [logger] { logger_clear(safe_ptr_get(logger)); });
}
