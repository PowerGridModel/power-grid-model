// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @brief header file which includes handle functions
 *
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_HANDLE_H
#define POWER_GRID_MODEL_C_HANDLE_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new handle.
 *
 * A handle object is needed to store error information.
 * If you run it in multi-threading at user side, each thread should have unique handle.
 * The handle should be destroyed by PGM_destroy_handle().
 *
 * @return A pointer to the created handle.
 */
PGM_API PGM_Handle* PGM_create_handle(void);

/**
 * @brief Destroy the handle.
 *
 * @param handle The pointer to the handle created by PGM_create_handle().
 */
PGM_API void PGM_destroy_handle(PGM_Handle* handle);

/**
 * @brief Get error code of last operation.
 *
 * The behavior is implementation-defined if the handle is NULL.
 *
 * @param handle The pointer to the handle you just used for an operation.
 * @return The error code, see #PGM_ErrorCode .
 */
PGM_API PGM_Idx PGM_error_code(PGM_Handle const* handle);

/**
 * @brief Get error message of last operation.
 *
 * If the error code is PGM_batch_error.
 * Use PGM_n_failed_scenarios(), PGM_failed_scenarios(), and PGM_batch_errors() to retrieve the detail.
 *
 * The behavior is implementation-defined if the handle is NULL.
 *
 * @param handle The pointer to the handle you just used for an operation.
 * @return A char const* poiner to a zero terminated string.
 * The pointer is not valid if you execute another operation.
 * You need to copy the string in your own data.
 */
PGM_API char const* PGM_error_message(PGM_Handle const* handle);

/**
 * @brief Get the number of failed scenarios. Only applicable when you just executed a batch calculation.
 *
 * The behavior is implementation-defined if the handle is NULL.
 *
 * @param handle The pointer to the handle you just used for a batch calculation.
 * @return The number of failed scenarios.
 */
PGM_API PGM_Idx PGM_n_failed_scenarios(PGM_Handle const* handle);

/**
 * @brief Get the list of failed scenarios, Only applicable when you just execute a batch calculation.
 *
 * The behavior is implementation-defined if the handle is NULL.
 *
 * @param handle The pointer to the handle you just used for a batch calculation.
 * @return A pointer to a PGM_Idx array with length returned by PGM_n_failed_scenarios().
 * The pointer is not valid if you execute another operation.
 * You need to copy the array in your own data.
 */
PGM_API PGM_Idx const* PGM_failed_scenarios(PGM_Handle const* handle);

/**
 * @brief Get the list of batch errors. Only applicable when you just execute a batch calculation.
 *
 * The behavior is implementation-defined if the handle is NULL.
 *
 * @param handle The pointer to the handle you just used for a batch calculation.
 * @return A pointer to a char const* array with length returned by PGM_n_failed_scenarios().
 * Each entry is a zero terminated string.
 * The pointer is not valid if you execute another operation.
 * You need to copy the array (and the string) in your own data.
 */
PGM_API char const** PGM_batch_errors(PGM_Handle const* handle);

/**
 * @brief Clear and reset the handle.
 *
 * The behavior is implementation-defined if the handle is NULL.
 *
 * @param handle The pointer to the handle.
 */
PGM_API void PGM_clear_error(PGM_Handle* handle);

/**
 * @brief Get the stop state of the handle.
 *
 * Concurrent calls to PGM_stop_requested(), PGM_request_stop() and PGM_clear_stop_requests() are guaranteed to be
 * atomic, but may still lead to unexpected behavior like ignored stop requests.
 *
 * The behavior is implementation-defined if the handle is NULL.
 *
 * @param handle The pointer to the handle.
 * @return 1 if a stop has been requested, 0 otherwise.
 */
PGM_API PGM_Idx PGM_stop_requested(PGM_Handle* handle);

/**
 * @brief Request a stop for all existing and future operations using the handle.
 *
 * The request needs to be manually reset using PGM_clear_stop_requests().
 *
 * This is a cooperative mechanism to request the stop of some long-running operations, such as calculations. Contrary
 * to other operations involving the same handle, this function is designed to be called from another thread.
 *
 * Concurrent calls to PGM_stop_requested(), PGM_request_stop() and PGM_clear_stop_requests() are guaranteed to be
 * atomic, but may still lead to unexpected behavior like ignored stop requests.
 *
 * The behavior is implementation-defined if the handle is NULL.
 *
 * @param handle The pointer to the handle.
 */
PGM_API void PGM_request_stop(PGM_Handle* handle);

/**
 * @brief Clear the stop request for all existing and future operations using the handle.
 *
 * This function is designed to be called from the same thread as a long-running operation, such as calculations.
 *
 * Concurrent calls to PGM_stop_requested(), PGM_request_stop() and PGM_clear_stop_requests() are guaranteed to be
 * atomic, but may still lead to unexpected behavior like ignored stop requests.
 *
 * The behavior is implementation-defined if the handle is NULL.
 *
 * @param handle The pointer to the handle.
 */
PGM_API void PGM_clear_stop_requests(PGM_Handle* handle);

#ifdef __cplusplus
}
#endif

#endif
