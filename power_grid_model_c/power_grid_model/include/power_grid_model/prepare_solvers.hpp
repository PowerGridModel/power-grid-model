// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common/common.hpp"

#include "math_solver/math_solver_dispatch.hpp"

#include "main_core/main_model_type.hpp"
#include "main_core/math_state.hpp"

namespace power_grid_model {
template <class ModelType>
    requires(main_core::is_main_model_type_v<ModelType>)
struct SolverPreparationContext {
    typename ModelType::MainModelState& state;
    main_core::MathState& math_state;
    Idx& n_math_solvers;
    bool& is_topology_up_to_date;
    bool& is_sym_parameter_up_to_date;
    bool& is_asym_parameter_up_to_date;
    bool& last_updated_calculation_symmetry_mode;
    typename ModelType::SequenceIdx& parameter_changed_components;
    MathSolverDispatcher const& math_solver_dispatcher;

    template <symmetry_tag sym> bool& is_parameter_up_to_date() {
        if constexpr (is_symmetric_v<sym>) {
            return is_sym_parameter_up_to_date;
        } else {
            return is_asym_parameter_up_to_date;
        }
    }
};

namespace detail {
template <class ModelType> void reset_solvers(SolverPreparationContext<ModelType>& context) {
    // assert(construction_complete_);
    context.is_topology_up_to_date = false;
    context.is_sym_parameter_up_to_date = false;
    context.is_asym_parameter_up_to_date = false;
    context.n_math_solvers = 0;
    main_core::clear(context.math_state);
    context.state.math_topology.clear();
    context.state.topo_comp_coup.reset();
    context.state.comp_coup = {};
}
} // namespace detail
} // namespace power_grid_model
