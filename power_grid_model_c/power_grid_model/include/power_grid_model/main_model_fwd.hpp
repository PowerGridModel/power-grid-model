// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common/common.hpp"

namespace power_grid_model {

struct cached_update_t : std::true_type {};
struct permanent_update_t : std::false_type {};

template <typename T>
concept cache_type_c = std::same_as<T, cached_update_t> || std::same_as<T, permanent_update_t>;

struct MainModelOptions {
    static constexpr Idx sequential = -1;

    CalculationType calculation_type{CalculationType::power_flow};
    CalculationSymmetry calculation_symmetry{CalculationSymmetry::symmetric};
    CalculationMethod calculation_method{CalculationMethod::default_method};
    OptimizerType optimizer_type{OptimizerType::no_optimization};
    OptimizerStrategy optimizer_strategy{OptimizerStrategy::fast_any};

    double err_tol{1e-8};
    Idx max_iter{20};
    Idx threading{sequential};

    ShortCircuitVoltageScaling short_circuit_voltage_scaling{ShortCircuitVoltageScaling::maximum};
};

} // namespace power_grid_model
