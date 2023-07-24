// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_LINEAR_SE_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_LINEAR_SE_SOLVER_HPP

/*
iterative linear state estimation solver
*/

#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "../timer.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// block class for the unknown vector and/or right-hand side in state estimation equation
template <bool sym>
struct SEUnknown : public Block<DoubleComplex, sym, false, 2> {
    template <int r, int c>
    using GetterType = typename Block<DoubleComplex, sym, false, 2>::template GetterType<r, c>;

    // eigen expression
    using Block<DoubleComplex, sym, false, 2>::Block;
    using Block<DoubleComplex, sym, false, 2>::operator=;

    GetterType<0, 0> u() {
        return this->template get_val<0, 0>();
    }
    GetterType<1, 0> phi() {
        return this->template get_val<1, 0>();
    }

    GetterType<0, 0> eta() {
        return this->template get_val<0, 0>();
    }
    GetterType<1, 0> tau() {
        return this->template get_val<1, 0>();
    }
};

// block class for the right hand side in state estimation equation
template <bool sym>
using SERhs = SEUnknown<sym>;

// class of 2*2 (6*6) se gain block
/*
[
   [G, QH]
   [Q, R ]
]
*/
template <bool sym>
class SEGainBlock : public Block<DoubleComplex, sym, true, 2> {
   public:
    template <int r, int c>
    using GetterType = typename Block<DoubleComplex, sym, true, 2>::template GetterType<r, c>;

    // eigen expression
    using Block<DoubleComplex, sym, true, 2>::Block;
    using Block<DoubleComplex, sym, true, 2>::operator=;

    GetterType<0, 0> g() {
        return this->template get_val<0, 0>();
    }
    GetterType<0, 1> qh() {
        return this->template get_val<0, 1>();
    }
    GetterType<1, 0> q() {
        return this->template get_val<1, 0>();
    }
    GetterType<1, 1> r() {
        return this->template get_val<1, 1>();
    }
};

// processed measurement struct
// combined all measurement of the same quantity
// accumulate for bus injection measurement
template <bool sym>
class MeasuredValues {
    static constexpr Idx disconnected = -1;
    static constexpr Idx unmeasured = -2;
    static constexpr Idx undefined = -3;

    // struct to store bus injection information
    struct BusInjection {
        // The index in main_value_ where the total measured bus injection is stored.
        // This includes node injection measurements, source power measurements and load/gen power measurements.
        Idx idx_bus_injection{undefined};

        // The number of unmeasured appliances
        Idx n_unmeasured_appliances = 0;
    };

   public:
    // construct
    MeasuredValues(YBus<sym> const& y_bus, StateEstimationInput<sym> const& input)
        : math_topology_{y_bus.shared_topology()},
          bus_appliance_injection_(math_topology().n_bus()),
          idx_voltage_(math_topology().n_bus()),
          bus_injection_(math_topology().n_bus()),
          idx_branch_from_power_(math_topology().n_branch()),
          idx_branch_to_power_(math_topology().n_branch()),
          idx_shunt_power_(math_topology().n_shunt()),
          idx_load_gen_power_(math_topology().n_load_gen()),
          idx_source_power_(math_topology().n_source()),
          n_angle_{},
          // default angle shift
          // sym: 0
          // asym: 0, -120deg, -240deg
          mean_angle_shift_{arg(ComplexValue<sym>{1.0})} {
        // loop bus
        process_bus_related_measurements(input);
        // loop branch
        process_branch_measurements(input);
        // normalize
        normalize_variance();
    }

    // checker of measured data, return true if measurement is available
    bool has_voltage(Idx bus) const {
        return idx_voltage_[bus] >= 0;
    }
    bool has_bus_injection(Idx bus) const {
        return bus_injection_[bus].idx_bus_injection >= 0;
    }
    bool has_branch_from(Idx branch) const {
        return idx_branch_from_power_[branch] >= 0;
    }
    bool has_branch_to(Idx branch) const {
        return idx_branch_to_power_[branch] >= 0;
    }
    bool has_shunt(Idx shunt) const {
        return idx_shunt_power_[shunt] >= 0;
    }
    bool has_load_gen(Idx load_gen) const {
        return idx_load_gen_power_[load_gen] >= 0;
    }
    bool has_source(Idx source) const {
        return idx_source_power_[source] >= 0;
    }
    bool has_angle() const {
        return n_angle_ > 0;
    }

    // getter of measurement and variance
    // if the obj is not measured, it is undefined behaviour to call this function
    // use checker first

    // getter of voltage variance
    double voltage_var(Idx bus) const {
        return main_value_[idx_voltage_[bus]].variance;
    }
    // getter of voltage value for all buses
    // for no measurement, the voltage phasor is used from the current iteration
    // for magnitude only measurement, angle is added from the current iteration
    // for magnitude and angle measurement, the measured phasor is returned
    ComplexValueVector<sym> voltage(ComplexValueVector<sym> const& current_u) const {
        ComplexValueVector<sym> u(current_u.size());
        for (Idx bus = 0; bus != (Idx)current_u.size(); ++bus) {
            // no measurement
            if (idx_voltage_[bus] == unmeasured) {
                u[bus] = current_u[bus];
            }
            // no angle measurement
            else if (is_nan(imag(main_value_[idx_voltage_[bus]].value))) {
                u[bus] = real(main_value_[idx_voltage_[bus]].value) * current_u[bus] /
                         cabs(current_u[bus]);  // U / |U| to get angle shift
            }
            // full measurement
            else {
                u[bus] = main_value_[idx_voltage_[bus]].value;
            }
        }
        return u;
    }

    // power measurement
    SensorCalcParam<sym> const& bus_injection(Idx bus) const {
        return main_value_[bus_injection_[bus].idx_bus_injection];
    }
    SensorCalcParam<sym> const& branch_from_power(Idx branch) const {
        return main_value_[idx_branch_from_power_[branch]];
    }
    SensorCalcParam<sym> const& branch_to_power(Idx branch) const {
        return main_value_[idx_branch_to_power_[branch]];
    }
    SensorCalcParam<sym> const& shunt_power(Idx shunt) const {
        return main_value_[idx_shunt_power_[shunt]];
    }
    SensorCalcParam<sym> const& load_gen_power(Idx load_gen) const {
        return extra_value_[idx_load_gen_power_[load_gen]];
    }
    SensorCalcParam<sym> const& source_power(Idx source) const {
        return extra_value_[idx_source_power_[source]];
    }

    // getter mean angle shift
    RealValue<sym> mean_angle_shift() const {
        return mean_angle_shift_;
    }
    bool has_angle_measurement() const {
        return n_angle_ > 0;
    }

    // calculate load_gen and source flow
    // with given bus voltage and bus current injection
    using FlowVector = std::vector<ApplianceMathOutput<sym>>;
    using LoadGenSourceFlow = std::pair<FlowVector, FlowVector>;

    LoadGenSourceFlow calculate_load_gen_source(ComplexValueVector<sym> const& u,
                                                ComplexValueVector<sym> const& s) const {
        std::vector<ApplianceMathOutput<sym>> load_gen_flow(math_topology_->n_load_gen());
        std::vector<ApplianceMathOutput<sym>> source_flow(math_topology_->n_source());
        // loop all buses
        for (Idx bus = 0; bus != math_topology_->n_bus(); ++bus) {
            Idx const load_gen_begin = math_topology_->load_gen_bus_indptr[bus];
            Idx const load_gen_end = math_topology_->load_gen_bus_indptr[bus + 1];
            Idx const source_begin = math_topology_->source_bus_indptr[bus];
            Idx const source_end = math_topology_->source_bus_indptr[bus + 1];

            // under-determined or exactly determined
            if (bus_injection_[bus].n_unmeasured_appliances > 0) {
                calculate_non_over_determined_injection(
                    bus_injection_[bus].n_unmeasured_appliances, load_gen_begin, load_gen_end, source_begin, source_end,
                    bus_appliance_injection_[bus], s[bus], load_gen_flow, source_flow);
            }
            // over-determined
            else {
                calculate_over_determined_injection(load_gen_begin, load_gen_end, source_begin, source_end,
                                                    bus_appliance_injection_[bus], s[bus], load_gen_flow, source_flow);
            }
            // current injection
            for (Idx load_gen = load_gen_begin; load_gen != load_gen_end; ++load_gen) {
                load_gen_flow[load_gen].i = conj(load_gen_flow[load_gen].s / u[bus]);
            }
            for (Idx source = source_begin; source != source_end; ++source) {
                source_flow[source].i = conj(source_flow[source].s / u[bus]);
            }
        }

        return std::make_pair(load_gen_flow, source_flow);
    }

   private:
    // cache topology
    std::shared_ptr<MathModelTopology const> math_topology_;

    // flat array of all the relevant measurement for the main calculation
    // branch/shunt flow, bus voltage, injection flow
    std::vector<SensorCalcParam<sym>> main_value_;
    // flat array of all the load_gen/source measurement
    // not relevant for the main calculation, as extra data for load_gen/source calculation
    std::vector<SensorCalcParam<sym>> extra_value_;
    // array of total appliance injection measurement per bus, regardless of the bus has all applianced measured or not
    std::vector<SensorCalcParam<sym>> bus_appliance_injection_;

    // indexing array of the entries
    // for unmeasured (non bus injection): connected, but no measurement
    // for disconnected (non bus injection): not connected
    // for bus_injection_, there is a separate struct, see BusInjection
    // relevant for main value
    IdxVector idx_voltage_;
    std::vector<BusInjection> bus_injection_;
    IdxVector idx_branch_from_power_;
    IdxVector idx_branch_to_power_;
    IdxVector idx_shunt_power_;
    // relevant for extra value
    IdxVector idx_load_gen_power_;
    IdxVector idx_source_power_;
    // number of angle measurement
    Idx n_angle_;
    // average angle shift of voltages with angle measurement
    // default is zero is no voltage has angle measurement
    RealValue<sym> mean_angle_shift_;

    MathModelTopology const& math_topology() const {
        return *math_topology_;
    }

    void process_bus_related_measurements(StateEstimationInput<sym> const& input) {
        /*
        The main purpose of this function is to aggregate all voltage and power sensor values to
            one voltage sensor value per bus.
            one injection power sensor value per bus.
            one power sensor value per shunt (in injection reference direction, note shunt itself is not considered as
        injection element).


        This function loops through all buses
        For each bus all voltage sensor measurements are combined in a weighted average, which is appended to
        main_value_. For each bus, for all connected components, all power sensor measurements (per component (shunt,
        load_gen, source)) are combined in a weighted average, which is appended to main_value_ (for shunt) or
        extra_value_ (for load_gen and source). E.g. a value in extra_value contains the weighted average of all sensors
        connected to one component. The extra_value_ of all load_gen and source, connected to the bus, are added and
        appended to appliace_injection_measurement.

        We combine all the available load_gen and source measurements into appliance_injection_measurement by summing
        them up, and store it in bus_appliance_injection_. If all the connected load_gen and source are measured, we
        further combine the appliance_injection_measurement into the (if available) direct bus injection measurement,
        and put it into main_value_.

        NOTE: if all load_gen and source are not connected (disconnected). It is a zero injection constraint,
        which is considered as a measurement in the main_value_ with zero variance.

        The voltage values in main_value_ can be found using idx_voltage.
        The power values in main_value_ can be found using bus_injection_ (for combined load_gen and source)
        and idx_shunt_power_ (for shunt).
        */
        MathModelTopology const& topo = math_topology();
        RealValue<sym> angle_cum{};  // cumulative angle
        for (Idx bus = 0; bus != topo.n_bus(); ++bus) {
            // voltage
            {
                Idx const begin = topo.voltage_sensor_indptr[bus];
                Idx const end = topo.voltage_sensor_indptr[bus + 1];
                if (begin == end) {
                    idx_voltage_[bus] = unmeasured;
                }
                else {
                    idx_voltage_[bus] = (Idx)main_value_.size();
                    // check if there is nan
                    if (std::any_of(input.measured_voltage.cbegin() + begin, input.measured_voltage.cbegin() + end,
                                    [](auto const& x) {
                                        return is_nan(imag(x.value));
                                    })) {
                        // only keep magnitude
                        main_value_.push_back(combine_measurements<true>(input.measured_voltage, begin, end));
                    }
                    else {
                        // keep complex number
                        main_value_.push_back(combine_measurements(input.measured_voltage, begin, end));
                        ++n_angle_;
                        // accumulate angle, offset by intrinsic phase shift
                        angle_cum += arg(main_value_.back().value * std::exp(-1.0i * topo.phase_shift[bus]));
                    }
                }
            }
            // shunt
            process_bus_objects(bus, topo.shunt_bus_indptr, topo.shunt_power_sensor_indptr, input.shunt_status,
                                input.measured_shunt_power, main_value_, idx_shunt_power_);
            // injection
            // load_gen
            process_bus_objects(bus, topo.load_gen_bus_indptr, topo.load_gen_power_sensor_indptr, input.load_gen_status,
                                input.measured_load_gen_power, extra_value_, idx_load_gen_power_);
            // source
            process_bus_objects(bus, topo.source_bus_indptr, topo.source_power_sensor_indptr, input.source_status,
                                input.measured_source_power, extra_value_, idx_source_power_);

            // combine load_gen/source to injection measurement
            {
                // if all the connected load_gen/source are measured, their sum can be considered as an injection
                // measurement. zero injection (no connected appliances) is also considered as measured
                Idx n_unmeasured = 0;
                SensorCalcParam<sym> appliance_injection_measurement{};

                for (Idx load_gen = topo.load_gen_bus_indptr[bus]; load_gen != topo.load_gen_bus_indptr[bus + 1];
                     ++load_gen) {
                    if (idx_load_gen_power_[load_gen] == unmeasured) {
                        ++n_unmeasured;
                        continue;
                    }
                    else if (idx_load_gen_power_[load_gen] == disconnected) {
                        continue;
                    }
                    appliance_injection_measurement.value += extra_value_[idx_load_gen_power_[load_gen]].value;
                    appliance_injection_measurement.variance += extra_value_[idx_load_gen_power_[load_gen]].variance;
                }

                for (Idx source = topo.source_bus_indptr[bus]; source != topo.source_bus_indptr[bus + 1]; ++source) {
                    if (idx_source_power_[source] == unmeasured) {
                        ++n_unmeasured;
                        continue;
                    }
                    else if (idx_source_power_[source] == disconnected) {
                        continue;
                    }
                    appliance_injection_measurement.value += extra_value_[idx_source_power_[source]].value;
                    appliance_injection_measurement.variance += extra_value_[idx_source_power_[source]].variance;
                }
                bus_appliance_injection_[bus] = appliance_injection_measurement;
                bus_injection_[bus].n_unmeasured_appliances = n_unmeasured;

                // get direct bus injection measurement, it will has infinite variance if there is no direct bus
                // injection measurement
                SensorCalcParam<sym> const direct_injection_measurement =
                    combine_measurements(input.measured_bus_injection, topo.bus_power_sensor_indptr[bus],
                                         topo.bus_power_sensor_indptr[bus + 1]);

                // combine valid appliance_injection_measurement and direct_injection_measurement
                // three scenarios, check if we have valid injection measurement
                if (n_unmeasured == 0 || !std::isinf(direct_injection_measurement.variance)) {
                    bus_injection_[bus].idx_bus_injection = (Idx)main_value_.size();
                    if (n_unmeasured > 0) {
                        // only direct injection
                        main_value_.push_back(direct_injection_measurement);
                    }
                    else if (std::isinf(direct_injection_measurement.variance) ||
                             appliance_injection_measurement.variance == 0.0) {
                        // only appliance injection if
                        //    there is no direct injection measurement,
                        //    or we have zero injection
                        main_value_.push_back(appliance_injection_measurement);
                    }
                    else {
                        // both valid, we combine again
                        main_value_.push_back(combine_measurements(
                            {direct_injection_measurement, appliance_injection_measurement}, 0, 2));
                    }
                }
                else {
                    bus_injection_[bus].idx_bus_injection = unmeasured;
                }
            }
        }
        // assign a meaningful mean angle shift, if at least one voltage has angle measurement
        if (n_angle_ > 0) {
            mean_angle_shift_ = angle_cum / n_angle_;
        }
    }

