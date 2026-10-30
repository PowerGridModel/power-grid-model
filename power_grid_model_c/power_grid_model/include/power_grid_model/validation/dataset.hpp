// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/exception.hpp>

#include <algorithm>
#include <string_view>

namespace power_grid_model::validation {

inline bool should_validate_component(ConstDataset const& dataset, std::string_view component) {
    if (!dataset.contains_component(component)) {
        return false;
    }

    auto const& buffer = dataset.get_buffer(component);
    return dataset.is_row_based(buffer) || std::ranges::any_of(buffer.attributes, [](auto const& attribute_buffer) {
               return std::string_view{attribute_buffer.meta_attribute->name} == "id";
           });
}

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
    if (update_dataset.get_next_cartesian_product_dimension() != nullptr) {
        throw InvalidArguments{"Batch validation does not support multi-dimensional batches.\n"};
    }
}

} // namespace power_grid_model::validation
