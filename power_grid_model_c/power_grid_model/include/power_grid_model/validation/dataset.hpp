// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/exception.hpp>

#include <string_view>

namespace power_grid_model::validation {

inline void validate_dataset_type(ConstDataset const& dataset) {
    if (std::string_view{dataset.dataset().name} != "input") {
        throw InvalidArguments{"Input validation requires dataset type 'input'.\n"};
    }
    if (dataset.is_batch()) {
        throw InvalidArguments{"Input validation does not support batch datasets.\n"};
    }
}

inline void validate_batch_dataset_type(ConstDataset const& input_dataset, ConstDataset const& update_dataset) {
    validate_dataset_type(input_dataset);
    if (std::string_view{update_dataset.dataset().name} != "update") {
        throw InvalidArguments{"Batch validation requires dataset type 'update'.\n"};
    }
    if (!update_dataset.is_batch()) {
        throw InvalidArguments{"Batch validation requires a batch update dataset.\n"};
    }
}

} // namespace power_grid_model::validation::detail