    void process_branch_measurements(StateEstimationInput<sym> const& input) {
        /*
        The main purpose of this function is to aggregate all power sensor values to one power sensor value per branch
        side.

        This function loops through all branches.
        The branch_bus_idx contains the from and to bus indexes of the branch, or disconnected if the branch is not
        connected at that side. For each branch the checker checks if the from and to side are connected by checking if
        branch_bus_idx = disconnected.

        If the branch_bus_idx = disconnected, idx_branch_to_power_/idx_branch_from_power_ is set to disconnected.
        If the side is connected, but there are no measurements in this branch side
        idx_branch_to_power_/idx_branch_from_power_ is set to disconnected.
        Else, idx_branch_to_power_/idx_branch_from_power_ is set to the index of the aggregated data in main_value_.

        All measurement values for a single side of a branch are combined in a weighted average, which is appended to
        main_value_. The power values in main_value_ can be found using idx_branch_to_power_/idx_branch_from_power_.
        */
        MathModelTopology const& topo = math_topology();
        static constexpr auto branch_from_checker = [](BranchIdx x) -> bool {
            return x[0] != -1;
        };
        static constexpr auto branch_to_checker = [](BranchIdx x) -> bool {
            return x[1] != -1;
        };
        for (Idx branch = 0; branch != topo.n_branch(); ++branch) {
            // from side
            idx_branch_from_power_[branch] =
                process_one_object(branch, topo.branch_from_power_sensor_indptr, topo.branch_bus_idx,
                                   input.measured_branch_from_power, main_value_, branch_from_checker);
            // to side
            idx_branch_to_power_[branch] =
                process_one_object(branch, topo.branch_to_power_sensor_indptr, topo.branch_bus_idx,
                                   input.measured_branch_to_power, main_value_, branch_to_checker);
        }
    }

    // combine multiple measurements of one quantity
    // using Kalman filter
    // if only_magnitude = true, combine the abs value of the individual data
    //      set imag part to nan, to signal this is a magnitude only measurement
    template <bool only_magnitude = false>
    static SensorCalcParam<sym> combine_measurements(std::vector<SensorCalcParam<sym>> const& data, Idx begin,
                                                     Idx end) {
        double accumulated_inverse_variance = 0.0;
        ComplexValue<sym> accumulated_value{};
        for (Idx pos = begin; pos != end; ++pos) {
            accumulated_inverse_variance += 1.0 / data[pos].variance;
            if constexpr (only_magnitude) {
                ComplexValue<sym> abs_value = piecewise_complex_value<sym>(DoubleComplex{0.0, nan});
                if (is_nan(imag(data[pos].value))) {
                    abs_value += real(data[pos].value);  // only keep real part
                }
                else {
                    abs_value += cabs(data[pos].value);  // get abs of the value
                }
                accumulated_value += abs_value / data[pos].variance;
            }
            else {
                // accumulate value
                accumulated_value += data[pos].value / data[pos].variance;
            }
        }
        return SensorCalcParam<sym>{accumulated_value / accumulated_inverse_variance,
                                    1.0 / accumulated_inverse_variance};
    }

    // process objects in batch for shunt, load_gen, source
    // return the status of the object type, if all the connected objects are measured
    static void process_bus_objects(Idx const bus, IdxVector const& obj_indptr, IdxVector const& sensor_indptr,
                                    IntSVector const& obj_status, std::vector<SensorCalcParam<sym>> const& input_data,
                                    std::vector<SensorCalcParam<sym>>& result_data, IdxVector& result_idx) {
        for (Idx obj = obj_indptr[bus]; obj != obj_indptr[bus + 1]; ++obj) {
            result_idx[obj] = process_one_object(obj, sensor_indptr, obj_status, input_data, result_data);
        }
    }

    // process one object
    static constexpr auto default_status_checker = [](auto x) -> bool {
        return x;
    };
    template <class TS, class StatusChecker = decltype(default_status_checker)>
    static Idx process_one_object(Idx const obj, IdxVector const& sensor_indptr, std::vector<TS> const& obj_status,
                                  std::vector<SensorCalcParam<sym>> const& input_data,
                                  std::vector<SensorCalcParam<sym>>& result_data,
                                  StatusChecker status_checker = default_status_checker) {
        Idx const begin = sensor_indptr[obj];
        Idx const end = sensor_indptr[obj + 1];
        if (!status_checker(obj_status[obj])) {
            return disconnected;
        }
        if (begin == end) {
            return unmeasured;
        }
        result_data.push_back(combine_measurements(input_data, begin, end));
        return (Idx)result_data.size() - 1;
    }

    // normalize the variance in the main value
    // pick the smallest variance (except zero, which is a constraint)
    // scale the smallest variance to one
    // in the gain matrix, the biggest weighting factor is then one
    void normalize_variance() {
        // loop to find min_var
        double min_var = std::numeric_limits<double>::infinity();
        for (SensorCalcParam<sym> const& x : main_value_) {
            // only non-zero variance is considered
            if (x.variance != 0.0) {
                min_var = std::min(min_var, x.variance);
            }
        }
        // scale
        std::for_each(main_value_.begin(), main_value_.end(), [&](SensorCalcParam<sym>& x) {
            x.variance /= min_var;
        });
    }

    void calculate_non_over_determined_injection(Idx n_unmeasured, Idx load_gen_begin, Idx load_gen_end,
                                                 Idx source_begin, Idx source_end,
                                                 SensorCalcParam<sym> const& bus_appliance_injection,
                                                 ComplexValue<sym> const& s, FlowVector& load_gen_flow,
                                                 FlowVector& source_flow) const {
        // calculate residual, divide, and assign to unmeasured (but connected) appliances
        ComplexValue<sym> const s_residual_per_appliance = (s - bus_appliance_injection.value) / (double)n_unmeasured;
        for (Idx load_gen = load_gen_begin; load_gen != load_gen_end; ++load_gen) {
            if (has_load_gen(load_gen)) {
                load_gen_flow[load_gen].s = load_gen_power(load_gen).value;
            }
            else if (idx_load_gen_power_[load_gen] == unmeasured) {
                load_gen_flow[load_gen].s = s_residual_per_appliance;
            }
        }
        for (Idx source = source_begin; source != source_end; ++source) {
            if (has_source(source)) {
                source_flow[source].s = source_power(source).value;
            }
            else if (idx_source_power_[source] == unmeasured) {
                source_flow[source].s = s_residual_per_appliance;
            }
        }
    }

    void calculate_over_determined_injection(Idx load_gen_begin, Idx load_gen_end, Idx source_begin, Idx source_end,
                                             SensorCalcParam<sym> const& bus_appliance_injection,
                                             ComplexValue<sym> const& s, FlowVector& load_gen_flow,
                                             FlowVector& source_flow) const {
        // residual normalized by variance
        // mu = (sum[S_i] - S_cal) / sum[variance]
        ComplexValue<sym> const mu = (bus_appliance_injection.value - s) / bus_appliance_injection.variance;
        // S_i = S_i_mea - var_i * mu
        for (Idx load_gen = load_gen_begin; load_gen != load_gen_end; ++load_gen) {
            if (has_load_gen(load_gen)) {
                load_gen_flow[load_gen].s = load_gen_power(load_gen).value - (load_gen_power(load_gen).variance) * mu;
            }
        }
        for (Idx source = source_begin; source != source_end; ++source) {
            if (has_source(source)) {
                source_flow[source].s = source_power(source).value - (source_power(source).variance) * mu;
            }
        }
    }
};

template class MeasuredValues<true>;
template class MeasuredValues<false>;

// solver
template <bool sym>
class IterativeLinearSESolver {
    // block size 2 for symmetric, 6 for asym
    static constexpr Idx bsr_block_size_ = sym ? 2 : 6;

   public:
    IterativeLinearSESolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          math_topo_{topo_ptr},
          data_gain_(y_bus.nnz_lu()),
          x_rhs_(y_bus.size()),
          sparse_solver_{y_bus.shared_indptr_lu(), y_bus.shared_indices_lu(), y_bus.shared_diag_lu()},
          perm_(y_bus.size()) {
    }

    MathOutput<sym> run_state_estimation(YBus<sym> const& y_bus, StateEstimationInput<sym> const& input, double err_tol,
                                         Idx max_iter, CalculationInfo& calculation_info) {
        // prepare
        Timer main_timer, sub_timer;
        MathOutput<sym> output;
        output.u.resize(n_bus_);
        output.bus_injection.resize(n_bus_);
        double max_dev = std::numeric_limits<double>::max();

        main_timer = Timer(calculation_info, 2220, "Math solver");

        // preprocess measured value
        sub_timer = Timer(calculation_info, 2221, "Pre-process measured value");
        MeasuredValues<sym> const measured_values{y_bus, input};

        // prepare matrix, including pre-factorization
        sub_timer = Timer(calculation_info, 2222, "Prepare matrix, including pre-factorization");
        prepare_matrix(y_bus, measured_values);

        // initialize voltage with initial angle
        sub_timer = Timer(calculation_info, 2223, "Initialize voltages");
        RealValue<sym> const mean_angle_shift = measured_values.mean_angle_shift();
        for (Idx bus = 0; bus != n_bus_; ++bus) {
            output.u[bus] = exp(1.0i * (mean_angle_shift + math_topo_->phase_shift[bus]));
        }

        // loop to iterate
        Idx num_iter = 0;
        do {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            sub_timer = Timer(calculation_info, 2224, "Calculate rhs");
            prepare_rhs(y_bus, measured_values, output.u);
            // solve with prefactorization
            sub_timer = Timer(calculation_info, 2225, "Solve sparse linear equation (pre-factorized)");
            sparse_solver_.solve_with_prefactorized_matrix(data_gain_, perm_, x_rhs_, x_rhs_);
            sub_timer = Timer(calculation_info, 2226, "Iterate unknown");
            max_dev = iterate_unknown(output.u, measured_values.has_angle_measurement());
        } while (max_dev > err_tol);

        // calculate math result
        sub_timer = Timer(calculation_info, 2227, "Calculate Math Result");
        calculate_result(y_bus, measured_values, output);

        // Manually stop timers to avoid "Max number of iterations" to be included in the timing.
        sub_timer.stop();
        main_timer.stop();

        const auto key = Timer::make_key(2228, "Max number of iterations");
        calculation_info[key] = std::max(calculation_info[key], (double)num_iter);

        return output;
    }

   private:
    // array selection function pointer
    static constexpr std::array has_branch_{&MeasuredValues<sym>::has_branch_from, &MeasuredValues<sym>::has_branch_to};
    static constexpr std::array branch_power_{&MeasuredValues<sym>::branch_from_power,
                                              &MeasuredValues<sym>::branch_to_power};

    Idx n_bus_;
    // shared topo data
    std::shared_ptr<MathModelTopology const> math_topo_;

    // data for gain matrix
    std::vector<SEGainBlock<sym>> data_gain_;
    // unknown and rhs
    std::vector<SERhs<sym>> x_rhs_;
    // solver
    SparseLUSolver<SEGainBlock<sym>, SERhs<sym>, SEUnknown<sym>> sparse_solver_;
    typename SparseLUSolver<SEGainBlock<sym>, SERhs<sym>, SEUnknown<sym>>::BlockPermArray perm_;

    void prepare_matrix(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value) {
        MathModelParam<sym> const& param = y_bus.math_model_param();
        IdxVector const& row_indptr = y_bus.row_indptr_lu();
        IdxVector const& col_indices = y_bus.col_indices_lu();

        // loop data index, all rows and columns
        for (Idx row = 0; row != n_bus_; ++row) {
            for (Idx data_idx_lu = row_indptr[row]; data_idx_lu != row_indptr[row + 1]; ++data_idx_lu) {
                Idx const col = col_indices[data_idx_lu];
                // get a reference and reset block to zero
                SEGainBlock<sym>& block = data_gain_[data_idx_lu];
                block = SEGainBlock<sym>{};
                // get data idx of y bus,
                // skip for a fill-in
                Idx const data_idx = y_bus.map_lu_y_bus()[data_idx_lu];
                if (data_idx == -1) {
                    continue;
                }
                // fill block with voltage measurement, only diagonal
                if ((row == col) && measured_value.has_voltage(row)) {
                    // G += 1.0 / variance
                    // for 3x3 tensor, fill diagonal
                    block.g() += ComplexTensor<sym>{1.0 / measured_value.voltage_var(row)};
                }
                // fill block with branch, shunt measurement
                for (Idx element_idx = y_bus.y_bus_entry_indptr()[data_idx];
                     element_idx != y_bus.y_bus_entry_indptr()[data_idx + 1]; ++element_idx) {
                    Idx const obj = y_bus.y_bus_element()[element_idx].idx;
                    YBusElementType const type = y_bus.y_bus_element()[element_idx].element_type;
                    // shunt
                    if (type == YBusElementType::shunt) {
                        if (measured_value.has_shunt(obj)) {
                            // G += Ys^H * Ys / variance
                            block.g() += dot(hermitian_transpose(param.shunt_param[obj]), param.shunt_param[obj]) /
                                         measured_value.shunt_power(obj).variance;
                        }
                    }
                    // branch
                    else {
                        // branch from- and to-side index at 0, and 1 position
                        IntS const b0 = static_cast<IntS>(type) / 2;
                        IntS const b1 = static_cast<IntS>(type) % 2;
                        // measured at from-side: 0, to-side: 1
                        for (IntS const measured_side : std::array<IntS, 2>{0, 1}) {
                            // has measurement
                            if (std::invoke(has_branch_[measured_side], measured_value, obj)) {
                                // G += Y{side, b0}^H * Y{side, b1} / variance
                                block.g() +=
                                    dot(hermitian_transpose(param.branch_param[obj].value[measured_side * 2 + b0]),
                                        param.branch_param[obj].value[measured_side * 2 + b1]) /
                                    std::invoke(branch_power_[measured_side], measured_value, obj).variance;
                            }
                        }
                    }
                }
                // fill block with injection measurement
                // injection measurement exist
                if (measured_value.has_bus_injection(row)) {
                    // Q_ij = Y_bus_ij
                    block.q() = y_bus.admittance()[data_idx];
                    // R_ii = -variance, only diagonal
                    if (row == col) {
                        // assign variance to diagonal of 3x3 tensor, for asym
                        block.r() = ComplexTensor<sym>{-measured_value.bus_injection(row).variance};
                    }
                }
                // injection measurement not exist
                else {
                    // Q_ij = 0
                    // R_ii = -1.0, only diagonal
                    // assign -1.0 to diagonal of 3x3 tensor, for asym
                    if (row == col) {
                        block.r() = ComplexTensor<sym>{-1.0};
                    }
                }
            }
        }

        // loop all transpose entry for QH
        // assign the hermitian transpose of the transpose entry of Q
        for (Idx data_idx_lu = 0; data_idx_lu != y_bus.nnz_lu(); ++data_idx_lu) {
            // skip for fill-in
            if (y_bus.map_lu_y_bus()[data_idx_lu] == -1) {
                continue;
            }
            Idx const data_idx_tranpose = y_bus.lu_transpose_entry()[data_idx_lu];
            data_gain_[data_idx_lu].qh() = hermitian_transpose(data_gain_[data_idx_tranpose].q());
        }
        // prefactorize
        sparse_solver_.prefactorize(data_gain_, perm_);
    }

