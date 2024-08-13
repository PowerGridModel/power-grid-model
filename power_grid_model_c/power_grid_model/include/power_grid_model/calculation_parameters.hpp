// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common/common.hpp"
#include "common/enum.hpp"
#include "common/grouped_index_vector.hpp"
#include "common/three_phase_tensor.hpp"

namespace power_grid_model {

// Entry of YBus, node addmittance matrix
struct YBusElement {
    YBusElementType element_type{};
    Idx idx{}; // index of the component
};

// everything here always per unit

// branch math calculation parameter and math output
template <symmetry_tag sym_type> struct BranchCalcParam {
    using sym = sym_type;

    std::array<ComplexTensor<sym>, 4> value;
    // getter
    ComplexTensor<sym>& yff() { return value[0]; }
    ComplexTensor<sym> const& yff() const { return value[0]; }
    ComplexTensor<sym>& yft() { return value[1]; }
    ComplexTensor<sym> const& yft() const { return value[1]; }
    ComplexTensor<sym>& ytf() { return value[2]; }
    ComplexTensor<sym> const& ytf() const { return value[2]; }
    ComplexTensor<sym>& ytt() { return value[3]; }
    ComplexTensor<sym> const& ytt() const { return value[3]; }
};

template <symmetry_tag sym_type> struct BranchSolverOutput {
    using sym = sym_type;

    ComplexValue<sym> s_f{};
    ComplexValue<sym> s_t{};
    ComplexValue<sym> i_f{};
    ComplexValue<sym> i_t{};
};

template <symmetry_tag sym_type> struct BranchShortCircuitSolverOutput {
    using sym = sym_type;

    ComplexValue<sym> i_f{};
    ComplexValue<sym> i_t{};
};

// fault math calculation parameters and math output
struct FaultCalcParam {
    DoubleComplex y_fault{};
    FaultType fault_type{};
    FaultPhase fault_phase{};
};

template <symmetry_tag sym_type> struct FaultShortCircuitSolverOutput {
    using sym = sym_type;

    ComplexValue<sym> i_fault{};
};

// appliance solver math output, always injection direction
// s > 0, energy appliance -> node
template <symmetry_tag sym_type> struct ApplianceSolverOutput {
    using sym = sym_type;

    ComplexValue<sym> s{};
    ComplexValue<sym> i{};
};
template <symmetry_tag sym_type> struct ApplianceShortCircuitSolverOutput {
    using sym = sym_type;

    ComplexValue<sym> i{};
};

// Complex measured value of a sensor in p.u. with a uniform variance across all phases and axes of the complex plane
// (circularly symmetric)
template <symmetry_tag sym_type> struct UniformComplexRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    ComplexValue<sym> value{};
    double variance{}; // variance (sigma^2) of the error range, in p.u.
};

// voltage sensor calculation parameters for state estimation
// The value is the complex voltage
// If the imaginary part is NaN, it means the angle calculation is not correct
template <symmetry_tag sym> using VoltageSensorCalcParam = UniformComplexRandomVariable<sym>;

// power sensor calculation parameters for state estimation
// The value is the complex power
//   * for appliances, it is always in injection direction
//   * for branches, the direction is node -> branch
template <symmetry_tag sym_type> struct PowerSensorCalcParam {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    ComplexValue<sym> value{};
    RealValue<sym> p_variance{}; // variance (sigma^2) of the error range of the active power, in p.u.
    RealValue<sym> q_variance{}; // variance (sigma^2) of the error range of the reactive power, in p.u.
};

template <typename T>
concept sensor_calc_param_type =
    std::derived_from<T, VoltageSensorCalcParam<symmetric_t>> ||
    std::derived_from<T, VoltageSensorCalcParam<asymmetric_t>> ||
    std::derived_from<T, PowerSensorCalcParam<symmetric_t>> || std::derived_from<T, PowerSensorCalcParam<asymmetric_t>>;

static_assert(sensor_calc_param_type<VoltageSensorCalcParam<symmetric_t>>);
static_assert(sensor_calc_param_type<VoltageSensorCalcParam<asymmetric_t>>);
static_assert(sensor_calc_param_type<PowerSensorCalcParam<symmetric_t>>);
static_assert(sensor_calc_param_type<PowerSensorCalcParam<asymmetric_t>>);

struct TransformerTapRegulatorCalcParam {
    double u_set{};
    double u_band{};
    DoubleComplex z_compensation{};
    IntS status{};
};

struct TransformerTapPosition {
    ID transformer_id{};
    IntS tap_position{};
};
using TransformerTapPositionOutput = std::vector<TransformerTapPosition>;

// from side, to side
// in case of indices for math model, -1 means the branch is not connected to that side
using BranchIdx = std::array<Idx, 2>;
// node 0, 1, 2 side
// in case of indices for math model, -1 means the branch is not connected to that side
using Branch3Idx = std::array<Idx, 3>;

struct MathModelTopology {
    Idx slack_bus{};
    std::vector<double> phase_shift;
    std::vector<BranchIdx> branch_bus_idx;
    std::vector<BranchIdx> fill_in;
    DenseGroupedIdxVector sources_per_bus;
    DenseGroupedIdxVector shunts_per_bus;
    SparseGroupedIdxVector load_gens_per_bus;
    std::vector<LoadGenType> load_gen_type;
    DenseGroupedIdxVector voltage_sensors_per_bus;
    DenseGroupedIdxVector power_sensors_per_source;
    DenseGroupedIdxVector power_sensors_per_load_gen;
    DenseGroupedIdxVector power_sensors_per_shunt;
    DenseGroupedIdxVector power_sensors_per_branch_from;
    DenseGroupedIdxVector power_sensors_per_branch_to;
    DenseGroupedIdxVector power_sensors_per_bus;
    DenseGroupedIdxVector tap_regulators_per_branch;

    Idx n_bus() const { return static_cast<Idx>(phase_shift.size()); }

    Idx n_branch() const { return static_cast<Idx>(branch_bus_idx.size()); }

    Idx n_source() const { return sources_per_bus.element_size(); }

    Idx n_shunt() const { return shunts_per_bus.element_size(); }

    Idx n_load_gen() const { return load_gens_per_bus.element_size(); }

    Idx n_voltage_sensor() const { return voltage_sensors_per_bus.element_size(); }

    Idx n_source_power_sensor() const { return power_sensors_per_source.element_size(); }

    Idx n_load_gen_power_sensor() const { return power_sensors_per_load_gen.element_size(); }

    Idx n_shunt_power_power_sensor() const { return power_sensors_per_shunt.element_size(); }

    Idx n_branch_from_power_sensor() const { return power_sensors_per_branch_from.element_size(); }

    Idx n_branch_to_power_sensor() const { return power_sensors_per_branch_to.element_size(); }

    Idx n_bus_power_sensor() const { return power_sensors_per_bus.element_size(); }

    Idx n_transformer_tap_regulator() const { return tap_regulators_per_branch.element_size(); }
};

struct SourceCalcParam {
    DoubleComplex y1;
    DoubleComplex y0;

    template <symmetry_tag sym> inline ComplexTensor<sym> y_ref() const {
        if constexpr (is_symmetric_v<sym>) {
            return y1;
        } else {
            return ComplexTensor<asymmetric_t>{(2.0 * y1 + y0) / 3.0, (y0 - y1) / 3.0};
        }
    }
};

template <symmetry_tag sym_type> struct MathModelParam {
    using sym = sym_type;

    std::vector<BranchCalcParam<sym>> branch_param;
    ComplexTensorVector<sym> shunt_param;
    std::vector<SourceCalcParam> source_param;
};

struct MathModelParamIncrement {
    std::vector<Idx> branch_param_to_change; // indices of changed branch_param
    std::vector<Idx> shunt_param_to_change;  // indices of changed shunt_param
};

template <symmetry_tag sym_type> struct PowerFlowInput {
    using sym = sym_type;

    ComplexVector source;                // Complex u_ref of each source
    ComplexValueVector<sym> s_injection; // Specified injection power of each load_gen
};

template <symmetry_tag sym_type> struct StateEstimationInput {
    using sym = sym_type;

    // connection status of shunt, load_gen, source
    // this is needed to determine if a measurement is relevant
    // if the shunt/load_gen/source is disconnected, all its measurements are discarded
    IntSVector shunt_status;
    IntSVector load_gen_status;
    IntSVector source_status;
    // measured value
    std::vector<VoltageSensorCalcParam<sym>> measured_voltage;
    std::vector<PowerSensorCalcParam<sym>> measured_source_power;
    std::vector<PowerSensorCalcParam<sym>> measured_load_gen_power;
    std::vector<PowerSensorCalcParam<sym>> measured_shunt_power;
    std::vector<PowerSensorCalcParam<sym>> measured_branch_from_power;
    std::vector<PowerSensorCalcParam<sym>> measured_branch_to_power;
    std::vector<PowerSensorCalcParam<sym>> measured_bus_injection;
};

