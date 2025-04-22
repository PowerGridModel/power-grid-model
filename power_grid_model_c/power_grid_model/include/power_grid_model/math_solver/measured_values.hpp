// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

/*
Collect all measured Values
*/

#include "../calculation_parameters.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"

#include <memory>

namespace power_grid_model::math_solver {
// processed measurement struct
// combined all measurement of the same quantity
// accumulate for bus injection measurement
template <symmetry_tag sym> class MeasuredValues {
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
    MeasuredValues(std::shared_ptr<MathModelTopology const> topo, StateEstimationInput<sym> const& input)
        : math_topology_{std::move(topo)},
          bus_appliance_injection_(math_topology().n_bus()),
          idx_voltage_(math_topology().n_bus()),
          bus_injection_(math_topology().n_bus()),
          idx_branch_from_power_(math_topology().n_branch()),
          idx_branch_to_power_(math_topology().n_branch()),
          idx_shunt_power_(math_topology().n_shunt()),
          idx_load_gen_power_(math_topology().n_load_gen()),
          idx_source_power_(math_topology().n_source()),
          idx_branch_from_current_(math_topology().n_branch()),
          idx_branch_to_current_(math_topology().n_branch()),
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

    constexpr bool has_angle() const { return n_voltage_angle_measurements_ > 0; }
    constexpr bool has_voltage_measurements() const { return n_voltage_measurements_ > 0; }
    constexpr bool has_global_angle_current() const { return n_global_angle_current_measurements_ > 0; }

    constexpr bool has_voltage(Idx bus) const { return idx_voltage_[bus] >= 0; }
    constexpr bool has_angle_measurement(Idx bus) const { return !is_nan(imag(voltage(bus))); }
    constexpr bool has_bus_injection(Idx bus) const { return bus_injection_[bus].idx_bus_injection >= 0; }
    constexpr bool has_branch_from_power(Idx branch) const { return idx_branch_from_power_[branch] >= 0; }
    constexpr bool has_branch_to_power(Idx branch) const { return idx_branch_to_power_[branch] >= 0; }
    constexpr bool has_branch_from_current(Idx branch) const { return idx_branch_from_current_[branch] >= 0; }
    constexpr bool has_branch_to_current(Idx branch) const { return idx_branch_to_current_[branch] >= 0; }
    constexpr bool has_shunt(Idx shunt) const { return idx_shunt_power_[shunt] >= 0; }
    constexpr bool has_load_gen(Idx load_gen) const { return idx_load_gen_power_[load_gen] >= 0; }
    constexpr bool has_source(Idx source) const { return idx_source_power_[source] >= 0; }

    // getter of measurement and variance
    // if the obj is not measured, it is undefined behaviour to call this function
    // use checker first

    constexpr double voltage_var(Idx bus) const { return voltage_main_value_[idx_voltage_[bus]].variance; }
    constexpr auto const& voltage(Idx bus) const { return voltage_main_value_[idx_voltage_[bus]].value; }
    constexpr auto const& bus_injection(Idx bus) const {
        return power_main_value_[bus_injection_[bus].idx_bus_injection];
    }
    constexpr auto const& branch_from_power(Idx branch) const {
        return power_main_value_[idx_branch_from_power_[branch]];
    }
    constexpr auto const& branch_to_power(Idx branch) const { return power_main_value_[idx_branch_to_power_[branch]]; }
    constexpr auto const& branch_from_current(Idx branch) const {
        return current_main_value_[idx_branch_from_current_[branch]];
    }
    constexpr auto const& branch_to_current(Idx branch) const {
        return current_main_value_[idx_branch_to_current_[branch]];
    }
    constexpr auto const& shunt_power(Idx shunt) const { return power_main_value_[idx_shunt_power_[shunt]]; }
    constexpr auto const& load_gen_power(Idx load_gen) const { return extra_value_[idx_load_gen_power_[load_gen]]; }
    constexpr auto const& source_power(Idx source) const { return extra_value_[idx_source_power_[source]]; }

    constexpr auto first_voltage_measurement() const {
        assert(has_voltage_measurements());
        return first_voltage_measurement_;
    }

    // getter mean angle shift
    RealValue<sym> mean_angle_shift() const { return mean_angle_shift_; }

    // calculate load_gen and source flow
    // with given bus voltage and bus current injection
    using FlowVector = std::vector<ApplianceSolverOutput<sym>>;
    using LoadGenSourceFlow = std::pair<FlowVector, FlowVector>;

    LoadGenSourceFlow calculate_load_gen_source(ComplexValueVector<sym> const& u,
                                                ComplexValueVector<sym> const& s) const {
        std::vector<ApplianceSolverOutput<sym>> load_gen_flow(math_topology_->n_load_gen());
        std::vector<ApplianceSolverOutput<sym>> source_flow(math_topology_->n_source());

        // loop all buses
        for (auto const& [bus, load_gens, sources] :
             enumerated_zip_sequence(math_topology_->load_gens_per_bus, math_topology_->sources_per_bus)) {
            // under-determined or exactly determined
            if (bus_injection_[bus].n_unmeasured_appliances > 0) {
                calculate_non_over_determined_injection(bus_injection_[bus].n_unmeasured_appliances, load_gens, sources,
                                                        bus_appliance_injection_[bus], s[bus], load_gen_flow,
                                                        source_flow);
            }
            // over-determined
            else {
                calculate_over_determined_injection(load_gens, sources, bus_appliance_injection_[bus], s[bus],
                                                    load_gen_flow, source_flow);
            }
            // current injection
            for (Idx const load_gen : load_gens) {
                load_gen_flow[load_gen].i = conj(load_gen_flow[load_gen].s / u[bus]);
            }
            for (Idx const source : sources) {
                source_flow[source].i = conj(source_flow[source].s / u[bus]);
            }
        }

        return std::make_pair(load_gen_flow, source_flow);
    }

    // Construct linearized voltage value for all buses
    // for no measurement, the voltage phasor of the current iteration is used
    // for magnitude only measurement, the angle of the current iteration is used
    // for magnitude and angle measurement, the measured phasor is used
    ComplexValueVector<sym>
    combine_voltage_iteration_with_measurements(ComplexValueVector<sym> const& current_u) const {
        ComplexValueVector<sym> u(current_u.size());

        auto const new_u = [this, &current_u](Idx bus) -> ComplexValue<sym> {
            auto const& current_u_bus = current_u[bus];

            if (!has_voltage(bus)) { // no measurement
                return current_u_bus;
            }

            auto const& u_measured = voltage(bus);
            if (has_angle_measurement(bus)) { // full measurement
                return u_measured;
            }

            // U / |U| to get angle shift
            return real(u_measured) * phase_shift(current_u_bus);
        };

        for (Idx bus = 0; bus != static_cast<Idx>(current_u.size()); ++bus) {
            u[bus] = new_u(bus);
        }
        return u;
    }

  private:
    // cache topology
    std::shared_ptr<MathModelTopology const> math_topology_;

    // flat arrays of all the relevant measurement for the main calculation
    // branch/shunt flow, bus voltage, injection flow
    std::vector<VoltageSensorCalcParam<sym>> voltage_main_value_;
    std::vector<PowerSensorCalcParam<sym>> power_main_value_;
    std::vector<CurrentSensorCalcParam<sym>> current_main_value_;

    // flat array of all the load_gen/source measurement
    // not relevant for the main calculation, as extra data for load_gen/source calculation
    std::vector<PowerSensorCalcParam<sym>> extra_value_;
    // array of total appliance injection measurement per bus, regardless of the bus has all applianced measured or not
    std::vector<PowerSensorCalcParam<sym>> bus_appliance_injection_;

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
    // current measurement
    IdxVector idx_branch_from_current_;
    IdxVector idx_branch_to_current_;

    Idx n_voltage_measurements_{};
    Idx n_voltage_angle_measurements_{};
    Idx n_global_angle_current_measurements_{};

    // average angle shift of voltages with angle measurement
    // default is zero is no voltage has angle measurement
    RealValue<sym> mean_angle_shift_;
    // the lowest bus index with a voltage measurement
    Idx first_voltage_measurement_{};

    constexpr MathModelTopology const& math_topology() const { return *math_topology_; }

    void process_bus_related_measurements(StateEstimationInput<sym> const& input) {
        /*
        The main purpose of this function is to aggregate all voltage and power/current sensor values to
            one voltage sensor value per bus.
            one injection power sensor value per bus.
            one power sensor value per shunt (in injection reference direction, note shunt itself is not considered as
        injection element).


        This function loops through all buses
        For each bus all voltage sensor measurements are combined in a weighted average, which is appended to
        voltage_main_value_. For each bus, for all connected components, all power sensor measurements (per component
        (shunt, load_gen, source)) are combined in a weighted average, which is appended to power_main_value_ (for
        shunt) or extra_value_ (for load_gen and source). E.g. a value in extra_value contains the weighted average of
        all sensors connected to one component. The extra_value_ of all load_gen and source, connected to the bus, are
        added and appended to appliace_injection_measurement.

        We combine all the available load_gen and source measurements into appliance_injection_measurement by summing
        them up, and store it in bus_appliance_injection_. If all the connected load_gen and source are measured, we
        further combine the appliance_injection_measurement into the (if available) direct bus injection measurement,
        and put it into power_main_value_.

        NOTE: if all load_gen and source are not connected (disconnected). It is a zero injection constraint,
        which is considered as a measurement in the main_value_ with zero variance.

        The voltage values in voltage_main_value_ can be found using idx_voltage.
        The power values in power_main_value_ can be found using bus_injection_ (for combined load_gen and source)
        and idx_shunt_power_ (for shunt).
        */
        process_voltage_measurements(input);
        process_appliance_measurements(input);
    }

    void process_voltage_measurements(StateEstimationInput<sym> const& input) {
        MathModelTopology const& topo = math_topology();

        RealValue<sym> angle_cum{};
        for (auto const& [bus, sensors] : enumerated_zip_sequence(topo.voltage_sensors_per_bus)) {
            angle_cum += process_bus_voltage_measurements(bus, sensors, input);
        }

        // assign a meaningful mean angle shift, if at least one voltage has angle measurement
        if (has_angle()) {
            mean_angle_shift_ = angle_cum / RealValue<sym>{static_cast<double>(n_voltage_angle_measurements_)};
        }

        static constexpr auto const is_measured = [](auto const& value) { return value >= 0; };
        n_voltage_measurements_ = std::ranges::count_if(idx_voltage_, is_measured);
        first_voltage_measurement_ =
            std::distance(idx_voltage_.begin(), std::ranges::find_if(idx_voltage_, is_measured));
    }

    RealValue<sym> process_bus_voltage_measurements(Idx bus, IdxRange const& sensors,
                                                    StateEstimationInput<sym> const& input) {
        RealValue<sym> angle_cum{};

        VoltageSensorCalcParam<sym> aggregated{ComplexValue<sym>{0.0}, std::numeric_limits<double>::infinity()};
        bool angle_measured{false};

        // check if there is nan
        if (auto const start = input.measured_voltage.cbegin() + *sensors.begin();
            std::any_of(start, start + sensors.size(), [](auto const& x) { return is_nan(imag(x.value)); })) {
            // only keep magnitude
            aggregated = combine_measurements<true>(input.measured_voltage, sensors);
        } else {
            // keep complex number
            aggregated = combine_measurements(input.measured_voltage, sensors);
            angle_measured = true;
        }

        if (is_inf(aggregated.variance)) {
            idx_voltage_[bus] = unmeasured;
        } else {
            idx_voltage_[bus] = static_cast<Idx>(voltage_main_value_.size());
            voltage_main_value_.push_back(aggregated);
            if (angle_measured) {
                ++n_voltage_angle_measurements_;
                // accumulate angle, offset by intrinsic phase shift
                angle_cum = arg(aggregated.value * std::exp(-1.0i * math_topology().phase_shift[bus]));
            }
        }
        return angle_cum;
    }

    void process_appliance_measurements(StateEstimationInput<sym> const& input) {
        MathModelTopology const& topo = math_topology();

        for (auto const& [bus, shunts, load_gens, sources] :
             enumerated_zip_sequence(topo.shunts_per_bus, topo.load_gens_per_bus, topo.sources_per_bus)) {
            process_bus_objects(shunts, topo.power_sensors_per_shunt, input.shunt_status, input.measured_shunt_power,
                                power_main_value_, idx_shunt_power_);
            process_bus_objects(load_gens, topo.power_sensors_per_load_gen, input.load_gen_status,
                                input.measured_load_gen_power, extra_value_, idx_load_gen_power_);
            process_bus_objects(sources, topo.power_sensors_per_source, input.source_status,
                                input.measured_source_power, extra_value_, idx_source_power_);

            combine_appliances_to_injection_measurements(input, topo, bus);
        }
    }

    void combine_appliances_to_injection_measurements(StateEstimationInput<sym> const& input,
                                                      MathModelTopology const& topo, Idx const bus) {
        Idx n_unmeasured = 0;
        PowerSensorCalcParam<sym> appliance_injection_measurement{};

        for (Idx const load_gen : topo.load_gens_per_bus.get_element_range(bus)) {
            add_appliance_measurements(idx_load_gen_power_[load_gen], appliance_injection_measurement, n_unmeasured);
        }

        for (Idx const source : topo.sources_per_bus.get_element_range(bus)) {
            add_appliance_measurements(idx_source_power_[source], appliance_injection_measurement, n_unmeasured);
        }

        bus_appliance_injection_[bus] = appliance_injection_measurement;
        bus_injection_[bus].n_unmeasured_appliances = n_unmeasured;

        // get direct bus injection measurement. It has infinite variance if there is no direct bus injection
        // measurement
        PowerSensorCalcParam<sym> const direct_injection_measurement =
            combine_measurements(input.measured_bus_injection, topo.power_sensors_per_bus.get_element_range(bus));

        // combine valid appliance_injection_measurement and direct_injection_measurement
        // three scenarios; check if we have valid injection measurement
        auto const uncertain_direct_injection = is_inf(direct_injection_measurement.real_component.variance) ||
                                                is_inf(direct_injection_measurement.imag_component.variance);

        bus_injection_[bus].idx_bus_injection = static_cast<Idx>(power_main_value_.size());
        if (n_unmeasured > 0) {
            if (uncertain_direct_injection) {
                bus_injection_[bus].idx_bus_injection = unmeasured;
            } else {
                // only direct injection
                power_main_value_.push_back(direct_injection_measurement);
            }
        } else if (uncertain_direct_injection || any_zero(appliance_injection_measurement.real_component.variance) ||
                   any_zero(appliance_injection_measurement.imag_component.variance)) {
            // only appliance injection if
            //    there is no direct injection measurement,
            //    or we have zero injection
            power_main_value_.push_back(appliance_injection_measurement);
        } else {
            // both valid, we combine again
            power_main_value_.push_back(
                combine_measurements(std::vector{direct_injection_measurement, appliance_injection_measurement}));
        }
    }

    // if all the connected load_gen/source are measured, their sum can be considered as an injection
    // measurement. zero injection (no connected appliances) is also considered as measured
    // invalid measurements (infinite sigma) are considered unmeasured
    void add_appliance_measurements(Idx const appliance_idx, PowerSensorCalcParam<sym>& measurements,
                                    Idx& n_unmeasured) {
        if (appliance_idx == unmeasured) {
            ++n_unmeasured;
            return;
        }
        if (appliance_idx == disconnected) {
            return;
        }

        auto const& appliance_measurement = extra_value_[appliance_idx];
        if (is_inf(appliance_measurement.real_component.variance) ||
            is_inf(appliance_measurement.imag_component.variance)) {
            ++n_unmeasured;
            return;
        }
        measurements.real_component.value += appliance_measurement.real_component.value;
        measurements.imag_component.value += appliance_measurement.imag_component.value;
        measurements.real_component.variance += appliance_measurement.real_component.variance;
        measurements.imag_component.variance += appliance_measurement.imag_component.variance;
    }

    void process_branch_measurements(StateEstimationInput<sym> const& input) {
        /*
        The main purpose of this function is to aggregate all power/current sensor values to one power/current sensor
        value per branch side.

        This function loops through all branches.
        The branch_bus_idx contains the from and to bus indexes of the branch, or disconnected if the branch is not
        connected at that side. For each branch the checker checks if the from and to side are connected by checking if
        branch_bus_idx = disconnected.

        If the branch_bus_idx = disconnected, idx_branch_(to/from)_(power/current)_ is set to disconnected.
        If the side is connected, but there are no measurements in this branch side
        idx_branch_(to/from)_(power/current)_ is set to disconnected.
        Else, idx_branch_(to/from)_(power/current)_ is set to the index of the aggregated data in
        power/current_main_value_.

        All measurement values for a single side of a branch are combined in a weighted average, which is appended to
        power/current_main_value_. The values in power/current_main_value_ can be found using
        idx_branch_(to/from)_(power/current)_.
        */
        MathModelTopology const& topo = math_topology();
        static constexpr auto branch_from_checker = [](BranchIdx x) { return x[0] != -1; };
        static constexpr auto branch_to_checker = [](BranchIdx x) { return x[1] != -1; };
        for (Idx const branch : IdxRange{topo.n_branch()}) {
            // from side power
            idx_branch_from_power_[branch] =
                process_one_object(branch, topo.power_sensors_per_branch_from, topo.branch_bus_idx,
                                   input.measured_branch_from_power, power_main_value_, branch_from_checker);
            // to side power
            idx_branch_to_power_[branch] =
                process_one_object(branch, topo.power_sensors_per_branch_to, topo.branch_bus_idx,
                                   input.measured_branch_to_power, power_main_value_, branch_to_checker);
            // from side current
            idx_branch_from_current_[branch] =
                process_one_object(branch, topo.current_sensors_per_branch_from, topo.branch_bus_idx,
                                   input.measured_branch_from_current, current_main_value_, branch_from_checker);
            // to side current
            idx_branch_to_current_[branch] =
                process_one_object(branch, topo.current_sensors_per_branch_to, topo.branch_bus_idx,
                                   input.measured_branch_to_current, current_main_value_, branch_to_checker);

            n_global_angle_current_measurements_ =
                std::ranges::count_if(current_main_value_, [](auto const& measurement) {
                    return measurement.angle_measurement_type == AngleMeasurementType::global_angle;
                });
        }
    }

    // combine multiple measurements of one quantity
    // using Kalman filter
    // if only_magnitude = true, combine the abs value of the individual data
    //      set imag part to nan, to signal this is a magnitude only measurement
    template <bool only_magnitude = false>
    static VoltageSensorCalcParam<sym> combine_measurements(std::vector<VoltageSensorCalcParam<sym>> const& data,
                                                            IdxRange const& sensors) {
        auto complex_measurements = sensors | std::views::transform([&data](Idx pos) -> auto& { return data[pos]; });
        if constexpr (only_magnitude) {
            return statistics::combine_magnitude(complex_measurements);
        } else {
            return statistics::combine(complex_measurements);
        }
    }
    template <bool only_magnitude = false>
        requires(!only_magnitude)
    static PowerSensorCalcParam<sym> combine_measurements(std::vector<PowerSensorCalcParam<sym>> const& data,
                                                          IdxRange const& sensors) {
        return statistics::combine(sensors | std::views::transform([&data](Idx pos) -> auto& { return data[pos]; }));
    }
    template <bool only_magnitude = false>
        requires(!only_magnitude)
    static CurrentSensorCalcParam<sym> combine_measurements(std::vector<CurrentSensorCalcParam<sym>> const& data,
                                                            IdxRange const& sensors) {
        auto const params = sensors | std::views::transform([&data](Idx pos) -> auto& { return data[pos]; });
        auto const angle_measurement_type = sensors.empty() ? AngleMeasurementType::local_angle // fallback
                                                            : params.front().angle_measurement_type;
        if (std::ranges::any_of(params, [angle_measurement_type](auto const& param) {
                return param.angle_measurement_type != angle_measurement_type;
            })) {
            throw ConflictingAngleMeasurementType{
                "Cannot mix local and global angle current measurements on the same terminal."};
        }

        return {.angle_measurement_type = angle_measurement_type,
                .measurement = statistics::combine(
                    params | std::views::transform([](auto const& param) -> auto& { return param.measurement; }))};
    }

    template <sensor_calc_param_type CalcParam, bool only_magnitude = false>
    static auto combine_measurements(std::vector<CalcParam> const& data) {
        return combine_measurements<only_magnitude>(data, IdxRange{static_cast<Idx>(data.size())});
    }

    // process objects in batch for shunt, load_gen, source
    // return the status of the object type, if all the connected objects are measured
    static void process_bus_objects(IdxRange const& objects, grouped_idx_vector_type auto const& sensors_per_object,
                                    IntSVector const& object_status,
                                    std::vector<PowerSensorCalcParam<sym>> const& input_data,
                                    std::vector<PowerSensorCalcParam<sym>>& result_data, IdxVector& result_idx) {
        for (Idx const object : objects) {
            result_idx[object] = process_one_object(object, sensors_per_object, object_status, input_data, result_data);
        }
    }

    // process one object
    struct DefaultStatusChecker {
        template <class T> bool operator()(T x) const { return x; }
    };

    static constexpr DefaultStatusChecker default_status_checker{};

    template <class TS, sensor_calc_param_type CalcParam, class StatusChecker = DefaultStatusChecker>
    static Idx process_one_object(Idx const object, grouped_idx_vector_type auto const& sensors_per_object,
                                  std::vector<TS> const& object_status, std::vector<CalcParam> const& input_data,
                                  std::vector<CalcParam>& result_data,
                                  StatusChecker status_checker = default_status_checker) {
        if (!status_checker(object_status[object])) {
            return disconnected;
        }
        auto const sensors = sensors_per_object.get_element_range(object);
        if (std::empty(sensors)) {
            return unmeasured;
        }
        result_data.push_back(combine_measurements(input_data, sensors));
        return static_cast<Idx>(result_data.size()) - 1;
    }

    // normalize the variance in the main values
    // pick the smallest variance (except zero, which is a constraint)
    // p and q variances are combined (see also https://en.wikipedia.org/wiki/Complex_random_variable)
    // scale the smallest variance
    // to one in the gain matrix, the biggest weighting factor is then one
    void normalize_variance() {
        double min_var = std::numeric_limits<double>::infinity();
        auto const unconstrained_min = [&min_var](double v) {
            // only non-zero variance is considered
            if (v != 0.0) {
                min_var = std::min(min_var, v);
            }
        };
        for (auto const& x : voltage_main_value_) {
            unconstrained_min(x.variance);
        }
        for (auto const& x : power_main_value_) {
            auto const variance = x.real_component.variance + x.imag_component.variance;
            if constexpr (is_symmetric_v<sym>) {
                unconstrained_min(variance);
            } else {
                for (Idx const phase : {0, 1, 2}) {
                    unconstrained_min(variance[phase]);
                }
            }
        }
        for (auto const& x : current_main_value_) {
            auto const variance = x.measurement.real_component.variance + x.measurement.imag_component.variance;
            if constexpr (is_symmetric_v<sym>) {
                unconstrained_min(variance);
            } else {
                for (Idx const phase : {0, 1, 2}) {
                    unconstrained_min(variance[phase]);
                }
            }
        }

        // scale
        auto const inv_norm_var = 1.0 / min_var;
        std::ranges::for_each(voltage_main_value_, [inv_norm_var](auto& x) { x.variance *= inv_norm_var; });
        std::ranges::for_each(power_main_value_, [inv_norm_var](auto& x) {
            x.real_component.variance *= inv_norm_var;
            x.imag_component.variance *= inv_norm_var;
        });
        std::ranges::for_each(current_main_value_, [inv_norm_var](auto& x) {
            x.measurement.real_component.variance *= inv_norm_var;
            x.measurement.imag_component.variance *= inv_norm_var;
        });
    }

    void calculate_non_over_determined_injection(Idx n_unmeasured, IdxRange const& load_gens, IdxRange const& sources,
                                                 PowerSensorCalcParam<sym> const& bus_appliance_injection,
                                                 ComplexValue<sym> const& s, FlowVector& load_gen_flow,
                                                 FlowVector& source_flow) const {
        // calculate residual, divide, and assign to unmeasured (but connected) appliances
        ComplexValue<sym> const s_residual_per_appliance =
            (s - bus_appliance_injection.value()) / static_cast<double>(n_unmeasured);
        for (Idx const load_gen : load_gens) {
            if (has_load_gen(load_gen)) {
                load_gen_flow[load_gen].s = load_gen_power(load_gen).value();
            } else if (idx_load_gen_power_[load_gen] == unmeasured) {
                load_gen_flow[load_gen].s = s_residual_per_appliance;
            }
        }
        for (Idx const source : sources) {
            if (has_source(source)) {
                source_flow[source].s = source_power(source).value();
            } else if (idx_source_power_[source] == unmeasured) {
                source_flow[source].s = s_residual_per_appliance;
            }
        }
    }

    void calculate_over_determined_injection(IdxRange const& load_gens, IdxRange const& sources,
                                             PowerSensorCalcParam<sym> const& bus_appliance_injection,
                                             ComplexValue<sym> const& s, FlowVector& load_gen_flow,
                                             FlowVector& source_flow) const {
        // residual normalized by variance
        // mu = (sum[S_i] - S_cal) / sum[variance]
        auto const delta = ComplexValue<sym>{bus_appliance_injection.value() - s};
        ComplexValue<sym> const mu = real(delta) / bus_appliance_injection.real_component.variance +
                                     1.0i * imag(delta) / bus_appliance_injection.imag_component.variance;

        // S_i = S_i_mea - var_i * mu
        auto const calculate_injection = [&mu](auto const& power) {
            return ComplexValue<sym>{power.value() - ((power.real_component.variance * real(mu)) +
                                                      (1.0i * power.imag_component.variance * imag(mu)))};
        };

        for (Idx const load_gen : load_gens) {
            if (has_load_gen(load_gen)) {
                load_gen_flow[load_gen].s = calculate_injection(load_gen_power(load_gen));
            }
        }
        for (Idx const source : sources) {
            if (has_source(source)) {
                source_flow[source].s = calculate_injection(source_power(source));
            }
        }
    }
};

template class MeasuredValues<symmetric_t>;
template class MeasuredValues<asymmetric_t>;
} // namespace power_grid_model::math_solver
