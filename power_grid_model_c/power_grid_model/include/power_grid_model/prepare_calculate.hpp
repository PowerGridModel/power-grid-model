// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "topology.hpp"

#include "common/common.hpp"

#include "math_solver/math_solver_dispatch.hpp"

#include "main_core/main_model_type.hpp"
#include "main_core/math_state.hpp"
#include "main_core/topology.hpp"
#include "main_core/y_bus.hpp"

namespace power_grid_model {
struct SolverPreparationContext {
    main_core::MathState math_state;
    MathSolverDispatcher const* math_solver_dispatcher;
};

template <class ModelType>
    requires(main_core::is_main_model_type_v<ModelType>)
struct StatusCheckingContext {
    bool is_topology_up_to_date{false};
    bool last_updated_calculation_symmetry_mode{false};
    typename ModelType::SequenceIdx parameter_changed_components{};
    struct IsParameterUpToDateHelper {
        bool sym{false};
        bool asym{false};
    };
    IsParameterUpToDateHelper is_parameter_up_to_date{};
};

namespace detail {
template <class ModelType>
void reset_solvers(typename ModelType::MainModelState& state, SolverPreparationContext& solver_context,
                   StatusCheckingContext<ModelType>& status_context) {
    status_context.is_topology_up_to_date = false;
    status_context.is_parameter_up_to_date.sym = false;
    status_context.is_parameter_up_to_date.asym = false;
    main_core::clear(solver_context.math_state);
    state.math_topology.clear();
    state.topo_comp_coup.reset();
    state.comp_coup = {};
}

template <class ModelType>
void rebuild_topology(typename ModelType::MainModelState& state, SolverPreparationContext& solver_context,
                      StatusCheckingContext<ModelType>& status_context) {
    // clear old solvers
    reset_solvers(state, solver_context, status_context);
    ComponentConnections const comp_conn = main_core::construct_components_connections<ModelType>(state.components);
    // re build
    Topology topology{*state.comp_topo, comp_conn};
    std::tie(state.math_topology, state.topo_comp_coup) = topology.build_topology();
    status_context.is_topology_up_to_date = true;
    status_context.is_parameter_up_to_date.sym = false;
    status_context.is_parameter_up_to_date.asym = false;
}
} // namespace detail

template <symmetry_tag sym, class ModelType>
bool& is_parameter_up_to_date(
    typename StatusCheckingContext<ModelType>::IsParameterUpToDateHelper& is_parameter_up_to_date) {
    if constexpr (is_symmetric_v<sym>) {
        return is_parameter_up_to_date.sym;
    } else {
        return is_parameter_up_to_date.asym;
    }
}

template <class ModelType> Idx get_n_math_solvers(typename ModelType::MainModelState const& state) {
    return static_cast<Idx>(state.math_topology.size());
}

template <symmetry_tag sym, class ModelType>
void prepare_solvers(typename ModelType::MainModelState& state, SolverPreparationContext& solver_context,
                     StatusCheckingContext<ModelType>& status_context) {
    std::vector<MathSolverProxy<sym>>& solvers = main_core::get_solvers<sym>(solver_context.math_state);
    // rebuild topology if needed
    if (!status_context.is_topology_up_to_date) {
        detail::rebuild_topology(state, solver_context, status_context);
    }
    Idx n_math_solvers = get_n_math_solvers<ModelType>(state);
    main_core::prepare_y_bus<sym, ModelType>(state, n_math_solvers, solver_context.math_state);
    if (n_math_solvers != static_cast<Idx>(solvers.size())) {
        assert(solvers.empty());
        assert(n_math_solvers == static_cast<Idx>(main_core::get_y_bus<sym>(solver_context.math_state).size()));

        solvers.clear();
        solvers.reserve(n_math_solvers);
        std::ranges::transform(state.math_topology, std::back_inserter(solvers),
                               [&solver_context](auto const& math_topo) {
                                   return MathSolverProxy<sym>{solver_context.math_solver_dispatcher, math_topo};
                               });
        for (Idx idx = 0; idx < n_math_solvers; ++idx) {
            main_core::get_y_bus<sym>(solver_context.math_state)[idx].register_parameters_changed_callback(
                [solver = std::ref(solvers[idx])](bool changed) { solver.get().get().parameters_changed(changed); });
        }
    } else if (!is_parameter_up_to_date<sym, ModelType>(status_context.is_parameter_up_to_date)) {
        std::vector<MathModelParam<sym>> const math_params = main_core::get_math_param<sym>(state, n_math_solvers);
        std::vector<MathModelParamIncrement> const math_param_increments =
            main_core::get_math_param_increment<ModelType>(state, n_math_solvers,
                                                           status_context.parameter_changed_components);
        if (status_context.last_updated_calculation_symmetry_mode == is_symmetric_v<sym>) {
            main_core::update_y_bus(solver_context.math_state, math_params, math_param_increments);
        } else {
            main_core::update_y_bus(solver_context.math_state, math_params);
        }
    }
    // else do nothing, set everything up to date
    is_parameter_up_to_date<sym, ModelType>(status_context.is_parameter_up_to_date) = true;
    std::ranges::for_each(status_context.parameter_changed_components, [](auto& comps) { comps.clear(); });
    status_context.last_updated_calculation_symmetry_mode = is_symmetric_v<sym>;
}
} // namespace power_grid_model
