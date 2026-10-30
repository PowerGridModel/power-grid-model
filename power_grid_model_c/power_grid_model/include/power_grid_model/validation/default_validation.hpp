// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "batch.hpp"
#include "default_registry.hpp"
#include "input.hpp"
#include "report.hpp"

namespace power_grid_model::validation {

inline ValidationReport validate_input_dataset(ConstDataset const& dataset, bool symmetric,
                                               CalculationType calculation_type) {
    return detail::validate_input_with_registry(dataset, symmetric, calculation_type, default_registry());
}

inline ValidationReport validate_batch_data(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                                            bool symmetric, CalculationType calculation_type) {
    return detail::validate_batch_with_registry(input_dataset, update_dataset, symmetric, calculation_type,
                                                default_registry());
}

} // namespace power_grid_model::validation