struct ShortCircuitInput {
    DenseGroupedIdxVector fault_buses;
    std::vector<FaultCalcParam> faults;
    ComplexVector source; // Complex u_ref of each source
};

template <typename T>
concept symmetric_calculation_input_type =
    std::derived_from<T, PowerFlowInput<symmetric_t>> || std::derived_from<T, StateEstimationInput<symmetric_t>>;

static_assert(symmetric_calculation_input_type<PowerFlowInput<symmetric_t>>);
static_assert(symmetric_calculation_input_type<StateEstimationInput<symmetric_t>>);
static_assert(!symmetric_calculation_input_type<PowerFlowInput<asymmetric_t>>);
static_assert(!symmetric_calculation_input_type<StateEstimationInput<asymmetric_t>>);
static_assert(!symmetric_calculation_input_type<ShortCircuitInput>);

template <typename T>
concept asymmetric_calculation_input_type =
    std::derived_from<T, PowerFlowInput<asymmetric_t>> || std::derived_from<T, StateEstimationInput<asymmetric_t>> ||
    std::derived_from<T, ShortCircuitInput>;

static_assert(!asymmetric_calculation_input_type<PowerFlowInput<symmetric_t>>);
static_assert(!asymmetric_calculation_input_type<StateEstimationInput<symmetric_t>>);
static_assert(asymmetric_calculation_input_type<PowerFlowInput<asymmetric_t>>);
static_assert(asymmetric_calculation_input_type<StateEstimationInput<asymmetric_t>>);
static_assert(asymmetric_calculation_input_type<ShortCircuitInput>);

template <typename T>
concept calculation_input_type = symmetric_calculation_input_type<T> || asymmetric_calculation_input_type<T>;

static_assert(calculation_input_type<PowerFlowInput<symmetric_t>>);
static_assert(calculation_input_type<StateEstimationInput<symmetric_t>>);
static_assert(calculation_input_type<PowerFlowInput<asymmetric_t>>);
static_assert(calculation_input_type<StateEstimationInput<asymmetric_t>>);
static_assert(calculation_input_type<ShortCircuitInput>);

struct solver_output_t {};

template <symmetry_tag sym_type> struct SolverOutput {
    using type = solver_output_t;
    using sym = sym_type;

    std::vector<ComplexValue<sym>> u;
    std::vector<ComplexValue<sym>> bus_injection;
    std::vector<BranchSolverOutput<sym>> branch;
    std::vector<ApplianceSolverOutput<sym>> source;
    std::vector<ApplianceSolverOutput<sym>> shunt;
    std::vector<ApplianceSolverOutput<sym>> load_gen;
};

template <symmetry_tag sym_type> struct ShortCircuitSolverOutput {
    using type = solver_output_t;
    using sym = sym_type;

    std::vector<ComplexValue<sym>> u_bus;
    std::vector<FaultShortCircuitSolverOutput<sym>> fault;
    std::vector<BranchShortCircuitSolverOutput<sym>> branch;
    std::vector<ApplianceShortCircuitSolverOutput<sym>> source;
    std::vector<ApplianceShortCircuitSolverOutput<sym>> shunt;
};

template <typename T>
concept solver_output_type = std::derived_from<typename T::type, solver_output_t>;

static_assert(solver_output_type<SolverOutput<symmetric_t>>);
static_assert(solver_output_type<SolverOutput<asymmetric_t>>);
static_assert(solver_output_type<ShortCircuitSolverOutput<symmetric_t>>);
static_assert(solver_output_type<ShortCircuitSolverOutput<asymmetric_t>>);

template <typename T>
concept symmetric_solver_output_type = solver_output_type<T> && is_symmetric_v<typename T::sym>;

static_assert(symmetric_solver_output_type<SolverOutput<symmetric_t>>);
static_assert(!symmetric_solver_output_type<SolverOutput<asymmetric_t>>);
static_assert(symmetric_solver_output_type<ShortCircuitSolverOutput<symmetric_t>>);
static_assert(!symmetric_solver_output_type<ShortCircuitSolverOutput<asymmetric_t>>);

template <typename T>
concept asymmetric_solver_output_type = solver_output_type<T> && is_asymmetric_v<typename T::sym>;

static_assert(!asymmetric_solver_output_type<SolverOutput<symmetric_t>>);
static_assert(asymmetric_solver_output_type<SolverOutput<asymmetric_t>>);
static_assert(!asymmetric_solver_output_type<ShortCircuitSolverOutput<symmetric_t>>);
static_assert(asymmetric_solver_output_type<ShortCircuitSolverOutput<asymmetric_t>>);

template <typename T>
concept steady_state_solver_output_type = solver_output_type<T> && std::derived_from<T, SolverOutput<typename T::sym>>;

static_assert(steady_state_solver_output_type<SolverOutput<symmetric_t>>);
static_assert(steady_state_solver_output_type<SolverOutput<asymmetric_t>>);
static_assert(!steady_state_solver_output_type<ShortCircuitSolverOutput<symmetric_t>>);
static_assert(!steady_state_solver_output_type<ShortCircuitSolverOutput<asymmetric_t>>);

template <typename T>
concept short_circuit_solver_output_type =
    solver_output_type<T> && std::derived_from<T, ShortCircuitSolverOutput<typename T::sym>>;

static_assert(!short_circuit_solver_output_type<SolverOutput<symmetric_t>>);
static_assert(!short_circuit_solver_output_type<SolverOutput<asymmetric_t>>);
static_assert(short_circuit_solver_output_type<ShortCircuitSolverOutput<symmetric_t>>);
static_assert(short_circuit_solver_output_type<ShortCircuitSolverOutput<asymmetric_t>>);

struct OptimizerOutput {
    TransformerTapPositionOutput transformer_tap_positions;
};

template <typename T> struct MathOutput {
    using SolverOutputType = T;

    SolverOutputType solver_output;
    OptimizerOutput optimizer_output;
};

// component indices at physical model side
// from, to node indices for branches
// node1, node2, node3 indices for 3-way branches
// node indices for source, load_gen, shunt
struct ComponentTopology {
    Idx n_node{};
    std::vector<BranchIdx> branch_node_idx;
    std::vector<Branch3Idx> branch3_node_idx;
    IdxVector shunt_node_idx;
    IdxVector source_node_idx;
    IdxVector load_gen_node_idx;
    std::vector<LoadGenType> load_gen_type;
    IdxVector voltage_sensor_node_idx;
    IdxVector power_sensor_object_idx; // the index is relative to branch, source, shunt or load_gen
    std::vector<MeasuredTerminalType> power_sensor_terminal_type;
    std::vector<ComponentType> regulator_type;
    IdxVector regulated_object_idx; // the index is relative to branch or branch3
    std::vector<ComponentType> regulated_object_type;

    Idx n_node_total() const { return n_node + static_cast<Idx>(branch3_node_idx.size()); }
};

// connection property
using BranchConnected = std::array<IntS, 2>;
using Branch3Connected = std::array<IntS, 3>;

// component connection property at model side
struct ComponentConnections {
    std::vector<BranchConnected> branch_connected;
    std::vector<Branch3Connected> branch3_connected;
    DoubleVector branch_phase_shift;
    // 3-way branch, phase shift = phase_node_x - phase_internal_node
    std::vector<std::array<double, 3>> branch3_phase_shift;
    IntSVector source_connected;
};

// To couple 3-way branch, to math model, 3 virtual branches are created
// they are always in the same math model group
// sequence numbers of 3 virtual branches are saved in pos
struct Idx2DBranch3 {
    Idx group{};
    // 0: node 0 -> internal node
    // 1: node 1 -> internal node
    // 2: node 2 -> internal node
    std::array<Idx, 3> pos;

    friend constexpr bool operator==(Idx2DBranch3 const& x, Idx2DBranch3 const& y) = default;
};

// couple component to math model
// use Idx2D to map component to math model
//		group = math model sequence number,
//		group = -1 means isolated component
//		pos = sequence number in math model,
//		pos = -1 means not connected at that side, only applicable for branches
struct ComponentToMathCoupling {
    std::vector<Idx2D> fault;
};

// couple component to math model
// like ComponentToMathCoupling but for components that are immutable after the topology is fixed
// use Idx2D to map component to math model
//		group = math model sequence number,
//		group = -1 means isolated component
//		pos = sequence number in math model,
//		pos = -1 means not connected at that side, only applicable for branches
struct TopologicalComponentToMathCoupling {
    std::vector<Idx2D> node;
    std::vector<Idx2D> branch;
    std::vector<Idx2DBranch3> branch3;
    std::vector<Idx2D> shunt;
    std::vector<Idx2D> load_gen;
    std::vector<Idx2D> source;
    std::vector<Idx2D> voltage_sensor;
    std::vector<Idx2D> power_sensor; // can be coupled to branch-from/to, source, load_gen, or shunt sensor
    std::vector<Idx2D> regulator;
};

} // namespace power_grid_model
