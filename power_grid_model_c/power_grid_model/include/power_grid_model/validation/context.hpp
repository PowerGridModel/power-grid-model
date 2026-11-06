// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "options.hpp"

#include "power_grid_model/auxiliary/dataset.hpp"
#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/exception.hpp"

#include <cstdint>
#include <functional>
#include <string_view>
#include <unordered_map>

namespace power_grid_model::validation {
namespace detail {
struct ComponentElement {
    std::string_view component_type;
    Idx element_idx;
};
} // namespace detail

enum class ValidationDatasetType : std::uint8_t { input, input_with_update };

using IDToComponentElementMap = std::unordered_map<ID, detail::ComponentElement>;

struct ValidationContext {
    std::reference_wrapper<ConstDataset const> input;
    DataValidatorOptions options{};

    // TODO(figueroa1395): Replace strings as keys in the map, here and elsewhere
    IDToComponentElementMap id_to_component_element_map{};

    ConstDataset const& dataset(ValidationDatasetType source) const {
        switch (source) {
        case ValidationDatasetType::input:
            return input.get();
        case ValidationDatasetType::input_with_update:
            throw NotImplementedError{};
        default:
            throw MissingCaseForEnumError{"ValidationContext::dataset", source};
        }
    }
};

} // namespace power_grid_model::validation
