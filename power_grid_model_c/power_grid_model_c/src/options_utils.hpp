// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "input_sanitization.hpp"
#include "options.hpp"

#include "power_grid_model_c/basics.h"

#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>

namespace power_grid_model_c {
constexpr auto get_calculation_type(PGM_Options const& options) {
    return safe_enum<power_grid_model::CalculationType>(options.calculation_type);
}

constexpr auto get_calculation_method(PGM_Options const& options) {
    return safe_enum<power_grid_model::CalculationMethod>(options.calculation_method);
}

constexpr auto get_optimizer_type(PGM_Options const& options) {
    using enum power_grid_model::OptimizerType;

    switch (options.tap_changing_strategy) {
    case PGM_tap_changing_strategy_disabled:
        return no_optimization;
    case PGM_tap_changing_strategy_any_valid_tap:
    case PGM_tap_changing_strategy_max_voltage_tap:
    case PGM_tap_changing_strategy_min_voltage_tap:
    case PGM_tap_changing_strategy_fast_any_tap:
        return automatic_tap_adjustment;
    default:
        throw power_grid_model::MissingCaseForEnumError{"get_optimizer_type", options.tap_changing_strategy};
    }
}

constexpr auto get_optimizer_strategy(PGM_Options const& options) {
    using enum power_grid_model::OptimizerStrategy;

    switch (options.tap_changing_strategy) {
    case PGM_tap_changing_strategy_disabled:
    case PGM_tap_changing_strategy_any_valid_tap:
        return any;
    case PGM_tap_changing_strategy_max_voltage_tap:
        return global_maximum;
    case PGM_tap_changing_strategy_min_voltage_tap:
        return global_minimum;
    case PGM_tap_changing_strategy_fast_any_tap:
        return fast_any;
    default:
        throw power_grid_model::MissingCaseForEnumError{"get_optimizer_strategy", options.tap_changing_strategy};
    }
}

constexpr auto get_short_circuit_voltage_scaling(PGM_Options const& options) {
    return safe_enum<power_grid_model::ShortCircuitVoltageScaling>(options.short_circuit_voltage_scaling);
}

constexpr auto get_calculation_initialization(PGM_Options const& options) {
    return safe_enum<power_grid_model::CalculationInitialization>(options.calculation_initialization);
}

constexpr auto get_calculation_symmetry(PGM_Options const& options) {
    switch (options.symmetric) {
    case PGM_asymmetric:
        return power_grid_model::CalculationSymmetry::asymmetric;
    case PGM_symmetric:
        return power_grid_model::CalculationSymmetry::symmetric;
    default:
        throw power_grid_model::MissingCaseForEnumError{"get_calculation_symmetry", options.symmetric};
    }
}
} // namespace power_grid_model_c
