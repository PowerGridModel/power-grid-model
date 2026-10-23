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
 * @brief Destroy a validation report instance.
 * @param report Report instance to destroy.
 */
PGM_API void PGM_destroy_validation_report(PGM_ValidationReport* report) PGM_NOEXCEPT;

/**
 * @brief Validate an input dataset or batch update scenarios and return the report.
 * @param handle Handle used to report errors.
 * @param input_dataset Input dataset to validate, or the base dataset for batch update scenarios.
 * @param update_dataset Batch update dataset to validate. Pass nullptr to validate only the input dataset.
 * Chained (multi-dimensional) update datasets are unsupported.
 * @param options Calculation options used during validation.
 * @return A report if validation completes, even if it finds issues. The report must be freed with
 * PGM_destroy_validation_report(). Returns nullptr if the call fails; check the handle for errors.
 */
PGM_API PGM_ValidationReport* PGM_validate_data(PGM_Handle* handle, PGM_ConstDataset const* input_dataset,
                                                PGM_ConstDataset const* update_dataset,
                                                PGM_Options const* options) PGM_NOEXCEPT;

/**
 * @brief Check whether a validation report found no issues.
 * @param handle Handle used to report errors.
 * @param report Report returned by PGM_validate_data().
 * @return 1 if validation found no issues; 0 if it found issues. If the call fails, check the handle for errors before
 * interpreting this value.
 */
PGM_API PGM_Idx PGM_validation_report_is_valid(PGM_Handle* handle, PGM_ValidationReport* report) PGM_NOEXCEPT;

/**
 * @brief Get a validation report as a zero-terminated JSON string.
 * @param handle Handle used to report errors.
 * @param report Report returned by PGM_validate_data().
 * @return Borrowed JSON string, or nullptr if the call fails; check the handle for errors.
 * @note The string remains valid until the report is destroyed.
 */
PGM_API char const* PGM_validation_report_get_to_zero_terminated_string(PGM_Handle* handle,
                                                                        PGM_ValidationReport* report) PGM_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif
