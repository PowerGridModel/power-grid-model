// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "power_grid_model/auxiliary/dataset.hpp"
#include "power_grid_model/common/common.hpp"

#include <algorithm>
#include <functional>
#include <ranges>

namespace power_grid_model::validation::detail {

inline Idx get_total_input_elements(ConstDataset const& input_dataset) {
    auto const components_sizes_view = input_dataset.get_description().component_info |
                                       std::views::transform(&meta_data::ComponentInfo::total_elements);
    return std::ranges::fold_left(components_sizes_view, Idx{0}, std::plus<>{});
}
} // namespace power_grid_model::validation::detail
