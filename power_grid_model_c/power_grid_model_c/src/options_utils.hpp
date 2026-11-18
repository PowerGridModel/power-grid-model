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
    using enum power_grid_model::CalculationType;
    switch (options.calculation_type) {
    case PGM_power_flow:
        return power_flow;
    case PGM_state_estimation:
        return state_estimation;
    case PGM_short_circuit:
        return short_circuit;
    default:
        throw power_grid_model::BadRequestEnumValueError{"get_calculation_type", "PGM_CalculationType",
                                                         options.calculation_type};
    }
}

constexpr auto get_calculation_method(PGM_Options const& options) {
    using enum power_grid_model::CalculationMethod;
    switch (options.calculation_method) {
    case PGM_default_method:
        return default_method;
    case PGM_linear:
        return linear;
    case PGM_newton_raphson:
        return newton_raphson;
    case PGM_iterative_linear:
        return iterative_linear;
    case PGM_iterative_current:
        return iterative_current;
    case PGM_linear_current:
        return linear_current;
    case PGM_iec60909:
        return iec60909;
    default:
        throw power_grid_model::BadRequestEnumValueError{"get_calculation_method", "PGM_CalculationMethod",
                                                         options.calculation_method};
    }
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
        throw power_grid_model::BadRequestEnumValueError{"get_optimizer_type", "PGM_TapChangingStrategy",
                                                         options.tap_changing_strategy};
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
        throw power_grid_model::BadRequestEnumValueError{"get_optimizer_strategy", "PGM_TapChangingStrategy",
                                                         options.tap_changing_strategy};
    }
}

constexpr auto get_short_circuit_voltage_scaling(PGM_Options const& options) {
    switch (options.short_circuit_voltage_scaling) {
        using enum power_grid_model::ShortCircuitVoltageScaling;
    case PGM_short_circuit_voltage_scaling_minimum:
        return minimum;
    case PGM_short_circuit_voltage_scaling_maximum:
        return maximum;
    default:
        throw power_grid_model::BadRequestEnumValueError{"get_short_circuit_voltage_caling",
                                                         "PGM_ShortCircuitVoltageScaling",
                                                         options.short_circuit_voltage_scaling};
    }
}

constexpr auto get_calculation_initialization(PGM_Options const& options) {
    switch (options.calculation_initialization) {
        using enum power_grid_model::CalculationInitialization;
    case PGM_calculation_initialization_default:
        return default_initialization;
    case PGM_calculation_initialization_flat:
        return flat;
    case PGM_calculation_initialization_linear:
        return linear;
    case PGM_calculation_initialization_average_source:
        return average_source;
    default:
        throw power_grid_model::BadRequestEnumValueError{
            "get_calculation_initialization", "PGM_CalculationInitialization", options.short_circuit_voltage_scaling};
    }
}

constexpr auto get_calculation_symmetry(PGM_Options const& options) {
    switch (options.symmetric) {
    case PGM_asymmetric:
        return power_grid_model::CalculationSymmetry::asymmetric;
    case PGM_symmetric:
        return power_grid_model::CalculationSymmetry::symmetric;
    default:
        throw power_grid_model::BadRequestEnumValueError{"get_calculation_symmetry", "PGM_SymmetryType",
                                                         options.symmetric};
    }
}
} // namespace power_grid_model_c