    void prepare_rhs(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value,
                     ComplexValueVector<sym> const& current_u) {
        MathModelParam<sym> const& param = y_bus.math_model_param();
        std::vector<BranchIdx> const branch_bus_idx = y_bus.math_topology().branch_bus_idx;
        // get generated (measured/estimated) voltage phasor
        // with current result voltage angle
        ComplexValueVector<sym> u = measured_value.voltage(current_u);

        // loop all bus to fill rhs
        for (Idx bus = 0; bus != n_bus_; ++bus) {
            Idx const data_idx = y_bus.bus_entry()[bus];
            // reset rhs block to fill values
            SERhs<sym>& rhs_block = x_rhs_[bus];
            rhs_block = SERhs<sym>{};
            // fill block with voltage measurement
            if (measured_value.has_voltage(bus)) {
                // eta += u / variance
                rhs_block.eta() += u[bus] / measured_value.voltage_var(bus);
            }
            // fill block with branch, shunt measurement, need to convert to current
            for (Idx element_idx = y_bus.y_bus_entry_indptr()[data_idx];
                 element_idx != y_bus.y_bus_entry_indptr()[data_idx + 1]; ++element_idx) {
                Idx const obj = y_bus.y_bus_element()[element_idx].idx;
                YBusElementType const type = y_bus.y_bus_element()[element_idx].element_type;
                // shunt
                if (type == YBusElementType::shunt) {
                    if (measured_value.has_shunt(obj)) {
                        SensorCalcParam<sym> const& m = measured_value.shunt_power(obj);
                        // eta -= Ys^H * i_shunt / variance
                        rhs_block.eta() -=
                            dot(hermitian_transpose(param.shunt_param[obj]), conj(m.value / u[bus])) / m.variance;
                    }
                }
                // branch
                else {
                    // branch is either ff or tt
                    IntS const b = static_cast<IntS>(type) / 2;
                    assert(b == static_cast<IntS>(type) % 2);
                    // measured at from-side: 0, to-side: 1
                    for (IntS const measured_side : std::array<IntS, 2>{0, 1}) {
                        // has measurement
                        if (std::invoke(has_branch_[measured_side], measured_value, obj)) {
                            SensorCalcParam<sym> const& m =
                                std::invoke(branch_power_[measured_side], measured_value, obj);
                            // the current needs to be calculated with the voltage of the measured bus side
                            // NOTE: not the current bus!
                            Idx const measured_bus = branch_bus_idx[obj][measured_side];
                            // eta += Y{side, b}^H * i_branch_{f, t} / variance
                            rhs_block.eta() +=
                                dot(hermitian_transpose(param.branch_param[obj].value[measured_side * 2 + b]),
                                    conj(m.value / u[measured_bus])) /
                                m.variance;
                        }
                    }
                }
            }
            // fill block with injection measurement, need to convert to current
            if (measured_value.has_bus_injection(bus)) {
                rhs_block.tau() = conj(measured_value.bus_injection(bus).value / u[bus]);
            }
        }
    }

    double iterate_unknown(ComplexValueVector<sym>& u, bool has_angle_measurement) {
        double max_dev = 0.0;
        // phase shift anti offset of slack bus, phase a
        // if no angle measurement is present
        DoubleComplex const angle_offset = [&]() -> DoubleComplex {
            if (has_angle_measurement) {
                return 1.0;
            }
            if constexpr (sym) {
                return cabs(x_rhs_[math_topo_->slack_bus_].u()) / x_rhs_[math_topo_->slack_bus_].u();
            }
            else {
                return cabs(x_rhs_[math_topo_->slack_bus_].u()(0)) / x_rhs_[math_topo_->slack_bus_].u()(0);
            }
        }();

        for (Idx bus = 0; bus != n_bus_; ++bus) {
            // phase offset to calculated voltage as normalized
            ComplexValue<sym> const u_normalized = x_rhs_[bus].u() * angle_offset;
            // get dev of last iteration, get max
            double const dev = max_val(cabs(u_normalized - u[bus]));
            max_dev = std::max(dev, max_dev);
            // assign
            u[bus] = u_normalized;
        }
        return max_dev;
    }

    void calculate_result(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value, MathOutput<sym>& output) {
        // call y bus
        output.branch = y_bus.template calculate_branch_flow<BranchMathOutput<sym>>(output.u);
        output.shunt = y_bus.template calculate_shunt_flow<ApplianceMathOutput<sym>>(output.u);
        output.bus_injection = y_bus.calculate_injection(output.u);
        std::tie(output.load_gen, output.source) =
            measured_value.calculate_load_gen_source(output.u, output.bus_injection);
    }
};

template class IterativeLinearSESolver<true>;
template class IterativeLinearSESolver<false>;

}  // namespace math_model_impl

template <bool sym>
using IterativeLinearSESolver = math_model_impl::IterativeLinearSESolver<sym>;

}  // namespace power_grid_model

#endif
