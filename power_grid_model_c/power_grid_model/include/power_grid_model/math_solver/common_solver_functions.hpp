// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/exception.hpp"

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
                                    IdxRange const& load_gens, IdxRange const& shunts) {
    if (sources.empty()) {
        return;
    }
    ComplexValue<sym> const i_load_gen_bus =
        std::transform_reduce(load_gens.begin(), load_gens.end(), ComplexValue<sym>{}, std::plus{},
                              [&](Idx const load_gen) { return output.load_gen[load_gen].i; });

    ComplexValue<sym> const i_shunt_bus =
        std::transform_reduce(shunts.begin(), shunts.end(), ComplexValue<sym>{}, std::plus{},
                              [&](Idx const shunt) { return output.shunt[shunt].i; });

    ComplexValue<sym> const i_inj_t = conj(output.bus_injection[bus_number] / output.u[bus_number]) - i_load_gen_bus - i_shunt_bus;
    if (sources.size() == 1) {
        output.source[*sources.begin()].i = i_inj_t;
        output.source[*sources.begin()].s = output.u[bus_number] * conj(output.source[*sources.begin()].i);
    } else {
        calculate_multiple_source_result(sources, y_bus, input, i_inj_t, output, bus_number);
    }
}

template <symmetry_tag sym, std::ranges::forward_range PVRange, std::ranges::forward_range PQRange>
    requires std::same_as<std::ranges::range_value_t<PQRange>, Idx> &&
             std::same_as<std::ranges::range_value_t<PVRange>, Idx>
inline void calculate_pv_gen_result(Idx const& bus_number, std::vector<BusType> const& bus_type,
                                    PowerFlowInput<sym> const& input, SolverOutput<sym>& output,
                                    PVRange&& pv_gens, PQRange&& pq_load_gens, IdxRange const& shunts) {
    if (bus_type[bus_number] == BusType::pq) {
        // TODO: #185 consider Q Limits (PV to PQ conversion)
        for (Idx const pv_gen : pv_gens) {
            output.load_gen[pv_gen].s = input.load_gen[pv_gen].s_injection;
            output.load_gen[pv_gen].i = conj(output.load_gen[pv_gen].s / output.u[bus_number]);
        }
    } else {
        // distribute Q to PV generators on PV or Slack bus
        if (pv_gens.empty()) {
            return;
        }

        ComplexValue<sym> const s_pq_gen_bus =
            std::transform_reduce(pq_load_gens.begin(), pq_load_gens.end(), ComplexValue<sym>{}, std::plus{},
                                [&](Idx const load_gen) { return output.load_gen[load_gen].s; });

        ComplexValue<sym> const s_shunt_bus =
            std::transform_reduce(shunts.begin(), shunts.end(), ComplexValue<sym>{}, std::plus{},
                                [&](Idx const shunt) { return output.shunt[shunt].s; });

        ComplexValue<sym> const s_remaining = output.bus_injection[bus_number] - s_pq_gen_bus - s_shunt_bus;

        size_t num_gens = std::ranges::distance(pv_gens);
        auto const& q_remaining = imag(s_remaining);
        for (Idx const pv_gen : pv_gens) {
            auto const& p_pv_gen = real(input.load_gen[pv_gen].s_injection);
            // TODO: #185 equal distribution for now, later consider proportional distribution based on Q limits
            if constexpr (is_symmetric_v<sym>) {
                output.load_gen[pv_gen].s = ComplexValue<sym>{p_pv_gen, q_remaining / num_gens};
            } else {
                output.load_gen[pv_gen].s = ComplexValue<asymmetric_t>{
                    {p_pv_gen[0], q_remaining[0] / num_gens},
                    {p_pv_gen[1], q_remaining[1] / num_gens},
                    {p_pv_gen[2], q_remaining[2] / num_gens}
                };
            }
            output.load_gen[pv_gen].i = conj(output.load_gen[pv_gen].s / output.u[bus_number]);
        }
    }
}

template <symmetry_tag sym, class LoadGenFunc, std::ranges::forward_range Range>
    requires std::invocable<std::remove_cvref_t<LoadGenFunc>, Idx> &&
             std::same_as<std::invoke_result_t<LoadGenFunc, Idx>, LoadGenType> &&
             std::same_as<std::ranges::range_value_t<Range>, Idx>
inline void calculate_load_gen_result(Range&& load_gens, Idx bus_number, PowerFlowInput<sym> const& input,
                                      SolverOutput<sym>& output, LoadGenFunc load_gen_func) {
    for (Idx const load_gen : load_gens) {
        switch (LoadGenType const type = load_gen_func(load_gen); type) {
            using enum LoadGenType;

        case const_pq:
            // always same power
            output.load_gen[load_gen].s = input.load_gen[load_gen].s_injection;
            break;
        case const_y:
            // power is quadratic relation to voltage
            output.load_gen[load_gen].s = input.load_gen[load_gen].s_injection * abs2(output.u[bus_number]);
            break;
        case const_i:
            // power is linear relation to voltage
            output.load_gen[load_gen].s = input.load_gen[load_gen].s_injection * cabs(output.u[bus_number]);
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
                                grouped_idx_vector_type auto const& load_gens_per_bus,
                                grouped_idx_vector_type auto const& shunts_per_bus,
                                SolverOutput<sym>& output,
                                LoadGenFunc load_gen_func) {
    assert(sources_per_bus.size() == load_gens_per_bus.size());

    std::vector<BusType> const& bus_type = y_bus.bus_type();

    // call y bus
    output.branch = y_bus.template calculate_branch_flow<BranchSolverOutput<sym>>(output.u);
    output.shunt = y_bus.template calculate_shunt_flow<ApplianceSolverOutput<sym>>(output.u);

    auto is_pv = [&](Idx load_gen) {
        return load_gen_func(load_gen) == LoadGenType::const_pv;
    };

    // prepare source, load gen and node injection
    output.source.resize(sources_per_bus.element_size());
    output.load_gen.resize(load_gens_per_bus.element_size());
    output.bus_injection.resize(sources_per_bus.size());
    output.bus_injection = y_bus.calculate_injection(output.u);
    for (auto const& [bus_number, sources, load_gens, shunts]
        : enumerated_zip_sequence(sources_per_bus, load_gens_per_bus, shunts_per_bus)) {

        auto pv_gens = load_gens | std::views::filter(is_pv);
        auto pq_load_gens = load_gens | std::views::filter(std::not_fn(is_pv));

        // handle only pq load gens here
        calculate_load_gen_result<sym>(pq_load_gens, bus_number, input, output,
                                       [&load_gen_func](Idx idx) { return load_gen_func(idx); });

        // handle pv gens separately - distribute Q
        calculate_pv_gen_result<sym>(bus_number, bus_type, input, output, pv_gens, pq_load_gens, shunts);

        // push remaining p and q to sources
        calculate_source_result<sym>(sources, bus_number, y_bus, input, output, load_gens, shunts);
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
