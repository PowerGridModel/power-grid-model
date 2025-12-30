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
        return UpdateChange{.topo = x.topo || y.topo, .param = x.param || y.param};
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

struct load_appliance_t {};
struct gen_appliance_t {};

template <typename T>
concept appliance_type_tag = std::same_as<T, load_appliance_t> || std::same_as<T, gen_appliance_t>;
template <appliance_type_tag T> constexpr bool is_generator_v = std::same_as<T, gen_appliance_t>;

static_assert(appliance_type_tag<load_appliance_t>);
static_assert(appliance_type_tag<gen_appliance_t>);
static_assert(!is_generator_v<load_appliance_t>);
static_assert(is_generator_v<gen_appliance_t>);

} // namespace power_grid_model
