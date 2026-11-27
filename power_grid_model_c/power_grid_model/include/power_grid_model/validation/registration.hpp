// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "components/line.hpp"
#include "components/node.hpp"
#include "components/sym_power_sensor.hpp"
#include "registry.hpp"

namespace power_grid_model::validation {

inline ValidationRegistry const& default_registry() {
    using namespace components;
    static ValidationRegistry const registry{node_rule_table(), line_rule_table(), sym_power_sensor_rule_table()};
    return registry;
}

} // namespace power_grid_model::validation
