// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/counting_iterator.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"
#include "../common/grouped_index_vector.hpp"
#include "../common/three_phase_tensor.hpp"

#include <Eigen/Core>

#include <algorithm>
#include <cassert>
#include <complex>
#include <concepts>
#include <functional>
#include <map>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <vector>

namespace power_grid_model::math_solver::detail {

template <symmetry_tag sym>
inline void add_sources(IdxRange const& sources, Idx /* bus_number */, YBus<sym> const& y_bus,
                        ComplexVector const& u_source_vector, ComplexTensor<sym>& diagonal_element,
                        ComplexValue<sym>& u_bus) {
    for (Idx const source_number : sources) {
        ComplexTensor<sym> const y_source = y_bus.math_model_param().source_param[source_number].template y_ref<sym>();
        diagonal_element += y_source; // add y_source to the diagonal of Ybus
        u_bus += dot(y_source, ComplexValue<sym>{u_source_vector[source_number]}); // rhs += Y_source * U_source
    }
}

template <symmetry_tag sym>
inline void add_linear_loads(IdxRange const& load_gens_per_bus, Idx /* bus_number */, PowerFlowInput<sym> const& input,
                             ComplexTensor<sym>& diagonal_element) {
    for (auto load_number : load_gens_per_bus) {
        // YBus_diag += -conj(S_base) // NOSONAR(S125)
        add_diag(diagonal_element, -conj(input.s_injection[load_number]));
    }
}

template <symmetry_tag sym>
inline void prepare_linear_matrix_and_rhs(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                          grouped_idx_vector_type auto const& load_gens_per_bus,
                                          grouped_idx_vector_type auto const& sources_per_bus,
                                          SolverOutput<sym>& output, ComplexTensorVector<sym>& mat_data) {
    IdxVector const& bus_entry = y_bus.lu_diag();
    for (auto const& [bus_number, load_gens, sources] : enumerated_zip_sequence(load_gens_per_bus, sources_per_bus)) {
        Idx const diagonal_position = bus_entry[bus_number];
        auto& diagonal_element = mat_data[diagonal_position];
        auto& u_bus = output.u[bus_number];
        add_linear_loads(load_gens, bus_number, input, diagonal_element);
        add_sources(sources, bus_number, y_bus, input.source, diagonal_element, u_bus);
    }
}

template <symmetry_tag sym> inline void copy_y_bus(YBus<sym> const& y_bus, ComplexTensorVector<sym>& mat_data) {
    ComplexTensorVector<sym> const& ydata = y_bus.admittance();
    std::ranges::transform(y_bus.map_lu_y_bus(), mat_data.begin(), [&](Idx k) {
        if (k == -1) {
            return ComplexTensor<sym>{};
        }
        return ydata[k];
    });
}

/// @brief Calculates current and power injection of source i for multiple symmetric sources at a node.
/// The current injection of source i to the bus in phase space is:
///     i_inj_i = (y_ref_i * z_ref_t) [ (u_ref_i * y_ref_t - i_ref_t) + i_inj_t]
/// The equation is organized in such a way to avoid the numerical instability induced in the substraction if y_ref_i
/// is large.
inline void calculate_multiple_source_result(IdxRange const& sources, YBus<symmetric_t> const& y_bus,
                                             PowerFlowInput<symmetric_t> const& input,
                                             ComplexValue<symmetric_t> const& i_inj_t,
                                             SolverOutput<symmetric_t>& output, Idx const& bus_number) {
    std::vector<SourceCalcParam> const y_ref = y_bus.math_model_param().source_param;
    DoubleComplex const y_ref_t = std::transform_reduce(sources.begin(), sources.end(), DoubleComplex{}, std::plus{},
                                                        [&](Idx const source) { return y_ref[source].y1; });

    auto const z_ref_t = 1.0 / y_ref_t;
    DoubleComplex const i_ref_t =
        std::transform_reduce(sources.begin(), sources.end(), DoubleComplex{}, std::plus{},
                              [&](Idx const source) { return input.source[source] * y_ref[source].y1; });
    for (Idx const source : sources) {
        DoubleComplex const y_ref_i_over_y_ref_t = y_ref[source].y1 * z_ref_t;
        DoubleComplex const i_inj_i_lhs = y_ref_i_over_y_ref_t * (input.source[source] * y_ref_t - i_ref_t);
        output.source[source].i = i_inj_i_lhs + (y_ref_i_over_y_ref_t * i_inj_t);
        output.source[source].s = output.u[bus_number] * conj(output.source[source].i);
    }
}

/// @brief Calculates current and power injection for source i for multiple asymmetric sources at a node.
/// The current injection of source i to the bus in 012 domain is:
/// i_inj_0 = y_ref_i_0 / y_ref_t_0 * i_inj_t_0
/// i_inj_1 = y_ref_i_1 * [(u_ref_i_1 - i_ref_t_1 / y_ref_t_1) + i_inj_t_1 / y_ref_t_1]
/// i_inj_2 = y_ref_i_2 / y_ref_t_2 * i_inj_t_2
/// Note: u_ref_i_0 = u_ref_i_2 = 0
/// 012 domain is used to solve the numerical instability as for the symmetric case.
inline void calculate_multiple_source_result(IdxRange const& sources, YBus<asymmetric_t> const& y_bus,
                                             PowerFlowInput<asymmetric_t> const& input,
                                             ComplexValue<asymmetric_t> const& i_inj_t,
                                             SolverOutput<asymmetric_t>& output, Idx const& bus_number) {
    std::vector<SourceCalcParam> const y_ref_012 = y_bus.math_model_param().source_param;
    ComplexValue<asymmetric_t> const y_ref_t_012 = std::transform_reduce(
        sources.begin(), sources.end(), ComplexValue<asymmetric_t>{}, std::plus{}, [&](Idx const source) {
            ComplexValue<asymmetric_t> y_012;
            y_012(0) = y_ref_012[source].y0;
            y_012(1) = y_ref_012[source].y1;
            y_012(2) = y_012(1); // y1 = y2
            return y_012;
        });
    DoubleComplex const i_ref_1_t =
        std::transform_reduce(sources.begin(), sources.end(), DoubleComplex{}, std::plus{},
                              [&](Idx const source) { return input.source[source] * y_ref_012[source].y1; });
    ComplexValue<asymmetric_t> const i_inj_t_012 = dot(get_sym_matrix_inv(), i_inj_t);

    for (Idx const source : sources) {
        DoubleComplex const y_ref_i_1_over_y_ref_t_1 = y_ref_012[source].y1 / y_ref_t_012[1];
        DoubleComplex const i_inj_i_1_lhs =
            y_ref_i_1_over_y_ref_t_1 * (input.source[source] * y_ref_t_012[1] - i_ref_1_t);
        DoubleComplex const i_inj_i_0 = (y_ref_012[source].y0 / y_ref_t_012[0]) * i_inj_t_012(0);
        DoubleComplex const i_inj_i_1 = i_inj_i_1_lhs + (y_ref_i_1_over_y_ref_t_1 * i_inj_t_012(1));
        DoubleComplex const i_inj_i_2 = (y_ref_012[source].y1 / y_ref_t_012[2]) * i_inj_t_012(2);
        ComplexValue<asymmetric_t> const i_inj_012{i_inj_i_0, i_inj_i_1, i_inj_i_2};
        output.source[source].i = dot(get_sym_matrix(), i_inj_012);
        output.source[source].s = output.u[bus_number] * conj(output.source[source].i);
    }
}

// Current implementation avoids numerical instability. For details refer to:
// https://github.com/PowerGridModel/power-grid-model-workshop/blob/experiment/source-calculation/source_calculation/source_calculation.ipynb
template <symmetry_tag sym>
inline void calculate_source_result(IdxRange const& sources, Idx const& bus_number, YBus<sym> const& y_bus,
                                    PowerFlowInput<sym> const& input, SolverOutput<sym>& output,
                                    IdxRange const& load_gens) {
    if (sources.empty()) {
        return;
    }
    ComplexValue<sym> const i_load_gen_bus =
        std::transform_reduce(load_gens.begin(), load_gens.end(), ComplexValue<sym>{}, std::plus{},
                              [&](Idx const load_gen) { return output.load_gen[load_gen].i; });
    ComplexValue<sym> const i_inj_t = conj(output.bus_injection[bus_number] / output.u[bus_number]) - i_load_gen_bus;
    if (sources.size() == 1) {
        output.source[*sources.begin()].i = i_inj_t;
        output.source[*sources.begin()].s = output.u[bus_number] * conj(output.source[*sources.begin()].i);
    } else {
        calculate_multiple_source_result(sources, y_bus, input, i_inj_t, output, bus_number);
    }
}

template <symmetry_tag sym> struct RegulatorQState {
    Idx regulator_idx;
    Idx load_gen_idx;
    bool is_regulating{false};
    bool has_available_capacity{false};
    RealValue<sym> q_allocated{0.0};
};

template <symmetry_tag sym>
inline auto initialize_regulator_output_and_distribution(
    PowerFlowInput<sym> const& input, SolverOutput<sym>& output, IdxRange const& load_gens,
    std::map<Idx, Idx> const& loadgen_to_regulator,
    std::vector<RegulatorQState<sym>>& regulator_state) -> std::tuple<int, ComplexValue<sym>> {

    int num_regulating_gens = 0;
    ComplexValue<sym> s_load_gen_bus = ComplexValue<sym>{0.0};

    for (Idx const load_gen : load_gens) {
        if (!loadgen_to_regulator.contains(load_gen)) {
            // sum up power for non-regulating load_gens
            s_load_gen_bus += output.load_gen[load_gen].s;
            continue;
        }

        auto const regulator = loadgen_to_regulator.at(load_gen);

        // initialize regulator output
        auto& output_regulator = output.voltage_regulator[regulator];
        output_regulator.generator_id = input.voltage_regulator[regulator].generator_id;
        output_regulator.generator_status = input.load_gen_status[load_gen];
        output_regulator.limit_violated = LimitViolation::none;

        auto const is_regulating =
            input.load_gen_status[load_gen] != status_off && input.voltage_regulator[regulator].status != status_off;

        if (!is_regulating) {
            // sum up power for non-regulating load_gens
            s_load_gen_bus += output.load_gen[load_gen].s;
            continue;
        }

        // collect regulator data for distribution of reactive power
        ++num_regulating_gens;
        regulator_state.emplace_back(RegulatorQState<sym>{
            .regulator_idx = regulator,
            .load_gen_idx = load_gen,
            .is_regulating = is_regulating,
            .has_available_capacity = false,
            .q_allocated = RealValue<sym>{0.0},
        });
    }

    return {num_regulating_gens, s_load_gen_bus};
}

template <symmetry_tag sym> inline double total_q(RealValue<sym> const& q) {
    if constexpr (is_symmetric_v<sym>) {
        return q;
    } else {
        return q(0) + q(1) + q(2); // sum phases
    }
}

template <symmetry_tag sym>
inline RealValue<sym> distribute_q(double q_scalar, [[maybe_unused]] RealValue<sym> const& base_distribution) {
    if constexpr (is_symmetric_v<sym>) {
        return q_scalar;
    } else {
        // preserve per-phase proportions, or divide equally if base distribution is near zero
        double const base_total = total_q<asymmetric_t>(base_distribution);
        if (std::abs(base_total) > numerical_tolerance) {
            double const scale = q_scalar / base_total;
            return RealValue<asymmetric_t>{base_distribution(0) * scale, base_distribution(1) * scale,
                                           base_distribution(2) * scale};
        }
        return RealValue<asymmetric_t>{q_scalar / 3.0, q_scalar / 3.0, q_scalar / 3.0};
    }
}

template <symmetry_tag sym>
inline void calculate_voltage_regulator_result(Idx const& bus_number, PowerFlowInput<sym> const& input,
                                               SolverOutput<sym>& output, IdxRange const& load_gens,
                                               std::map<Idx, Idx> const& loadgen_to_regulator) {
    if (load_gens.empty()) {
        return;
    }

    // 1. initialize output and collect regulator state for distribution of reactive power
    std::vector<RegulatorQState<sym>> regulator_state;
    auto const [num_regulating_gens, s_load_gen_bus] =
        initialize_regulator_output_and_distribution(input, output, load_gens, loadgen_to_regulator, regulator_state);

    if (num_regulating_gens == 0) {
        return;
    }

    // 2. determine distibution of reactive power under consideration of regulator limits
    ComplexValue<sym> const s_remaining = output.bus_injection[bus_number] - s_load_gen_bus;
    auto const q_remaining = imag(s_remaining);

    auto const bus_limit_violated = output.bus[bus_number].q_limit_violated;
    if (bus_limit_violated == LimitViolation::none) {
        // no bus limit violation, but but individual regulator limits must be considered
        allocate_q_iterative_distribution<sym>(num_regulating_gens, q_remaining, input, output, regulator_state);
    } else {
        // bus limit violation, and therefore violation of all inidividual regulator limits
        allocate_q_bus_limit_violated<sym>(bus_limit_violated, input, output, regulator_state);
    }

    // 3. apply distributed Q to load_gens
    for (auto& reg_state : regulator_state) {
        if (!reg_state.is_regulating) {
            continue;
        }
        auto const& s_load_gen = output.load_gen[reg_state.load_gen_idx].s;
        auto const p_load_gen = real(s_load_gen);
        auto& output_load_gen = output.load_gen[reg_state.load_gen_idx];
        if constexpr (is_symmetric_v<sym>) {
            output_load_gen.s = ComplexValue<symmetric_t>{p_load_gen, reg_state.q_allocated};
        } else {
            output_load_gen.s = ComplexValue<asymmetric_t>{{p_load_gen[0], reg_state.q_allocated[0]},
                                                           {p_load_gen[1], reg_state.q_allocated[1]},
                                                           {p_load_gen[2], reg_state.q_allocated[2]}};
        }
        output_load_gen.i = conj(output_load_gen.s / output.u[bus_number]);
    }
}

template <symmetry_tag sym>
inline void allocate_q_bus_limit_violated(LimitViolation bus_limit_violated, PowerFlowInput<sym> const& input,
                                          SolverOutput<sym>& output,
                                          std::vector<RegulatorQState<sym>>& regulator_state) {

    // bus limit is violated, therefore each individual regulator limit is violated as well
    for (auto& reg_state : regulator_state) {
        if (!reg_state.is_regulating) {
            continue;
        }

        auto const& input_regulator = input.voltage_regulator[reg_state.regulator_idx];
        auto& output_regulator = output.voltage_regulator[reg_state.regulator_idx];

        output_regulator.limit_violated = bus_limit_violated;

        double const limit_value =
            bus_limit_violated == LimitViolation::upper ? input_regulator.q_max : input_regulator.q_min;
        double const q_limit_scalar = is_nan(limit_value) ? 0.0 : limit_value;

        RealValue<sym> const base_q = imag(output.load_gen[reg_state.load_gen_idx].s);

        reg_state.q_allocated = distribute_q<sym>(q_limit_scalar, base_q);
        reg_state.has_available_capacity = false;
    }
}

template <symmetry_tag sym>
inline void allocate_q_iterative_distribution(int num_regulating_gens, RealValue<sym> q_remaining,
                                              PowerFlowInput<sym> const& input, SolverOutput<sym>& output,
                                              std::vector<RegulatorQState<sym>>& regulator_state) {

    // setup active load_gens with active regulators as having available capacity for allocation
    for (auto& reg_state : regulator_state) {
        reg_state.has_available_capacity = reg_state.is_regulating;
    }

    // Iterative equal distribution of Q with limit checking on the regulators with available capacity.
    // When a regulator hits its limit, it is removed from the active set and the unallocated Q is
    // redistributed among the remaining regulators. Unallocated Q remaining after all regulators hit
    // their limits should not occur, as then the bus would have been switched to a PQ bus.
    while (std::abs(total_q<sym>(q_remaining)) > numerical_tolerance && num_regulating_gens > 0) {
        RealValue<sym> const q_per_regulator = q_remaining / num_regulating_gens;
        RealValue<sym> q_unallocated{0.0};

        for (auto& reg_state : regulator_state) {
            if (!reg_state.has_available_capacity) {
                continue;
            }

            auto const& input_regulator = input.voltage_regulator[reg_state.regulator_idx];
            auto& output_regulator = output.voltage_regulator[reg_state.regulator_idx];

            RealValue<sym> const q_prev_allocated = reg_state.q_allocated;
            RealValue<sym> const q_next_allocated = q_prev_allocated + q_per_regulator;

            // check individual limits
            double const q_next_allocated_scalar = total_q<sym>(q_next_allocated);

            if (!is_nan(input_regulator.q_max) &&
                q_next_allocated_scalar > input_regulator.q_max + numerical_tolerance) {
                // hit upper limit
                reg_state.q_allocated = distribute_q<sym>(input_regulator.q_max, q_next_allocated);
                reg_state.has_available_capacity = false;
                q_unallocated += q_per_regulator - (reg_state.q_allocated - q_prev_allocated);
                num_regulating_gens -= 1;
                // TODO(frie-soptim): change to no violation if bus limit itself is not violated, simply set Q to limit
                //                    -> then limit_violated on the regulator strictly implies that correcponding bus
                //                    was switched to PQ
                output_regulator.limit_violated = LimitViolation::upper;
            } else if (!is_nan(input_regulator.q_min) &&
                       q_next_allocated_scalar < input_regulator.q_min - numerical_tolerance) {
                // hit lower limit
                reg_state.q_allocated = distribute_q<sym>(input_regulator.q_min, q_next_allocated);
                reg_state.has_available_capacity = false;
                q_unallocated += q_per_regulator - (reg_state.q_allocated - q_prev_allocated);
                num_regulating_gens -= 1;
                output_regulator.limit_violated = LimitViolation::lower;
            } else {
                // within limits
                reg_state.q_allocated = q_next_allocated;
                output_regulator.limit_violated = LimitViolation::none;
            }
        }

        if (std::abs(total_q<sym>(q_remaining - q_unallocated)) < numerical_tolerance) {
            // throw just in case, as this should only happen when all limits are hit,
            // and this case should have been handled in allocate_q_bus_limit_violated()
            throw CalculationError("Unallocated Q remains after distribution on");
        }

        q_remaining = q_unallocated;
    }
}

template <symmetry_tag sym, class LoadGenFunc>
    requires std::invocable<std::remove_cvref_t<LoadGenFunc>, Idx> &&
             std::same_as<std::invoke_result_t<LoadGenFunc, Idx>, LoadGenType>
inline void calculate_load_gen_result(IdxRange const& load_gens, Idx bus_number, PowerFlowInput<sym> const& input,
                                      SolverOutput<sym>& output, LoadGenFunc load_gen_func) {
    for (Idx const load_gen : load_gens) {
        switch (LoadGenType const type = load_gen_func(load_gen); type) {
            using enum LoadGenType;

        case const_pq:
            // always same power
            output.load_gen[load_gen].s = input.s_injection[load_gen];
            break;
        case const_y:
            // power is quadratic relation to voltage
            output.load_gen[load_gen].s = input.s_injection[load_gen] * abs2(output.u[bus_number]);
            break;
        case const_i:
            // power is linear relation to voltage
            output.load_gen[load_gen].s = input.s_injection[load_gen] * cabs(output.u[bus_number]);
            break;
        default:
            throw MissingCaseForEnumError("Power injection", type);
        }
        output.load_gen[load_gen].i = conj(output.load_gen[load_gen].s / output.u[bus_number]);
    }
}

template <symmetry_tag sym, typename LoadGenFunc>
    requires std::invocable<std::remove_cvref_t<LoadGenFunc>, Idx> &&
             std::same_as<std::invoke_result_t<LoadGenFunc, Idx>, LoadGenType>
inline void calculate_pf_result(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                grouped_idx_vector_type auto const& sources_per_bus,
                                grouped_idx_vector_type auto const& load_gens_per_bus, SolverOutput<sym>& output,
                                LoadGenFunc load_gen_func) {
    assert(sources_per_bus.size() == load_gens_per_bus.size());

    auto const& voltage_regulators_per_load_gen = y_bus.math_topology().voltage_regulators_per_load_gen;

    // call y bus
    output.branch = y_bus.template calculate_branch_flow<BranchSolverOutput<sym>>(output.u);
    output.shunt = y_bus.template calculate_shunt_flow<ApplianceSolverOutput<sym>>(output.u);

    // prepare source, load gen and node injection
    output.source.resize(sources_per_bus.element_size());
    output.load_gen.resize(load_gens_per_bus.element_size());
    output.voltage_regulator.resize(voltage_regulators_per_load_gen.element_size());
    output.bus_injection.resize(sources_per_bus.size());
    output.bus_injection = y_bus.calculate_injection(output.u);

    std::map<Idx, Idx> loadgen_to_regulator; // save mapping from generator to its regulator
    for (auto const& [load_gen, regulators] : enumerated_zip_sequence(voltage_regulators_per_load_gen)) {
        for (Idx const regulator : regulators) {
            loadgen_to_regulator.insert({load_gen, regulator});
        }
    }

    for (auto const& [bus_number, sources, load_gens] : enumerated_zip_sequence(sources_per_bus, load_gens_per_bus)) {
        calculate_load_gen_result<sym>(load_gens, bus_number, input, output,
                                       [&load_gen_func](Idx idx) { return load_gen_func(idx); });
        calculate_voltage_regulator_result<sym>(bus_number, input, output, load_gens, loadgen_to_regulator);
        calculate_source_result<sym>(sources, bus_number, y_bus, input, output, load_gens);
    }
}

template <symmetry_tag sym>
inline void calculate_se_result(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value,
                                SolverOutput<sym>& output) {
    // call y bus
    output.branch = y_bus.template calculate_branch_flow<BranchSolverOutput<sym>>(output.u);
    output.shunt = y_bus.template calculate_shunt_flow<ApplianceSolverOutput<sym>>(output.u);
    output.bus_injection = y_bus.calculate_injection(output.u);
    std::tie(output.load_gen, output.source) = measured_value.calculate_load_gen_source(output.u, output.bus_injection);
}

} // namespace power_grid_model::math_solver::detail
