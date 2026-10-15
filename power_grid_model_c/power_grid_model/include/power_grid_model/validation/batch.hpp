// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "input.hpp"

#include <power_grid_model/common/exception.hpp>

namespace power_grid_model::validation {

inline ValidationReport validate_batch_report(ConstDataset const& /*input_dataset*/,
                                              ConstDataset const& /*update_dataset*/, bool /*symmetric*/,
                                              CalculationType /*calculation_type*/) {
    throw NotImplementedError{};
}

} // namespace power_grid_model::validation
