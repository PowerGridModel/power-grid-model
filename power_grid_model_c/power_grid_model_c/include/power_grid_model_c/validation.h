#pragma once
#ifndef POWER_GRID_MODEL_C_VALIDATION_H
#define POWER_GRID_MODEL_C_VALIDATION_H

#include "basics.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Validate an input dataset and store its validation report.
 * @param handle Handle used for the operation and to store the report.
 * @param input_dataset Input dataset to validate.
 * @param options Calculation options used during validation.
 * @return True if the dataset is valid; otherwise false. Retrieve details with PGM_validation_report_get_json.
 */
PGM_API bool PGM_validate_input_data(PGM_Handle* handle, PGM_ConstDataset const* input_dataset,
                                     PGM_Options const* options) PGM_NOEXCEPT;

/**
 * @brief Validate batch update scenarios and store their validation report.
 * @param handle Handle used for the operation and to store the report.
 * @param input_dataset Input dataset used as the base for the update scenarios.
 * @param update_dataset Batch update dataset to validate.
 * @param options Calculation options used during validation.
 * @return True if all scenarios are valid; otherwise false. Retrieve details with PGM_validation_report_get_json.
 */
PGM_API bool PGM_validate_batch_data(PGM_Handle* handle, PGM_ConstDataset const* input_dataset,
                                     PGM_ConstDataset const* update_dataset, PGM_Options const* options) PGM_NOEXCEPT;

/**
 * @brief Get the latest validation report as borrowed JSON data.
 * @param handle Handle containing the report.
 * @param data Output pointer to the JSON bytes, or null if no report is available.
 * @param size Output JSON size in bytes, excluding the null terminator.
 * @return No value; check the handle for errors.
 * @note The borrowed data remains valid until the next validation call or handle destruction.
 */
PGM_API void PGM_validation_report_get_json(PGM_Handle* handle, char const** data, PGM_Idx* size) PGM_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif
