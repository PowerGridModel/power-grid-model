// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_LINEAR_SE_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_LINEAR_SE_SOLVER_HPP

/*
iterative linear state estimation solver
*/

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "../timer.hpp"
#include "bsr_solver.hpp"
#include "y_bus.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// block class for the unknown vector in state estimation equation
template <bool sym>
struct SEUnknown {
    ComplexValue<sym> u;    // real unknown
    ComplexValue<sym> phi;  // artificial unknown
};
static_assert(sizeof(SEUnknown<true>) == sizeof(double[4]));
static_assert(alignof(SEUnknown<true>) == alignof(double[4]));
static_assert(std::is_standard_layout_v<SEUnknown<true>>);
static_assert(sizeof(SEUnknown<false>) == sizeof(double[12]));
static_assert(alignof(SEUnknown<false>) >= alignof(double[12]));
static_assert(std::is_standard_layout_v<SEUnknown<false>>);
// block class for the right hand side in state estimation equation
template <bool sym>
struct SERhs {
    ComplexValue<sym> eta;  // bus voltage, branch flow, shunt flow
    ComplexValue<sym> tau;  // injection flow, zero injection constraint
};
static_assert(sizeof(SERhs<true>) == sizeof(double[4]));
static_assert(alignof(SERhs<true>) == alignof(double[4]));
static_assert(std::is_standard_layout_v<SERhs<true>>);
static_assert(sizeof(SERhs<false>) == sizeof(double[12]));
static_assert(alignof(SERhs<false>) >= alignof(double[12]));
static_assert(std::is_standard_layout_v<SERhs<false>>);

// class of 2*2 (6*6) se gain block
/*
[
   [G, QH]
   [Q, R ]
]
*/
template <bool sym>
class SEGainBlock : public BlockEntry<DoubleComplex, sym> {
   public:
    using typename BlockEntry<DoubleComplex, sym>::GetterType;
    GetterType g() {
        return this->template get_val<0, 0>();
    }
    GetterType qh() {
        return this->template get_val<0, 1>();
    }
    GetterType q() {
        return this->template get_val<1, 0>();
    }
    GetterType r() {
        return this->template get_val<1, 1>();
    }
};
constexpr block_entry_trait<SEGainBlock> se_gain_trait{};

