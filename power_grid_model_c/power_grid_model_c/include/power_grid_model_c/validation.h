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
 * @brief Validate node and line components in a non-batch input dataset.
 *
 * Invalid model data is returned as findings and does not set a handle error. Invalid arguments and internal
 * failures return NULL and set the handle error. The returned result is caller-owned and independent of the input
 * dataset lifetime; release it with PGM_destroy_validation_result(). Other components are ignored. The borrowed options
 * object must be non-NULL and its `symmetric` setting must be 0 or 1. Symmetry controls whether the line zero-sequence
 * fields (r0, x0, c0, tan0) are required. Only `node` and `line` components are validated.
 */
PGM_API PGM_ValidationResult* PGM_validate_input_data(PGM_Handle* handle, PGM_ConstDataset const* dataset,
                                                      PGM_Options const* options) PGM_NOEXCEPT;

/**
 * @brief Validate node and line input data for each scenario in a batch update dataset.
 *
 * The input dataset must be non-batch and the update dataset must be batch. Each scenario is applied independently
 * to the input data. Node updates validate IDs but do not change node values; line updates may change only
 * `from_status` and `to_status`. Empty update scenarios validate the unchanged input. A batch with zero scenarios
 * returns an empty report without checking input data. Other components are ignored; only `node` and `line` are
 * validated.
 *
 * The returned result is caller-owned; release it with PGM_destroy_validation_result(). The borrowed options object
 * must be non-NULL and its `symmetric` setting must be 0 or 1. Symmetry controls whether the line zero-sequence fields
 * (r0, x0, c0, tan0) are required. Invalid model data is returned as findings; invalid arguments and internal failures
 * return NULL and set the handle error.
 */
PGM_API PGM_ValidationResult* PGM_validate_batch_data(PGM_Handle* handle, PGM_ConstDataset const* input_dataset,
                                                      PGM_ConstDataset const* update_dataset,
                                                      PGM_Options const* options) PGM_NOEXCEPT;

/**
 * @brief Return the failure-only validation report as compact UTF-8 JSON.
 *
 * Reports use schema version 3 and contain `validation_type`, `valid`, and a `results` array. Only scopes with findings
 * are returned, each with a `scenario` and non-empty `issues` array. Input validation returns no results when the input is valid. Batch
 * validation returns a failed base-input result first when applicable, followed by failed scenarios in scenario order.
 * Base ID failures are reported on the base result and prevent scenario checks. A zero-scenario batch returns an empty
 * results array and does not validate input values. `valid: true` covers validated components only; ignored components
 * are not assessed.
 *
 * Both output pointers are required. Any non-null output is reset to NULL/0 before checking arguments. The bytes are
 * borrowed from the result, size excludes the terminating NUL, and the data remains valid until the result is
 * destroyed.
 */
PGM_API void PGM_validation_result_get_json(PGM_Handle* handle, PGM_ValidationResult* result, char const** data,
                                            PGM_Idx* size) PGM_NOEXCEPT;

/** @brief Destroy a result returned by a validation function. */
PGM_API void PGM_destroy_validation_result(PGM_ValidationResult* result) PGM_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif
