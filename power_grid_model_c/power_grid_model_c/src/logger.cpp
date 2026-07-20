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
        handle->composite_logger.add(safe_ptr_get(logger).logger.get());
    });
}

void PGM_unregister_logger(PGM_Handle* handle, PGM_Logger* logger) {
    call_with_catch(handle, [handle, logger] {
        handle->composite_logger.remove(safe_ptr_get(logger).logger.get());
        // Intentional no-op when the logger is not registered (documented in logger.h).
    });
}

void PGM_unregister_all_loggers(PGM_Handle* handle) {
    call_with_catch(handle, [handle] { safe_ptr_get(handle).composite_logger.reset(); });
}

void PGM_logger_get_output(PGM_Handle* handle, PGM_Logger* logger, PGM_LogOutputCallback callback,
                           void* user_data) {
    call_with_catch(handle, [logger, callback, user_data] {
        logger_get_output(safe_ptr_get(logger), callback, user_data);
    });
}

void PGM_logger_clear(PGM_Handle* handle, PGM_Logger* logger) {
    call_with_catch(handle, [logger] { logger_clear(safe_ptr_get(logger)); });
}
