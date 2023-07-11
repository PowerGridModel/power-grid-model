// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @brief header file which includes options functions
 *
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_OPTIONS_H
#define POWER_GRID_MODEL_C_OPTIONS_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create an option instance.
 *
 * The option is needed to run calculations.
 * This function create a new option instance with the following default values:
 *   - calculation_type: PGM_power_flow
 *   - calculation_method: PGM_default_method
 *   - symmetric: 1
 *   - err_tol: 1e-8
 *   - max_iter: 20
 *   - threading: -1
 *
 * @param handle
 * @return a pointer to the option instance. Should be freed by PGM_destroy_options()
 */
PGM_API PGM_Options* PGM_create_options(PGM_Handle* handle);

/**
 * @brief Free the option instance
 *
 * @param opt pointer to an option instance created by PGM_create_options()
 */
PGM_API void PGM_destroy_options(PGM_Options* opt);

/**
 * @brief Specify type of calculation
 *
 * @param handle
 * @param opt pointer to option instance
 * @param type See #PGM_CalculationType
 */
PGM_API void PGM_set_calculation_type(PGM_Handle* handle, PGM_Options* opt, PGM_Idx type);

/**
 * @brief Specify method of calculation
 *
 * @param handle
 * @param opt pointer to option instance
 * @param method See #PGM_CalculationMethod
 */
PGM_API void PGM_set_calculation_method(PGM_Handle* handle, PGM_Options* opt, PGM_Idx method);

/**
 * @brief Specify if we are calculating symmetrically or asymmetrically
 *
 * @param handle
 * @param opt pointer to option instance
 * @param sym One for symmetric calculation. Zero for asymmetric calculation
 */
PGM_API void PGM_set_symmetric(PGM_Handle* handle, PGM_Options* opt, PGM_Idx sym);

/**
 * @brief Specify the error tolerance to stop iterations. Only applicable if using iterative method.
 *
 * It is in terms of voltage deviation per iteration in p.u.
 *
 * @param handle
 * @param opt pointer to option instance
 * @param err_tol Relative votlage deviation tolerance
 */
PGM_API void PGM_set_err_tol(PGM_Handle* handle, PGM_Options* opt, double err_tol);

/**
 * @brief Specify maximum number of iterations. Only applicable if using iterative method.
 *
 * @param handle
 * @param opt pointer to option instance
 * @param max_iter Maximum number of iterations
 */
PGM_API void PGM_set_max_iter(PGM_Handle* handle, PGM_Options* opt, PGM_Idx max_iter);

/**
 * @brief Specify the multi-threading strategy. Only applicable for batch calculation.
 *
 * @param handle
 * @param opt pointer to option instance
 * @param threading Threading option. See below
 *   - -1: No multi-threading, calculate sequentially
 *   - 0: use number of machine available threads
 *   - >0: specify number of threads you want to calculate in parallel
 */
PGM_API void PGM_set_threading(PGM_Handle* handle, PGM_Options* opt, PGM_Idx threading);

#ifdef __cplusplus
}
#endif

#endif
