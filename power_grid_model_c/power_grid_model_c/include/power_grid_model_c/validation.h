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
 * @return A report instance that must be freed with PGM_destroy_validation_report().
 */
PGM_API PGM_ValidationReport* PGM_create_validation_report(PGM_Handle* handle) PGM_NOEXCEPT;

/**
 * @brief Destroy a validation report instance.
 * @param report Report instance to destroy.
 */
PGM_API void PGM_destroy_validation_report(PGM_ValidationReport* report) PGM_NOEXCEPT;

/**
 * @brief Validate an input dataset or batch update scenarios and store the report.
 * @param handle Handle used to report errors.
 * @param report Caller-owned report instance created by PGM_create_validation_report(). Each call replaces its
 * previous contents, including when validation fails.
 * @param input_dataset Input dataset to validate, or the base dataset for batch update scenarios.
 * @param update_dataset Batch update dataset to validate. Pass nullptr to validate only the input dataset.
 * Chained (multi-dimensional) update datasets are unsupported.
 * @param options Calculation options used during validation.
 * @return 0 if validation passes; 1 if validation finds issues. If the call fails, check the handle for errors before
 * interpreting this status. Retrieve validation details with PGM_validation_report_get_json.
 */
PGM_API PGM_Idx PGM_validate_data(PGM_Handle* handle, PGM_ValidationReport* report,
                                  PGM_ConstDataset const* input_dataset, PGM_ConstDataset const* update_dataset,
                                  PGM_Options const* options) PGM_NOEXCEPT;

/**
 * @brief Get a validation report as borrowed JSON data.
 * @param handle Handle used to report errors.
 * @param report Report containing the data.
 * @param data Output pointer to the JSON bytes, or null if no report is available.
 * @param size Output JSON size in bytes, excluding the null terminator.
 * @return No value; check the handle for errors.
 * @note The borrowed data remains valid until the report is validated again or destroyed.
 */
PGM_API void PGM_validation_report_get_json(PGM_Handle* handle, PGM_ValidationReport* report, char const** data,
                                            PGM_Idx* size) PGM_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif
