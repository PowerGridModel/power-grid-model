// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @brief Header file which includes logger functions.
 *
 * Loggers provide opt-in diagnostic output from Power Grid Model calculations.
 * They are designed for advanced users: output is non-conclusive and intended as debugging hints.
 *
 * Lifecycle:
 *   1. Create a logger:   PGM_create_logger()
 *   2. Register it:       PGM_register_logger()
 *   3. Run calculations.
 *   4. Read output:       PGM_logger_get_output()
 *   5. Optionally clear:  PGM_logger_clear()
 *   6. Unregister it:     PGM_unregister_logger()
 *   7. Destroy it:        PGM_destroy_logger()
 *
 * Undefined behaviour:
 *   - Calling PGM_destroy_logger() while the logger is still registered.
 *   - Using the same logger from multiple user threads simultaneously
 *     (internal batch threads spawned by the calculation core are safe).
 *
 * Idempotent operations:
 *   - Registering the same logger to the same handle more than once is a no-op.
 *   - Unregistering a logger that is not registered is a no-op.
 *
 * Multiple loggers of different types may be registered to the same handle simultaneously.
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_LOGGER_H
#define POWER_GRID_MODEL_C_LOGGER_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new logger of the specified type.
 *
 * @param handle The handle used to report errors.
 * @param logger_type The type of logger to create (see #PGM_LoggerType).
 * @return A pointer to the created logger, or NULL on error.
 *   Destroy with PGM_destroy_logger().
 */
PGM_API PGM_Logger* PGM_create_logger(PGM_Handle* handle, PGM_Idx logger_type);

/**
 * @brief Destroy a logger created by PGM_create_logger().
 *
 * The logger must not be registered to any handle when this is called (UB otherwise).
 *
 * @param logger The logger to destroy.
 */
PGM_API void PGM_destroy_logger(PGM_Logger* logger);

/**
 * @brief Register a logger to a handle so it receives output from subsequent calculations.
 *
 * Multiple loggers of different types may be registered simultaneously.
 * Registering the same logger instance twice to the same handle is a no-op (idempotent).
 *
 * @param handle The handle to register to.
 * @param logger The logger to register.
 */
PGM_API void PGM_register_logger(PGM_Handle* handle, PGM_Logger* logger);

/**
 * @brief Unregister a logger from a handle.
 *
 * Unregistering a logger that is not registered to the handle is a no-op.
 *
 * @param handle The handle to unregister from.
 * @param logger The logger to unregister.
 */
PGM_API void PGM_unregister_logger(PGM_Handle* handle, PGM_Logger* logger);

/**
 * @brief Callback type for receiving logger output.
 *
 * Called exactly once by PGM_logger_get_output().
 * The @p data pointer and @p size describe the log content and are valid only
 * for the duration of the callback. Do not store @p data beyond the callback.
 *
 * @param data  Pointer to the log content (not necessarily null-terminated).
 * @param size  Length of the log content in bytes.
 * @param user_data  Opaque pointer passed through from PGM_logger_get_output().
 */
typedef void (*PGM_LogOutputCallback)(char const* data, PGM_Idx size, void* user_data);

/**
 * @brief Deliver the current output of a logger to a caller-supplied callback.
 *
 * The callback is called exactly once, synchronously, before this function returns.
 * The data pointer passed to the callback is valid only for the duration of that call.
 *
 * For #PGM_text_logger: delivers timestamped log lines.
 * For #PGM_benchmark_logger: delivers one line per logged event in the format
 *   EVENT_CODE<TAB>VALUE
 * For #PGM_do_nothing_logger: delivers an empty buffer (size 0).
 *
 * @param handle     The handle used to report errors.
 * @param logger     The logger whose output to retrieve.
 * @param callback   Function called with the log data.
 * @param user_data  Passed through unchanged to @p callback.
 */
PGM_API void PGM_logger_get_output(PGM_Handle* handle, PGM_Logger* logger, PGM_LogOutputCallback callback,
                                   void* user_data);

/**
 * @brief Clear the accumulated output of a logger.
 *
 * For #PGM_do_nothing_logger this is a no-op.
 *
 * @param handle The handle used to report errors.
 * @param logger The logger to clear.
 */
PGM_API void PGM_logger_clear(PGM_Handle* handle, PGM_Logger* logger);

#ifdef __cplusplus
}
#endif

#endif
