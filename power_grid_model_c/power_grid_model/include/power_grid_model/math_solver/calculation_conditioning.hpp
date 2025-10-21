// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"
#include "y_bus.hpp"

#include "../common/exception.hpp"

namespace power_grid_model::math_solver {
enum class CalculationConditioning : int8_t {
    well_conditioned = 0,
    possibly_ill_conditioned = 1,
};

} // namespace power_grid_model::math_solver
