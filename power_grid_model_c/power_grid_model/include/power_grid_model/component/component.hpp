// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/common.hpp"
#include "../common/enum.hpp"

#include <concepts>
#include <string_view>

namespace power_grid_model {

// change of update cause topology and param change, or just param change
struct UpdateChange {
    bool topo{};
    bool param{};

    friend constexpr UpdateChange operator||(UpdateChange const& x, UpdateChange const& y) {
        return UpdateChange{x.topo || y.topo, x.param || y.param};
    }
};

template <typename T>
concept component_c = requires(T t, T const& ct, typename T::UpdateType u, typename T::UpdateType const& cu) {
                          typename T::InputType;
                          typename T::UpdateType;

                          { T::name } -> std::convertible_to<std::string_view>;
                          { ct.math_model_type() } -> std::convertible_to<ComponentType>;

                          { ct.id() } -> std::same_as<ID>;

                          { t.update(cu) } -> std::same_as<UpdateChange>;
                          { ct.inverse(u) } -> std::same_as<typename T::UpdateType>;
                      };

} // namespace power_grid_model
