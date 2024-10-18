// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// define all components
#include "common/common.hpp"
#include "common/component_list.hpp"
// component include
#include "component/line.hpp"
#include "component/load_gen.hpp"
#include "component/node.hpp"
#include "component/source.hpp"

namespace power_grid_model {

using AllComponents =
    ComponentList<Node, Line, Source, SymLoad>;

} // namespace power_grid_model
