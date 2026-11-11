// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_C_VALIDATION_H
#define POWER_GRID_MODEL_C_VALIDATION_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a validation report instance.
 * @param handle Handle used to report errors.
 * @return An empty report. Returns NULL if the call fails; check the handle for errors.
 */
PGM_API PGM_ValidationReport* PGM_create_validation_report(PGM_Handle* handle) PGM_NOEXCEPT;

/**
 * @brief Destroy a validation report instance.
 * @param report Report instance created by PGM_create_validation_report().
 */
PGM_API void PGM_destroy_validation_report(PGM_ValidationReport* report) PGM_NOEXCEPT;

/**
 * @brief Validate an input dataset or batch update scenarios and store the result in a report.
 * @param handle Handle used to report errors.
 * @param report Report instance created by PGM_create_validation_report(). On success, its contents are replaced
 * with the validation results.
 * @param input_dataset Input dataset to validate, or the base dataset for batch update scenarios.
 * @param update_dataset Batch update dataset to validate. Pass NULL to validate only the input dataset.
 * Chained (multi-dimensional) update datasets are unsupported.
 * @param options Calculation options used during validation.
 * @note If the call fails, check the handle for errors.
 */
PGM_API void PGM_validate_data(PGM_Handle* handle, PGM_ValidationReport* report, PGM_ConstDataset const* input_dataset,
                               PGM_ConstDataset const* update_dataset, PGM_Options const* options) PGM_NOEXCEPT;

/**
 * @brief Check whether a validation report passed.
 * @param handle Handle used to report errors.
 * @param report Report populated by PGM_validate_data().
 * @return PGM_validation_pass if validation found no issues; If the call fails, check the handle for errors.
 */
PGM_API PGM_Idx PGM_check_validation_report(PGM_Handle* handle, PGM_ValidationReport* report) PGM_NOEXCEPT;

/**
 * @brief Get a validation report as a zero-terminated JSON string.
 * @param handle Handle used to report errors.
 * @param report Report populated by PGM_validate_data().
 * @return A NULL-terminated json string.
 *     Returns NULL if errors occurred (check the handle for error information).
 * @note The string remains valid until the report is destroyed.
 * @note Concurrent calls to this function using the same report must be externally synchronized.
 */
PGM_API char const* PGM_validation_report_get_to_zero_terminated_string(PGM_Handle* handle,
                                                                        PGM_ValidationReport* report) PGM_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif
