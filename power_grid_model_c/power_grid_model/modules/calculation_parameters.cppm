// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <array>
#include <boost/container/small_vector.hpp>
#include <concepts>
#include <span>
#include <vector>

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

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/calculation_parameters.hpp>

export module power_grid_model.calculation_parameters;

export namespace power_grid_model {
using power_grid_model::disconnected;
using power_grid_model::status_off;
using power_grid_model::status_on;

using power_grid_model::YBusElement;

using power_grid_model::ApplianceShortCircuitSolverOutput;
using power_grid_model::ApplianceSolverOutput;
using power_grid_model::Branch3Idx;
using power_grid_model::BranchCalcParam;
using power_grid_model::BranchIdx;
using power_grid_model::BranchShortCircuitSolverOutput;
using power_grid_model::BranchSolverOutput;
using power_grid_model::BusSolverOutput;
using power_grid_model::CurrentSensorCalcParam;
using power_grid_model::FaultCalcParam;
using power_grid_model::FaultShortCircuitSolverOutput;
using power_grid_model::MathModelTopology;
using power_grid_model::PowerSensorCalcParam;
using power_grid_model::sensor_calc_param_type;
using power_grid_model::SourceCalcParam;
using power_grid_model::TransformerTapPosition;
using power_grid_model::TransformerTapPositionOutput;
using power_grid_model::TransformerTapRegulatorCalcParam;
using power_grid_model::VoltageRegulatorCalcParam;
using power_grid_model::VoltageRegulatorSolverOutput;
using power_grid_model::VoltageSensorCalcParam;

using power_grid_model::math_model_param_c;
using power_grid_model::MathModelParam;
using power_grid_model::MathModelParamIncrement;

using power_grid_model::asymmetric_calculation_input_type;
using power_grid_model::calculation_input_type;
using power_grid_model::PowerFlowInput;
using power_grid_model::ShortCircuitInput;
using power_grid_model::StateEstimationInput;
using power_grid_model::symmetric_calculation_input_type;

using power_grid_model::asymmetric_solver_output_type;
using power_grid_model::short_circuit_solver_output_type;
using power_grid_model::ShortCircuitSolverOutput;
using power_grid_model::solver_output_t;
using power_grid_model::solver_output_type;
using power_grid_model::SolverOutput;
using power_grid_model::steady_state_solver_output_type;
using power_grid_model::symmetric_solver_output_type;

using power_grid_model::OptimizerOutput;

using power_grid_model::MathOutput;
using power_grid_model::SupernodeOutput;
using power_grid_model::UserNodeValueVector;

using power_grid_model::Branch3Connected;
using power_grid_model::BranchConnected;
using power_grid_model::ComponentConnections;
using power_grid_model::ComponentToMathCoupling;
using power_grid_model::ComponentTopology;
using power_grid_model::ComponentToTopoNodeCoupling;
using power_grid_model::Idx2DBranch3;
using power_grid_model::ReducedComponentTopology;
using power_grid_model::ReducedTopology;
using power_grid_model::TopologicalComponentToMathCoupling;
using power_grid_model::TopologicalNode;
using power_grid_model::TopologicalNodesAndCoupling;
} // namespace power_grid_model

template class std::vector<power_grid_model::Idx2DBranch3>;
template class std::vector<power_grid_model::FaultCalcParam>;
template class std::vector<power_grid_model::SourceCalcParam>;
template class std::vector<power_grid_model::BranchCalcParam<power_grid_model::symmetric_t>>;
// non-template classes: odr-use the destructor from a function with module linkage
void power_grid_model_emit_math_model_param_dtor(power_grid_model::MathModelParam<power_grid_model::asymmetric_t>* p) {
    p->~MathModelParam();
}
void power_grid_model_emit_component_to_topo_node_coupling_dtor(power_grid_model::ComponentToTopoNodeCoupling* p) {
    p->~ComponentToTopoNodeCoupling();
}
