// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/validation/components/all_components.hpp>

#include <power_grid_model/component/line.hpp>
#include <power_grid_model/component/node.hpp>

#include <string_view>

namespace {
template <class Descriptor, class Component>
constexpr bool is_same_name_v = std::string_view{Descriptor::name} == std::string_view{Component::name};

template <class Descriptor, class Component> consteval bool types_match() {
    return std::is_same_v<typename Descriptor::InputType, typename Component::InputType>;
}

// line
static_assert(is_same_name_v<power_grid_model::validation::components::Line, power_grid_model::Line>);
static_assert(types_match<power_grid_model::validation::components::Line, power_grid_model::Line>());

// node
static_assert(is_same_name_v<power_grid_model::validation::components::Node, power_grid_model::Node>);
static_assert(types_match<power_grid_model::validation::components::Node, power_grid_model::Node>());
} // namespace