// processed measurement struct
// combined all measurement of the same quantity
// accumulate for bus injection measurement
template <bool sym>
class MeasuredValues {
   public:
    // construct
    MeasuredValues(YBus<sym> const& y_bus, StateEstimationInput<sym> const& input)
        : math_topology_{y_bus.shared_topology()},
          idx_voltage_(math_topology().n_bus()),
          idx_bus_injection_power_(math_topology().n_bus()),
          idx_branch_from_power_(math_topology().n_branch()),
          idx_branch_to_power_(math_topology().n_branch()),
          idx_shunt_power_(math_topology().n_shunt()),
          idx_load_gen_power_(math_topology().n_load_gen()),
          idx_source_power_(math_topology().n_source()),
          idx_partial_injection_(math_topology().n_bus(), -1),
          n_angle_{},
          // default angle shift
          // sym: 0
          // asym: 0, -120deg, -240deg
          mean_angle_shift_{arg(ComplexValue<sym>{1.0})},
          min_var_{} {
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
        return idx_bus_injection_power_[bus] >= 0;
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
            if (idx_voltage_[bus] == -1) {
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
    SensorCalcParam<sym> const& bus_injection_power(Idx bus) const {
        return main_value_[idx_bus_injection_power_[bus]];
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
    // first load_gen, second source
    using LoadGenSourceFlow = std::pair<std::vector<ApplianceMathOutput<sym>>, std::vector<ApplianceMathOutput<sym>>>;

    LoadGenSourceFlow calculate_load_gen_source(ComplexValueVector<sym> const& u,
                                                ComplexValueVector<sym> const& s) const {
        LoadGenSourceFlow pair{};
        pair.first.resize(math_topology_->n_load_gen());
        pair.second.resize(math_topology_->n_source());
        // loop all buses
        for (Idx bus = 0; bus != math_topology_->n_bus(); ++bus) {
            Idx const load_gen_begin = math_topology_->load_gen_bus_indptr[bus];
            Idx const load_gen_end = math_topology_->load_gen_bus_indptr[bus + 1];
            Idx const source_begin = math_topology_->source_bus_indptr[bus];
            Idx const source_end = math_topology_->source_bus_indptr[bus + 1];

            // under-determined or exactly determined
            if (idx_bus_injection_power_[bus] < 0) {
                calculate_non_over_determined_injection(-idx_bus_injection_power_[bus],  // n_unmeasured
                                                        load_gen_begin, load_gen_end, source_begin, source_end,
                                                        partial_injection_[idx_partial_injection_[bus]], s[bus], pair);
            }
            // over-determined
            else {
                calculate_over_determined_injection(load_gen_begin, load_gen_end, source_begin, source_end,
                                                    bus_injection_power(bus), s[bus], pair);
            }
            // current injection
            for (Idx load_gen = load_gen_begin; load_gen != load_gen_end; ++load_gen) {
                pair.first[load_gen].i = conj(pair.first[load_gen].s / u[bus]);
            }
            for (Idx source = source_begin; source != source_end; ++source) {
                pair.second[source].i = conj(pair.second[source].s / u[bus]);
            }
        }

        return pair;
    }

   private:
    // cache topology
    std::shared_ptr<MathModelTopology const> math_topology_;

    // flat array of all the relevant measurement for the main calculation
    // branch/shunt flow, bus voltage, injection flow
    std::vector<SensorCalcParam<sym>> main_value_;
    // flat array of all the loadgen/source measurement
    // not relevant for the main calculation, as extra data for loadgen/source calculation
    std::vector<SensorCalcParam<sym>> extra_value_;
    // array of partial injection measurement, the bus with not all connected appliances measured
    std::vector<SensorCalcParam<sym>> partial_injection_;

    // indexing array of the entries
    // for -1 (non bus injection): connected, but no measurement
    // for -2 (non bus injection): not connected
    // for <0 (bus injection): the number of unmeasured appliances in negative
    // 	   -2 means 2 appliances are unmeasured, bus-appliance is under-determined
    //     -1 means 1 appliance is unmeasured, bus-appliance is exactly determined
    // for >=0: position of this measurement in relevant flat array
    // 	    for bus injection, the bus-appliance is over-determined
    //      for bus injection with zero injection constraint,
    //            it is considered as measured with zero value and zero variance
    // relevant for main value
    IdxVector idx_voltage_;
    IdxVector idx_bus_injection_power_;
    IdxVector idx_branch_from_power_;
    IdxVector idx_branch_to_power_;
    IdxVector idx_shunt_power_;
    // relevant for extra value
    IdxVector idx_load_gen_power_;
    IdxVector idx_source_power_;
    // index for partial injection per bus, if available
    // 	for bus with full measurement, this is -1
    IdxVector idx_partial_injection_;
    // number of angle measurement
    Idx n_angle_;
    // average angle shift of voltages with angle measurement
    // default is zero is no voltage has angle measurement
    RealValue<sym> mean_angle_shift_;
    // scaling factor as minimum variance for the normalization
    double min_var_;

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
        connected to one component. The extra_value of all load_gen and source, connected to the bus, are added and
        appended to injection_measurement. If all connected load_gen and source contain measurements
        injection_measurement is appended to main_value_. If one or more connected load_gen or source is not
        measured(-1) the injection_measurement is appended to partial_injection_. NOTE: if all load_gen and source are
        not connected. It is a zero injection constraint, which is considered as a measurement in the main_value_ with
        zero variance.

        The voltage values in main_value_ can be found using idx_voltage.
        The power values in main_value_ can be found using idx_bus_injection_power_ (for combined load_gen and source)
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
                    idx_voltage_[bus] = -1;  // not measured
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
                // injection measurement is only available if all the connected load_gen/source are measured.
                // zero injection (all disconnected) is also considered as measured
                Idx n_unmeasured = 0;
                SensorCalcParam<sym> injection_measurement{};

                for (Idx load_gen = topo.load_gen_bus_indptr[bus]; load_gen != topo.load_gen_bus_indptr[bus + 1];
                     ++load_gen) {
                    if (idx_load_gen_power_[load_gen] == -1) {
                        ++n_unmeasured;
                        continue;
                    }
                    else if (idx_load_gen_power_[load_gen] == -2) {
                        continue;
                    }
                    injection_measurement.value += extra_value_[idx_load_gen_power_[load_gen]].value;
                    injection_measurement.variance += extra_value_[idx_load_gen_power_[load_gen]].variance;
                }

                for (Idx source = topo.source_bus_indptr[bus]; source != topo.source_bus_indptr[bus + 1]; ++source) {
                    if (idx_source_power_[source] == -1) {
                        ++n_unmeasured;
                        continue;
                    }
                    else if (idx_source_power_[source] == -2) {
                        continue;
                    }
                    injection_measurement.value += extra_value_[idx_source_power_[source]].value;
                    injection_measurement.variance += extra_value_[idx_source_power_[source]].variance;
                }

                if (n_unmeasured == 0) {
                    idx_bus_injection_power_[bus] = (Idx)main_value_.size();
                    main_value_.push_back(injection_measurement);
                }
                else {
                    idx_bus_injection_power_[bus] =
                        -n_unmeasured;  // not measured, bus-appliance exactly- or under-determined
                    // push to partial injection
                    idx_partial_injection_[bus] = (Idx)partial_injection_.size();
                    partial_injection_.push_back(injection_measurement);
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
        The branch_bus_idx contains the from and to bus indexes of the branch, or -1 if the branch is not connected at
        that side. For each branch the checker checks if the from and to side are connected by checking if
        branch_bus_idx = -1.

        If the branch_bus_idx = -1 (not connected),  idx_branch_to_power_/idx_branch_from_power_ is set to -2.
        If the side is connected, but there are no measurements in this branch side
        idx_branch_to_power_/idx_branch_from_power_ is set to -1. Else, idx_branch_to_power_/idx_branch_from_power_ is
        set to the index of the aggregated data in main_value_

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
            process_one_object(branch, topo.branch_from_power_sensor_indptr, topo.branch_bus_idx,
                               input.measured_branch_from_power, main_value_, idx_branch_from_power_,
                               branch_from_checker);
            // to side
            process_one_object(branch, topo.branch_to_power_sensor_indptr, topo.branch_bus_idx,
                               input.measured_branch_to_power, main_value_, idx_branch_to_power_, branch_to_checker);
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
            process_one_object(obj, sensor_indptr, obj_status, input_data, result_data, result_idx);
        }
    }

    // process one object
    static constexpr auto default_status_checker = [](auto x) -> bool {
        return x;
    };
    template <class TS, class StatusChecker = decltype(default_status_checker)>
    static void process_one_object(Idx const obj, IdxVector const& sensor_indptr, std::vector<TS> const& obj_status,
                                   std::vector<SensorCalcParam<sym>> const& input_data,
                                   std::vector<SensorCalcParam<sym>>& result_data, IdxVector& result_idx,
                                   StatusChecker status_checker = default_status_checker) {
        Idx const begin = sensor_indptr[obj];
        Idx const end = sensor_indptr[obj + 1];
        if (!status_checker(obj_status[obj])) {
            result_idx[obj] = -2;  // not connected
        }
        else if (begin == end) {
            result_idx[obj] = -1;  // not measured
        }
        else {
            result_idx[obj] = (Idx)result_data.size();
            result_data.push_back(combine_measurements(input_data, begin, end));
        }
    }

    // normalize the variance in the main value
    // pick the smallest variance (except zero, which is a constraint)
    // scale the smallest variance to one
    // in the gain matrix, the biggest weighting factor is then one
    void normalize_variance() {
        // loop to find min_var
        min_var_ = std::numeric_limits<double>::infinity();
        for (SensorCalcParam<sym> const& x : main_value_) {
            // only non-zero variance is considered
            if (x.variance != 0.0) {
                min_var_ = std::min(min_var_, x.variance);
            }
        }
        // scale
        std::for_each(main_value_.begin(), main_value_.end(), [this](SensorCalcParam<sym>& x) {
            x.variance /= min_var_;
        });
    }

    void calculate_non_over_determined_injection(Idx n_unmeasured, Idx load_gen_begin, Idx load_gen_end,
                                                 Idx source_begin, Idx source_end,
                                                 SensorCalcParam<sym> const& partial_injection,
                                                 ComplexValue<sym> const& s, LoadGenSourceFlow& pair) const {
        // calculate residual, divide, and assign to unmeasured (but connected) appliances
        ComplexValue<sym> const s_residual_per_appliance = (s - partial_injection.value) / (double)n_unmeasured;
        for (Idx load_gen = load_gen_begin; load_gen != load_gen_end; ++load_gen) {
            if (has_load_gen(load_gen)) {
                pair.first[load_gen].s = load_gen_power(load_gen).value;
            }
            else if (idx_load_gen_power_[load_gen] == -1) {
                pair.first[load_gen].s = s_residual_per_appliance;
            }
        }
        for (Idx source = source_begin; source != source_end; ++source) {
            if (has_source(source)) {
                pair.second[source].s = source_power(source).value;
            }
            else if (idx_source_power_[source] == -1) {
                pair.second[source].s = s_residual_per_appliance;
            }
        }
    }

    void calculate_over_determined_injection(Idx load_gen_begin, Idx load_gen_end, Idx source_begin, Idx source_end,
                                             SensorCalcParam<sym> const& full_injection, ComplexValue<sym> const& s,
                                             LoadGenSourceFlow& pair) const {
        // residual normalized by variance
        // mu = (sum[S_i] - S_cal) / sum[variance]
        ComplexValue<sym> const mu = (full_injection.value - s) / full_injection.variance;
        // S_i = S_i_mea - var_i * mu
        for (Idx load_gen = load_gen_begin; load_gen != load_gen_end; ++load_gen) {
            if (has_load_gen(load_gen)) {
                pair.first[load_gen].s = load_gen_power(load_gen).value -
                                         // also scale the variance here using the same normalization
                                         (load_gen_power(load_gen).variance / min_var_) * mu;
            }
        }
        for (Idx source = source_begin; source != source_end; ++source) {
            if (has_source(source)) {
                pair.second[source].s = source_power(source).value -
                                        // also scale the variance here using the same normalization
                                        (source_power(source).variance / min_var_) * mu;
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
          data_gain_(y_bus.nnz()),
          x_(y_bus.size()),
          rhs_(y_bus.size()),
          bsr_solver_{y_bus.size(), bsr_block_size_, y_bus.shared_indptr(), y_bus.shared_indices()} {
    }

    MathOutput<sym> run_state_estimation(YBus<sym> const& y_bus, StateEstimationInput<sym> const& input, double err_tol,
                                         Idx max_iter, CalculationInfo& calculation_info) {
        // prepare
        Timer main_timer, sub_timer;
        MathOutput<sym> output;
        output.u.resize(n_bus_);
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
        while (max_dev > err_tol) {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            sub_timer = Timer(calculation_info, 2224, "Calculate rhs");
            prepare_rhs(y_bus, measured_values, output.u);
            // solve with prefactorization
            sub_timer = Timer(calculation_info, 2225, "Solve sparse linear equation (pre-factorized)");
            bsr_solver_.solve(data_gain_.data(), rhs_.data(), x_.data(), true);
            sub_timer = Timer(calculation_info, 2226, "Iterate unknown");
            max_dev = iterate_unknown(output.u, measured_values.has_angle_measurement());
        }

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
    std::vector<SEUnknown<sym>> x_;
    std::vector<SERhs<sym>> rhs_;
    // solver
    BSRSolver<DoubleComplex> bsr_solver_;

    void prepare_matrix(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value) {
        MathModelParam<sym> const& param = y_bus.math_model_param();

        // loop data index, all rows and columns
        for (Idx data_idx = 0; data_idx != y_bus.nnz(); ++data_idx) {
            Idx const row = y_bus.row_indices()[data_idx];
            Idx const col = y_bus.col_indices()[data_idx];
            // get a reference and reset block to zero
            SEGainBlock<sym>& block = data_gain_[data_idx];
            block = {};
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
                            block.g() += dot(hermitian_transpose(param.branch_param[obj].value[measured_side * 2 + b0]),
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
                    block.r() = ComplexTensor<sym>{-measured_value.bus_injection_power(row).variance};
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

        // loop all transpose entry for QH
        // assign the hermitian transpose of the transpose entry of Q
        for (Idx data_idx = 0; data_idx != y_bus.nnz(); ++data_idx) {
            Idx const data_idx_tranpose = y_bus.transpose_entry()[data_idx];
            data_gain_[data_idx].qh() = hermitian_transpose(data_gain_[data_idx_tranpose].q());
        }
        // prefactorize
        bsr_solver_.prefactorize(data_gain_.data());
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
            SERhs<sym>& rhs_block = rhs_[bus];
            rhs_block = {};
            // fill block with voltage measurement
            if (measured_value.has_voltage(bus)) {
                // eta += u / variance
                rhs_block.eta += u[bus] / measured_value.voltage_var(bus);
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
                        rhs_block.eta -=
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
                            rhs_block.eta +=
                                dot(hermitian_transpose(param.branch_param[obj].value[measured_side * 2 + b]),
                                    conj(m.value / u[measured_bus])) /
                                m.variance;
                        }
                    }
                }
            }
            // fill block with injection measurement, need to convert to current
            if (measured_value.has_bus_injection(bus)) {
                rhs_block.tau = conj(measured_value.bus_injection_power(bus).value / u[bus]);
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
                return cabs(x_[math_topo_->slack_bus_].u) / x_[math_topo_->slack_bus_].u;
            }
            else {
                return cabs(x_[math_topo_->slack_bus_].u(0)) / x_[math_topo_->slack_bus_].u(0);
            }
        }();

        for (Idx bus = 0; bus != n_bus_; ++bus) {
            // phase offset to calculated voltage as normalized
            ComplexValue<sym> u_normalized = x_[bus].u * angle_offset;
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
        output.branch = y_bus.calculate_branch_flow(output.u);
        output.shunt = y_bus.calculate_shunt_flow(output.u);
        ComplexValueVector<sym> const s_injection = y_bus.calculate_injection(output.u);
        std::tie(output.load_gen, output.source) = measured_value.calculate_load_gen_source(output.u, s_injection);
    }
};

template class IterativeLinearSESolver<true>;
template class IterativeLinearSESolver<false>;

}  // namespace math_model_impl

template <bool sym>
using IterativeLinearSESolver = math_model_impl::IterativeLinearSESolver<sym>;

}  // namespace power_grid_model

#endif
