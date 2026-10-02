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

enum PGM_ValidationIssueKind {
    PGM_validation_issue_none = 0,
    PGM_validation_issue_missing_value = 1,
    PGM_validation_issue_infinity = 2,
    PGM_validation_issue_not_unique = 3,
    PGM_validation_issue_not_greater_than_zero = 4,
};

/**
 * @brief Borrowed view of one validation finding.
 *
 * The ids array and strings remain valid until the owning PGM_ValidationResult is destroyed.
 */
typedef struct PGM_ValidationIssue {
    enum PGM_ValidationIssueKind kind;
    char const* component;
    char const* field;
    PGM_Idx n_ids;
    PGM_ID const* ids;
} PGM_ValidationIssue;

/**
 * @brief Validate a non-batch input dataset containing no component other than node.
 *
 * Invalid model data is returned as findings and does not set a handle error. Invalid arguments and internal
 * failures return NULL and set the handle error. The returned result is caller-owned and independent of the input
 * dataset lifetime; release it with PGM_destroy_validation_result().
 */
PGM_API PGM_ValidationResult* PGM_validate_input_data(PGM_Handle* handle, PGM_ConstDataset const* dataset) PGM_NOEXCEPT;

/** @brief Return the number of findings in a validation result. */
PGM_API PGM_Idx PGM_validation_result_issue_count(PGM_Handle* handle, PGM_ValidationResult const* result) PGM_NOEXCEPT;

/**
 * @brief Return a borrowed view of a finding by index.
 *
 * An invalid result or index returns a zero-initialized view and sets the handle error.
 */
PGM_API PGM_ValidationIssue PGM_validation_result_get_issue(PGM_Handle* handle, PGM_ValidationResult const* result,
                                                            PGM_Idx issue_idx) PGM_NOEXCEPT;

/** @brief Destroy a result returned by PGM_validate_input_data(). */
PGM_API void PGM_destroy_validation_result(PGM_ValidationResult* result) PGM_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif
