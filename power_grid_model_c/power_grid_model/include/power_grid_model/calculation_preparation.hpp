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
class SolversCacheStatus {
  public:
    using SequenceIdx = typename ModelType::SequenceIdx;

  private:
    struct YBusParameterCacheValidity {
        bool sym{false};
        bool asym{false};
    };

    enum class SymmetryMode : IntS { symmetric = 0, asymmetric = 1, not_set = na_IntS };

    bool topology_cache_validity{false};
    YBusParameterCacheValidity parameter_cache_validity{};
    SymmetryMode previous_symmetry_mode{SymmetryMode::not_set};
    SequenceIdx changed_components_indices_{};

  public:
    SequenceIdx const& changed_components_indices() const { return changed_components_indices_; }
    SequenceIdx& changed_components_indices() { return changed_components_indices_; }
    void clear_changed_components_indices() {
        std::ranges::for_each(changed_components_indices(), [](auto& comps) { comps.clear(); });
    }

    bool is_topology_valid() const { return topology_cache_validity; }
    void set_topology_status(bool topology) { topology_cache_validity = topology; }

    template <symmetry_tag sym> bool is_parameter_valid() const {
        if constexpr (is_symmetric_v<sym>) {
            return parameter_cache_validity.sym;
        } else {
            return parameter_cache_validity.asym;
        }
    }
    template <symmetry_tag sym> void set_parameter_status(bool parameter) {
        if constexpr (is_symmetric_v<sym>) {
            parameter_cache_validity.sym = parameter;
        } else {
            parameter_cache_validity.asym = parameter;
        }
    }

    template <symmetry_tag sym> bool is_symmetry_mode_conserved() const {
        if (previous_symmetry_mode == SymmetryMode::not_set) {
            return false; // No previous calculation
        }

        if constexpr (is_symmetric_v<sym>) {
            return previous_symmetry_mode == SymmetryMode::symmetric;
        } else {
            return previous_symmetry_mode == SymmetryMode::asymmetric;
        }
    }
    template <symmetry_tag sym> void set_previous_symmetry_mode() {
        if constexpr (is_symmetric_v<sym>) {
            previous_symmetry_mode = SymmetryMode::symmetric;
        } else {
            previous_symmetry_mode = SymmetryMode::asymmetric;
        }
    }

    void update(UpdateChange const& changes) {
        // if topology changed, everything is not up to date
        // if only param changed, set param to not up to date
        if (changes.topo || !is_topology_valid()) {
            set_topology_status(false);
            set_parameter_status<symmetric_t>(false);
            set_parameter_status<asymmetric_t>(false);
            return;
        }

        set_topology_status(true);
        if (changes.param) {
            set_parameter_status<symmetric_t>(false);
            set_parameter_status<asymmetric_t>(false);
            return;
        }
        set_parameter_status<symmetric_t>(is_parameter_valid<symmetric_t>());
        set_parameter_status<asymmetric_t>(is_parameter_valid<asymmetric_t>());
    }
};

namespace detail {
template <class ModelType>
void reset_solvers(typename ModelType::MainModelState& state, SolverPreparationContext& solver_context,
                   SolversCacheStatus<ModelType>& solvers_cache_status) {
    solvers_cache_status.set_topology_status(false);
    solvers_cache_status.template set_parameter_status<symmetric_t>(false);
    solvers_cache_status.template set_parameter_status<asymmetric_t>(false);
    main_core::clear(solver_context.math_state);
    state.math_topology.clear();
    state.topo_comp_coup.reset();
    state.comp_coup = {};
}

template <class ModelType>
void rebuild_topology(typename ModelType::MainModelState& state, SolverPreparationContext& solver_context,
                      SolversCacheStatus<ModelType>& solvers_cache_status) {
    // clear old solvers
    reset_solvers(state, solver_context, solvers_cache_status);
    ComponentConnections const comp_conn = main_core::construct_components_connections<ModelType>(state.components);
    // re build
    Topology topology{*state.comp_topo, comp_conn};
    std::tie(state.math_topology, state.topo_comp_coup) = topology.build_topology();
    solvers_cache_status.set_topology_status(true);
    solvers_cache_status.template set_parameter_status<symmetric_t>(false);
    solvers_cache_status.template set_parameter_status<asymmetric_t>(false);
}
} // namespace detail

template <class ModelType> Idx get_n_math_solvers(typename ModelType::MainModelState const& state) {
    return static_cast<Idx>(state.math_topology.size());
}

template <symmetry_tag sym, class ModelType>
void prepare_solvers(typename ModelType::MainModelState& state, SolverPreparationContext& solver_context,
                     SolversCacheStatus<ModelType>& solvers_cache_status) {
    std::vector<MathSolverProxy<sym>>& solvers = main_core::get_solvers<sym>(solver_context.math_state);
    // rebuild topology if needed
    if (!solvers_cache_status.is_topology_valid()) {
        detail::rebuild_topology(state, solver_context, solvers_cache_status);
    }
    Idx const n_math_solvers = get_n_math_solvers<ModelType>(state);
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
    } else if (!solvers_cache_status.template is_parameter_valid<sym>()) {
        std::vector<MathModelParam<sym>> const math_params = main_core::get_math_param<sym>(state, n_math_solvers);
        if (solvers_cache_status.template is_symmetry_mode_conserved<sym>()) {
            std::vector<MathModelParamIncrement> const math_param_increments =
                main_core::get_math_param_increment<ModelType>(state, n_math_solvers,
                                                               solvers_cache_status.changed_components_indices());
            main_core::update_y_bus(solver_context.math_state, math_params, math_param_increments);
        } else {
            main_core::update_y_bus(solver_context.math_state, math_params);
        }
    }
    // else do nothing, set everything up to date
    solvers_cache_status.template set_parameter_status<sym>(true);
    solvers_cache_status.clear_changed_components_indices();
    solvers_cache_status.template set_previous_symmetry_mode<sym>();
}
} // namespace power_grid_model
