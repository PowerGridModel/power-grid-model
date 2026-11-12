// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "options.hpp"

#include "power_grid_model/auxiliary/dataset.hpp"
#include "power_grid_model/common/common.hpp"

#include <cassert>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace power_grid_model::validation {
namespace detail {
struct ComponentElement {
    // TODO(figueroa1395): Replace strings as keys in the map, here and elsewhere
    std::string_view component_type;
    Idx element_idx;
};
} // namespace detail

// TODO(figueroa1395): This doens't belong here anymore. Revisit names too
enum class ValidationDatasetType : std::uint8_t { input, input_with_update };

using IDToComponentElementMap = std::unordered_map<ID, detail::ComponentElement>;
using ComponentToAttributeToIdxMap = std::unordered_map<std::string_view, std::unordered_map<std::string_view, Idx>>;

struct ValidationContext {
    std::reference_wrapper<ConstDataset const> input;
    std::optional<std::reference_wrapper<ConstDataset const>> update;
    DataValidatorOptions options{};

    IDToComponentElementMap id_to_component_element_map{};
    ComponentToAttributeToIdxMap component_to_attribute_to_idx_map{};

    ConstDataset const& get_input() const { return input.get(); }
    ConstDataset const& get_update() const {
        assert(update.has_value());
        return update->get();
    }
};

} // namespace power_grid_model::validation
