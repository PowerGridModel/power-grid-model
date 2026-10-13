#pragma once
#ifndef POWER_GRID_MODEL_C_VALIDATION_H
#define POWER_GRID_MODEL_C_VALIDATION_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Validate an input dataset and store its report in the handle. */
PGM_API void PGM_validate_input_data(PGM_Handle* handle, PGM_ConstDataset const* dataset,
                                     PGM_Options const* options) PGM_NOEXCEPT;

/** @brief Validate batch update scenarios and store the report in the handle. */
PGM_API void PGM_validate_batch_data(PGM_Handle* handle, PGM_ConstDataset const* input_dataset,
                                     PGM_ConstDataset const* update_dataset,
                                     PGM_Options const* options) PGM_NOEXCEPT;

/** @brief Get borrowed JSON data, valid until the next validation call or handle destruction. */
PGM_API void PGM_validation_report_get_json(PGM_Handle* handle, char const** data, PGM_Idx* size) PGM_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif