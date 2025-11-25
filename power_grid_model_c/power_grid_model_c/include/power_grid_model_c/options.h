// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
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
 *   - short_circuit_voltage_scaling: PGM_short_circuit_voltage_scaling_maximum
 *   - experimental_features: PGM_experimental_features_disabled
 *
 * @param handle
 * @return The pointer to the option instance. Should be freed by PGM_destroy_options().
 */
PGM_API PGM_Options* PGM_create_options(PGM_Handle* handle);

/**
 * @brief Free an option instance.
 *
 * @param opt The pointer to the option instance created by PGM_create_options().
 */
PGM_API void PGM_destroy_options(PGM_Options* opt);

/**
 * @brief Specify type of calculation.
 *
 * @param handle
 * @param opt The pointer to the option instance.
 * @param type See #PGM_CalculationType .
 */
PGM_API void PGM_set_calculation_type(PGM_Handle* handle, PGM_Options* opt, PGM_Idx type);

/**
 * @brief Specify method of calculation.
 *
 * @param handle
 * @param opt The pointer to the option instance.
 * @param method See #PGM_CalculationMethod .
 */
PGM_API void PGM_set_calculation_method(PGM_Handle* handle, PGM_Options* opt, PGM_Idx method);

/**
 * @brief Specify if we are calculating symmetrically or asymmetrically.
 *
 * @param handle
 * @param opt The pointer to the option instance.
 * @param sym See #PGM_CalculationSymmetry . 1 for symmetric calculation; 0 for asymmetric calculation.
 */
PGM_API void PGM_set_symmetric(PGM_Handle* handle, PGM_Options* opt, PGM_Idx sym);

/**
 * @brief Specify the error tolerance to stop iterations. Only applicable if using iterative method.
 *
 * It is in terms of voltage deviation per iteration in p.u.
 *
 * @param handle
 * @param opt The pointer to the option instance.
 * @param err_tol The relative votlage deviation tolerance.
 */
PGM_API void PGM_set_err_tol(PGM_Handle* handle, PGM_Options* opt, double err_tol);

/**
 * @brief Specify maximum number of iterations. Only applicable if using iterative method.
 *
 * @param handle
 * @param opt The pointer to the option instance.
 * @param max_iter The maximum number of iterations.
 */
PGM_API void PGM_set_max_iter(PGM_Handle* handle, PGM_Options* opt, PGM_Idx max_iter);

/**
 * @brief Specify the multi-threading strategy. Only applicable for batch calculation.
 *
 * @param handle
 * @param opt The pointer to the option instance.
 * @param threading The value of the threading setting. See below:
 *   - -1: No multi-threading, calculate sequentially.
 *   - 0: use number of machine available threads.
 *   - >0: specify number of threads you want to calculate in parallel.
 */
PGM_API void PGM_set_threading(PGM_Handle* handle, PGM_Options* opt, PGM_Idx threading);

/**
 * @brief Specify the voltage scaling min/max for short circuit calculations
 *
 * @param handle
 * @param opt pointer to option instance
 * @param short_circuit_voltage_scaling See #PGM_ShortCircuitVoltageScaling
 */
PGM_API void PGM_set_short_circuit_voltage_scaling(PGM_Handle* handle, PGM_Options* opt,
                                                   PGM_Idx short_circuit_voltage_scaling);

/**
 * @brief Specify the tap changing strategy for power flow calculations
 *
 * @param handle
 * @param opt pointer to option instance
 * @param tap_changing_strategy See #PGM_TapChangingStrategy
 */
PGM_API void PGM_set_tap_changing_strategy(PGM_Handle* handle, PGM_Options* opt, PGM_Idx tap_changing_strategy);

/**
 * @brief Enable/disable experimental features.
 *
 * [Danger mode]
 *
 * The behavior of experimental features may not be final and no stability guarantees are made to the users.
 * Features marked as 'experimental' as well as the behavior of experimental functionality itself may change over time.
 *
 * @param handle
 * @param opt pointer to option instance
 * @param experimental_features See #PGM_ExperimentalFeatures
 */
PGM_API void PGM_set_experimental_features(PGM_Handle* handle, PGM_Options* opt, PGM_Idx experimental_features);

#ifdef __cplusplus
}
#endif

#endif
