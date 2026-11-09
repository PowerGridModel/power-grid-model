// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <boost/container/small_vector.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/pending/property.hpp>
#include <cassert>
#include <format>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <vector>

import power_grid_model.calculation_parameters;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.typing;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.common.exception;
import power_grid_model.component.component;
import power_grid_model.all_components;
import power_grid_model.common.component_list;
import power_grid_model.component.appliance;
import power_grid_model.component.base;
import power_grid_model.auxiliary.input;
import power_grid_model.auxiliary.output;
import power_grid_model.auxiliary.update;
import power_grid_model.component.asym_line;
import power_grid_model.component.branch;
import power_grid_model.component.edge;
import power_grid_model.component.line_utils;
import power_grid_model.common.matrix_utils;
import power_grid_model.component.branch3;
import power_grid_model.component.current_sensor;
import power_grid_model.component.sensor;
import power_grid_model.component.fault;
import power_grid_model.component.generic_branch;
import power_grid_model.component.line;
import power_grid_model.component.link;
import power_grid_model.component.load_gen;
import power_grid_model.component.node;
import power_grid_model.component.power_sensor;
import power_grid_model.component.regulator;
import power_grid_model.component.shunt;
import power_grid_model.component.source;
import power_grid_model.component.three_winding_transformer;
import power_grid_model.component.transformer;
import power_grid_model.component.transformer_utils;
import power_grid_model.component.transformer_tap_regulator;
import power_grid_model.component.voltage_regulator;
import power_grid_model.component.voltage_sensor;
import power_grid_model.main_core.main_model_type;
import power_grid_model.container;
import power_grid_model.main_core.state;
import power_grid_model.container_fwd;
import power_grid_model.main_core.update;
import power_grid_model.main_core.container_queries;
import power_grid_model.main_core.core_utils;
import power_grid_model.auxiliary.dataset;
import power_grid_model.auxiliary.dataset_fwd;
import power_grid_model.auxiliary.meta_data;
import power_grid_model.main_core.math_state;
import power_grid_model.math_solver.math_solver_dispatch;
import power_grid_model.common.logging;
import power_grid_model.math_solver.y_bus;
import power_grid_model.main_core.topology;
import power_grid_model.main_core.y_bus;
import power_grid_model.supernodes;
import power_grid_model.topology;
import power_grid_model.sparse_ordering;
import power_grid_model.index_mapping;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/calculation_preparation.hpp>

export module power_grid_model.calculation_preparation;

export namespace power_grid_model {
using power_grid_model::get_n_math_solvers;
using power_grid_model::prepare_solvers;
using power_grid_model::SolverPreparationContext;
using power_grid_model::SolversCacheState;
} // namespace power_grid_model
