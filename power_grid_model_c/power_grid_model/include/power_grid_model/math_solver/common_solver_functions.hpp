// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/exception.hpp"

#include <map>

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
    std::transform(y_bus.map_lu_y_bus().cbegin(), y_bus.map_lu_y_bus().cend(), mat_data.begin(), [&](Idx k) {
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

template <symmetry_tag sym>
inline void calculate_voltage_regulator_result(Idx const& bus_number, PowerFlowInput<sym> const& input, SolverOutput<sym>& output,
                                               IdxRange const& load_gens, std::map<Idx, Idx> const& loadgen_to_regulator) {
    if (load_gens.empty()) {
        return;
    }

    ComplexValue<sym> const s_load_gen_bus =
        std::transform_reduce(load_gens.begin(), load_gens.end(), ComplexValue<sym>{}, std::plus{},
                            [&](Idx const load_gen) { return output.load_gen[load_gen].s; });

    ComplexValue<sym> const s_remaining = output.bus_injection[bus_number] - s_load_gen_bus;

    auto regulating_gens = load_gens
        | std::views::filter([&](Idx lg) {
            return loadgen_to_regulator.contains(lg);
        })
        | std::views::filter([&](Idx lg) {
            auto const regulator = loadgen_to_regulator.at(lg);
            return input.load_gen_status[lg] != 0 &&
                input.voltage_regulator[regulator].status != 0;
        });
    double num_regulating_gens = 0.0;
    for ([[maybe_unused]] auto const& _ : regulating_gens) {
        ++num_regulating_gens;
    }
    if (num_regulating_gens == 0.0) {
        return;
    }

    auto const& q_remaining = imag(s_remaining);
    for (Idx const load_gen : load_gens) {
        if (!loadgen_to_regulator.contains(load_gen)) {
            continue;
        }
        auto const regulator = loadgen_to_regulator.at(load_gen);

        auto const& input_regulator = input.voltage_regulator[regulator];
        auto& output_regulator = output.voltage_regulator[regulator];
        output_regulator.generator_id = input_regulator.generator_id;
        output_regulator.generator_status = input.load_gen_status[load_gen];
        output_regulator.limit_violated = 0;
        output_regulator.q = RealValue<sym>{0};

        if(input.load_gen_status[load_gen] == 0 || input.voltage_regulator[regulator].status == 0) {
            continue;
        }

        // TODO(scud-soptim): #185 consider Q Limits (PV to PQ conversion)
        // TODO(scud-soptim): #185 equal distribution for now, later consider proportional distribution based on Q limits
        if constexpr (is_symmetric_v<sym>) {
            auto const q_regulator = q_remaining / num_regulating_gens;
            output_regulator.q = q_regulator;
            output_regulator.limit_violated = 0; // no violation

            auto const& s_load_gen = output.load_gen[load_gen].s;
            output.load_gen[load_gen].s = ComplexValue<sym>{real(s_load_gen), imag(s_load_gen) + q_regulator};
        } else {
            output.voltage_regulator[regulator].q = RealValue<asymmetric_t>{
                q_remaining[0] / num_regulating_gens,
                q_remaining[1] / num_regulating_gens,
                q_remaining[2] / num_regulating_gens
            };
            output.voltage_regulator[regulator].limit_violated = 0; // no violation

            auto const& s_load_gen = output.load_gen[load_gen].s;
            auto const& p_load_gen = real(s_load_gen);
            auto const& q_load_gen = imag(s_load_gen);
            output.load_gen[load_gen].s = ComplexValue<asymmetric_t>{
                {p_load_gen[0], q_load_gen[0] + q_remaining[0] / num_regulating_gens},
                {p_load_gen[1], q_load_gen[1] + q_remaining[1] / num_regulating_gens},
                {p_load_gen[2], q_load_gen[2] + q_remaining[2] / num_regulating_gens}
            };
        }
        output.load_gen[load_gen].i = conj(output.load_gen[load_gen].s / output.u[bus_number]);
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
