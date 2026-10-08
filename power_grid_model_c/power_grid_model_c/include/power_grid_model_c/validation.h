#pragma once
#ifndef POWER_GRID_MODEL_C_VALIDATION_H
#define POWER_GRID_MODEL_C_VALIDATION_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Experimental result handle for input validation.
 *
 * This API is a prototype; its declarations and result shape are not a stable ABI.
 */
typedef struct PGM_ValidationResult PGM_ValidationResult;

/**
 * @brief Validate a non-batch input dataset containing only node and line components.
 *
 * Invalid model data is returned as findings and does not set a handle error. Invalid arguments and internal
 * failures return NULL and set the handle error. The returned result is caller-owned and independent of the input
 * dataset lifetime; release it with PGM_destroy_validation_result(). `symmetric` must be 0 or 1 and controls whether
 * the line zero-sequence fields (r0, x0, c0, tan0) are required. Only `node` and `line` components are supported.
 */
PGM_API PGM_ValidationResult* PGM_validate_input_data(PGM_Handle* handle, PGM_ConstDataset const* dataset,
                                                      PGM_Idx symmetric) PGM_NOEXCEPT;

/**
 * @brief Return 1 if validation passed, or 0 if the result contains findings.
 *
 * An invalid result returns 0 and sets the handle error.
 */
PGM_API PGM_Idx PGM_validation_result_passed(PGM_Handle* handle, PGM_ValidationResult const* result) PGM_NOEXCEPT;

/**
 * @brief Return the complete validation report as compact UTF-8 JSON.
 *
 * Both output pointers are required. Any non-null output is reset to NULL/0 before checking arguments. The bytes are
 * borrowed from the result, size excludes the terminating NUL, and the data remains valid until the result is
 * destroyed.
 */
PGM_API void PGM_validation_result_get_json(PGM_Handle* handle, PGM_ValidationResult const* result, char const** data,
                                            PGM_Idx* size) PGM_NOEXCEPT;

/** @brief Destroy a result returned by PGM_validate_input_data(). */
PGM_API void PGM_destroy_validation_result(PGM_ValidationResult* result) PGM_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif
