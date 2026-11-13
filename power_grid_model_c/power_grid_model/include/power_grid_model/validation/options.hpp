// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/common/enum.hpp>

namespace power_grid_model::validation {

struct DataValidatorOptions {
    CalculationType calculation_type{CalculationType::power_flow};
    CalculationSymmetry calculation_symmetry{CalculationSymmetry::symmetric};
};

} // namespace power_grid_model::validation
