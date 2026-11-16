// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "power_grid_model/auxiliary/input.hpp"

#include <tuple>

namespace power_grid_model::validation::components {

struct Node {
    static constexpr char const* name = "node";
    using InputType = NodeInput;
};
struct Line {
    static constexpr char const* name = "line";
    using InputType = LineInput;
};

using AllComponentsTuple = std::tuple<Node, Line>;

} // namespace power_grid_model::validation::components
