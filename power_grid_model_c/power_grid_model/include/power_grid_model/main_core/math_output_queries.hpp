// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cassert>
#include <concepts>
#include <vector>

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../component/branch.hpp"
#include "../component/branch3.hpp"
#include "../component/fault.hpp"
#include "../component/load_gen.hpp"
#include "../component/shunt.hpp"
#include "../component/source.hpp"

namespace power_grid_model::main_core {

template <typename Component, solver_output_type SolverOutputType>
constexpr auto const& get_component_output(MathOutput<std::vector<SolverOutputType>> const& math_output,
                                           Idx2D const& math_id) {
    auto const& solver_output = math_output.solver_output[math_id.group];

    // TODO(mgovers): cleanup v2: change back to std::derived_from<Component, Branch>
    if constexpr (std::derived_from<Component, Edge> || std::derived_from<Component, Branch3>) {
        return solver_output.branch[math_id.pos];
    } else if constexpr (std::same_as<Component, Source> && requires { solver_output.source; }) {
        return solver_output.source[math_id.pos];
    } else if constexpr (std::same_as<Component, Shunt> && requires { solver_output.shunt; }) {
        return solver_output.shunt[math_id.pos];
    } else if constexpr (std::derived_from<Component, GenericLoadGen> && requires { solver_output.load_gen; }) {
        return solver_output.load_gen[math_id.pos];
    } else if constexpr (std::same_as<Component, Fault> && requires { solver_output.fault; }) {
        return solver_output.fault[math_id.pos];
    } else {
        static_assert(false, "Unsupported component type for output retrieval");
    }
}

} // namespace power_grid_model::main_core
