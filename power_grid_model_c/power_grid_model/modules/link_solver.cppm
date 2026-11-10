// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Dense>
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

import power_grid_model.calculation_parameters;
import power_grid_model.common.common;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.typing;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.three_phase_tensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/link_solver.hpp>

export module power_grid_model.link_solver;

export namespace power_grid_model::link_solver {
using power_grid_model::link_solver::compute_loads_link_elements;

namespace detail {
using power_grid_model::link_solver::detail::AdjacencyMap;
using power_grid_model::link_solver::detail::backward_substitution;
using power_grid_model::link_solver::detail::backward_substitution_free_right_cols;
using power_grid_model::link_solver::detail::backward_substitution_pivots;
using power_grid_model::link_solver::detail::backward_substitution_rows;
using power_grid_model::link_solver::detail::build_adjacency_map;
using power_grid_model::link_solver::detail::compute_internal_loads;
using power_grid_model::link_solver::detail::CooSparseMatrix;
using power_grid_model::link_solver::detail::EdgeEvent;
using power_grid_model::link_solver::detail::forward_elimination;
using power_grid_model::link_solver::detail::naive_gauss_elimination;
using power_grid_model::link_solver::detail::ReducedEchelonForm;
using power_grid_model::link_solver::detail::set_projection_system;
using power_grid_model::link_solver::detail::set_solution_system;
using power_grid_model::link_solver::detail::SolutionSet;
} // namespace detail
} // namespace power_grid_model::link_solver
