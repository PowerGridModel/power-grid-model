// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/calculation_preparation.hpp>
#include <power_grid_model/main_core/main_model_type.hpp>
#include <power_grid_model/main_core/update.hpp>
#include <power_grid_model/math_solver/math_solver.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
using MainModelType = main_core::MainModelType<AllExtraRetrievableTypes, AllComponents>;

TEST_CASE("Test SolversCacheStatus") {
    SUBCASE("Default construction") {
        SolversCacheStatus<MainModelType> const cache_status{};

        CHECK_FALSE(cache_status.is_topology_valid());
        CHECK_FALSE(cache_status.is_parameter_valid<symmetric_t>());
        CHECK_FALSE(cache_status.is_parameter_valid<asymmetric_t>());
        CHECK_FALSE(cache_status.is_symmetry_mode_conserved<symmetric_t>());
        CHECK_FALSE(cache_status.is_symmetry_mode_conserved<asymmetric_t>());
        CHECK(cache_status.changed_components_indices() == MainModelType::SequenceIdx{});
    }

    SUBCASE("Setters and getters") {
        SolversCacheStatus<MainModelType> cache_status{};

        // Topology status
        cache_status.set_topology_status(true);
        CHECK(cache_status.is_topology_valid());
        cache_status.set_topology_status(false);
        CHECK_FALSE(cache_status.is_topology_valid());

        // Parameter status
        cache_status.set_parameter_status<symmetric_t>(true);
        CHECK(cache_status.is_parameter_valid<symmetric_t>());
        CHECK_FALSE(cache_status.is_parameter_valid<asymmetric_t>());

        cache_status.set_parameter_status<asymmetric_t>(true);
        CHECK(cache_status.is_parameter_valid<symmetric_t>());
        CHECK(cache_status.is_parameter_valid<asymmetric_t>());

        cache_status.set_parameter_status<symmetric_t>(false);
        CHECK_FALSE(cache_status.is_parameter_valid<symmetric_t>());
        CHECK(cache_status.is_parameter_valid<asymmetric_t>());

        // Symmetry mode
        cache_status.set_previous_symmetry_mode<symmetric_t>();
        CHECK(cache_status.is_symmetry_mode_conserved<symmetric_t>());
        CHECK_FALSE(cache_status.is_symmetry_mode_conserved<asymmetric_t>());

        cache_status.set_previous_symmetry_mode<asymmetric_t>();
        CHECK_FALSE(cache_status.is_symmetry_mode_conserved<symmetric_t>());
        CHECK(cache_status.is_symmetry_mode_conserved<asymmetric_t>());

        // Changed components indices
        auto& indices = cache_status.changed_components_indices();
        std::get<0>(indices).push_back(Idx2D{.group = 0, .pos = 1});
        std::get<1>(indices).push_back(Idx2D{.group = 1, .pos = 2});
        CHECK(std::get<0>(cache_status.changed_components_indices())[0] == Idx2D{.group = 0, .pos = 1});
        CHECK(std::get<1>(cache_status.changed_components_indices())[0] == Idx2D{.group = 1, .pos = 2});

        cache_status.clear_changed_components_indices();
        CHECK(std::ranges::all_of(cache_status.changed_components_indices(),
                                  [](auto const& vec) { return vec.empty(); }));
    }
}

TEST_CASE("Test SolverPreparationContext") {
    SUBCASE("Default construction") {
        SolverPreparationContext const context{};

        CHECK(context.math_solver_dispatcher == nullptr);
        CHECK(context.math_state.y_bus_vec_sym.empty());
        CHECK(context.math_state.y_bus_vec_asym.empty());
        CHECK(context.math_state.math_solvers_sym.empty());
        CHECK(context.math_state.math_solvers_asym.empty());
    }

    SUBCASE("Dummy construction") {
        MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<MathSolver>{}};
        SolverPreparationContext const context{.math_state = {}, .math_solver_dispatcher = &dispatcher};

        CHECK(context.math_solver_dispatcher == &dispatcher);
        CHECK(context.math_state.y_bus_vec_sym.empty());
        CHECK(context.math_state.y_bus_vec_asym.empty());
        CHECK(context.math_state.math_solvers_sym.empty());
        CHECK(context.math_state.math_solvers_asym.empty());
    }
}

} // namespace
} // namespace power_grid_model
