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

template <solver_output_type SolverOutputType>
constexpr auto get_voltage_output(MathOutput<std::vector<SolverOutputType>> const& math_output, Idx2D const& math_id) {
    if constexpr (steady_state_solver_output_type<SolverOutputType>) {
        return math_output.solver_output[math_id.group].u[math_id.pos];
    } else if constexpr (short_circuit_solver_output_type<SolverOutputType>) {
        return math_output.solver_output[math_id.group].u_bus[math_id.pos];
    } else {
        static_assert(false, "Unsupported solver output type for voltage output retrieval");
    }
}

template <steady_state_solver_output_type SolverOutputType>
constexpr auto get_bus_injection_output_from_topo_id(MathOutput<std::vector<SolverOutputType>> const& math_output,
                                                     Idx2D const& topo_id) {
    return math_output.supernode_output[topo_id.group].bus_injection[topo_id.pos];
}

template <steady_state_solver_output_type SolverOutputType>
constexpr auto get_bus_injection_output_from_math_id(MathOutput<std::vector<SolverOutputType>> const& math_output,
                                                     Idx2D const& math_id) {
    return math_output.solver_output[math_id.group].bus_injection[math_id.pos];
}

template <typename Component, solver_output_type SolverOutputType>
constexpr auto const& get_component_output(MathOutput<std::vector<SolverOutputType>> const& math_output,
                                           Idx2D const& math_id) {
    auto const& solver_output = math_output.solver_output[math_id.group];

    auto const& component_type_output = [&solver_output]() -> auto const& {
        // TODO(mgovers): cleanup v2: change back to std::derived_from<Component, Branch>
        if constexpr (std::derived_from<Component, Edge> || std::derived_from<Component, Branch3>) {
            return solver_output.branch;
        } else if constexpr (std::same_as<Component, Source> && requires { solver_output.source; }) {
            return solver_output.source;
        } else if constexpr (std::same_as<Component, Shunt> && requires { solver_output.shunt; }) {
            return solver_output.shunt;
        } else if constexpr (std::derived_from<Component, GenericLoadGen> && requires { solver_output.load_gen; }) {
            return solver_output.load_gen;
        } else if constexpr (std::same_as<Component, Fault> && requires { solver_output.fault; }) {
            return solver_output.fault;
        } else {
            static_assert(false, "Unsupported component type for output retrieval");
        }
    }();

    return component_type_output[math_id.pos];
}

} // namespace power_grid_model::main_core
