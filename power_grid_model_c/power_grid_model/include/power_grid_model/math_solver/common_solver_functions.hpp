// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/exception.hpp"
#include <numeric>

namespace power_grid_model::math_solver::detail {

template <symmetry_tag sym>
inline void add_sources(IdxRange const& sources, Idx /* bus_number */, YBus<sym> const& y_bus,
                        ComplexVector const& u_source_vector, ComplexTensor<sym>& diagonal_element,
                        ComplexValue<sym>& u_bus) {
    for (Idx const source_number : sources) {
        ComplexTensor<sym> const y_source = y_bus.math_model_param().source_param[source_number];
        diagonal_element += y_source; // add y_source to the diagonal of Ybus
        u_bus += dot(y_source, ComplexValue<sym>{u_source_vector[source_number]}); // rhs += Y_source * U_source
    }
}

template <symmetry_tag sym>
inline void add_linear_loads(boost::iterator_range<IdxCount> const& load_gens_per_bus, Idx /* bus_number */,
                             PowerFlowInput<sym> const& input, ComplexTensor<sym>& diagonal_element) {
    for (auto load_number : load_gens_per_bus) {
        // YBus_diag += -conj(S_base)
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

template <symmetry_tag sym>
inline void accumulate_y_ref(std::vector<ComplexTensor<sym>>& y_ref_acc, ComplexTensor<sym>& y_ref_t,
                             IdxRange const& sources, YBus<sym> const& y_bus) {
    std::ranges::transform(sources, y_ref_acc.begin(), [&](Idx const source) -> ComplexTensor<sym> {
        return y_bus.math_model_param().source_param[source];
    });
    y_ref_t = std::accumulate(y_ref_acc.begin(), y_ref_acc.end(), ComplexTensor<sym>{});
}

template <symmetry_tag sym>
inline void accumulate_i_ref(std::vector<ComplexValue<sym>>& i_ref_acc, ComplexValue<sym>& i_ref_t,
                             IdxRange const& sources, YBus<sym> const& y_bus, PowerFlowInput<sym> const& input) {
    std::ranges::transform(sources, i_ref_acc.begin(), [&](Idx const source) -> ComplexValue<sym> {
        ComplexValue<sym> const u_ref{input.source[source]};
        ComplexTensor<sym> const y_ref = y_bus.math_model_param().source_param[source];
        return dot(y_ref, u_ref);
    });
    i_ref_t = std::accumulate(i_ref_acc.begin(), i_ref_acc.end(), ComplexValue<sym>{});
}

inline void accumulate_y0_y1(std::vector<DoubleComplex>& y_ref_0_acc, std::vector<DoubleComplex>& y_ref_1_acc,
                             DoubleComplex& y_ref_0_t, DoubleComplex& y_ref_1_t, IdxRange const& sources,
                             YBus<asymmetric_t> const& y_bus, PowerFlowInput<asymmetric_t> const& input) {
    std::ranges::transform(sources, y_ref_0_acc.begin(), [&](Idx const source) -> DoubleComplex {
        return y_bus.math_model_param().source_param_y0_y1[source].first;
    });
    std::ranges::transform(sources, y_ref_1_acc.begin(), [&](Idx const source) -> DoubleComplex {
        return y_bus.math_model_param().source_param_y0_y1[source].second;
    });
    y_ref_0_t = std::accumulate(y_ref_0_acc.begin(), y_ref_0_acc.end(), DoubleComplex{});
    y_ref_1_t = std::accumulate(y_ref_1_acc.begin(), y_ref_1_acc.end(), DoubleComplex{});
}

// calculates current and power result for a single source (sym or asym)
template <symmetry_tag sym>
inline void calculate_single_source_result(SolverOutput<sym>& output, Idx const& source_id, Idx const& bus_number,
                                           ComplexValue<sym> const& i_inj_t) {
    output.source[source_id].i = i_inj_t;
    output.source[source_id].s = output.u[bus_number] * conj(output.source[source_id].i);
}

// calculates current and power source result for multiple symmetric sources
inline void calculate_multiple_source_result(IdxRange const& sources, YBus<symmetric_t> const& y_bus,
                                             PowerFlowInput<symmetric_t> const& input,
                                             std::vector<Idx> const& sources_acc,
                                             ComplexValue<symmetric_t> const& i_inj_t,
                                             SolverOutput<symmetric_t>& output, Idx const& bus_number) {
    std::vector<ComplexTensor<symmetric_t>> y_ref_acc(sources.size());
    ComplexTensor<symmetric_t> y_ref_t;
    accumulate_y_ref<symmetric_t>(y_ref_acc, y_ref_t, sources, y_bus);
    std::vector<ComplexValue<symmetric_t>> i_ref_acc(sources.size());
    ComplexValue<symmetric_t> i_ref_t;
    accumulate_i_ref<symmetric_t>(i_ref_acc, i_ref_t, sources, y_bus, input);
    ComplexTensor<symmetric_t> z_ref_t = inv(y_ref_t);

    for (size_t i = 0; i < sources.size(); ++i) {
        Idx const source = sources_acc[i];
        ComplexTensor<symmetric_t> y_ref_i_z_ref_t = y_ref_acc[i] / y_ref_t;
        auto aux = dot(y_ref_i_z_ref_t, i_ref_t);
        ComplexValue<symmetric_t> i_inj_lhs = i_ref_acc[i] - dot(y_ref_i_z_ref_t, i_ref_t);
        ComplexValue<symmetric_t> i_inj_rhs = dot(y_ref_i_z_ref_t, i_inj_t);
        output.source[source].i = i_inj_lhs + i_inj_rhs;
        output.source[source].s = output.u[bus_number] * conj(output.source[source].i);
    }
}

// calculates current and power source result for multiple asymmetric sources
inline void calculate_multiple_source_result(IdxRange const& sources, YBus<asymmetric_t> const& y_bus,
                                             PowerFlowInput<asymmetric_t> const& input,
                                             std::vector<Idx> const& sources_acc,
                                             ComplexValue<asymmetric_t> const& i_inj_t,
                                             SolverOutput<asymmetric_t>& output, Idx const& bus_number) {
    DoubleComplex i_inj_t_0 = (i_inj_t(0) + i_inj_t(1) + i_inj_t(2)) / 3.0;
    DoubleComplex i_inj_t_1 = (i_inj_t(0) + a * i_inj_t(1) + a2 * i_inj_t(2)) / 3.0;
    DoubleComplex i_inj_t_2 = (i_inj_t(0) + a2 * i_inj_t(1) + a * i_inj_t(2)) / 3.0;

    std::vector<DoubleComplex> y_ref_0_acc(sources.size());
    std::vector<DoubleComplex> y_ref_1_acc(sources.size());
    DoubleComplex y_ref_0_t;
    DoubleComplex y_ref_1_t;
    accumulate_y0_y1(y_ref_0_acc, y_ref_1_acc, y_ref_0_t, y_ref_1_t, sources, y_bus, input);
    std::vector<ComplexValue<asymmetric_t>> i_ref_acc(sources.size());
    ComplexValue<asymmetric_t> i_ref_t;
    accumulate_i_ref<asymmetric_t>(i_ref_acc, i_ref_t, sources, y_bus, input);

    for (size_t i = 0; i < sources.size(); ++i) {
        Idx const source = sources_acc[i];
        DoubleComplex const i_ref_1_i = i_ref_acc[i](0) + a * i_ref_acc[i](1) + a2 * i_ref_acc[i](2);
        DoubleComplex const i_ref_1_t = i_ref_t(0) + a * i_ref_t(1) + a2 * i_ref_t(2);
        DoubleComplex y_ref_0_over_y_ref_0_t = (y_ref_0_acc[i] / y_ref_0_t);
        DoubleComplex const i_inj_0 = (y_ref_0_acc[i] / y_ref_0_t) * i_inj_t_0;
        DoubleComplex i_inj_1_rhs = (y_ref_1_acc[i] / y_ref_1_t) * i_inj_t_1;
        DoubleComplex i_inj_1_lhs = i_ref_1_i - ((y_ref_1_acc[i] / y_ref_1_t) * i_ref_1_t);
        DoubleComplex const i_inj_1 = i_inj_1_lhs - i_inj_1_rhs;
        DoubleComplex const i_inj_2 = (y_ref_1_acc[i] / y_ref_1_t) * i_inj_t_2;
        DoubleComplex i_inj_a = i_inj_0 + i_inj_1 + i_inj_2;
        DoubleComplex i_inj_b = i_inj_0 + a2 * i_inj_1 + a * i_inj_2;
        DoubleComplex i_inj_c = i_inj_0 + a * i_inj_1 + a2 * i_inj_2;
        ComplexValue<asymmetric_t> i_inj_abc{i_inj_a, i_inj_b, i_inj_c};
        output.source[source].i = i_inj_abc;
        output.source[source].s = output.u[bus_number] * conj(output.source[source].i);
    }
}

template <symmetry_tag sym>
inline void calculate_source_result(IdxRange const& sources, Idx const& bus_number, YBus<sym> const& y_bus,
                                    PowerFlowInput<sym> const& input, SolverOutput<sym>& output,
                                    ComplexValue<sym> const& i_load_gen_bus) {
    ComplexValue<sym> const i_inj_t = conj(output.bus_injection[bus_number] / output.u[bus_number]) - i_load_gen_bus;
    std::vector<Idx> sources_acc(sources.size());
    std::ranges::transform(sources, sources_acc.begin(), [&](Idx const source) { return source; });

    if (sources_acc.size() == 1) {
        calculate_single_source_result(output, sources_acc[0], bus_number, i_inj_t);
    } else if (!sources_acc.empty()) {
        calculate_multiple_source_result(sources, y_bus, input, sources_acc, i_inj_t, output, bus_number);
    }
}

template <symmetry_tag sym, class LoadGenFunc>
    requires std::invocable<std::remove_cvref_t<LoadGenFunc>, Idx> &&
             std::same_as<std::invoke_result_t<LoadGenFunc, Idx>, LoadGenType>
inline void calculate_load_gen_result(IdxRange const& load_gens, Idx bus_number, PowerFlowInput<sym> const& input,
                                      SolverOutput<sym>& output, LoadGenFunc&& load_gen_func,
                                      ComplexValue<sym>& i_load_gen_bus) {
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
        i_load_gen_bus += output.load_gen[load_gen].i;
    }
}

template <symmetry_tag sym, typename LoadGenFunc>
    requires std::invocable<std::remove_cvref_t<LoadGenFunc>, Idx> &&
             std::same_as<std::invoke_result_t<LoadGenFunc, Idx>, LoadGenType>
inline void calculate_pf_result(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                grouped_idx_vector_type auto const& sources_per_bus,
                                grouped_idx_vector_type auto const& load_gens_per_bus, SolverOutput<sym>& output,
                                LoadGenFunc&& load_gen_func) {
    assert(sources_per_bus.size() == load_gens_per_bus.size());

    // call y bus
    output.branch = y_bus.template calculate_branch_flow<BranchSolverOutput<sym>>(output.u);
    output.shunt = y_bus.template calculate_shunt_flow<ApplianceSolverOutput<sym>>(output.u);

    // prepare source, load gen and node injection
    output.source.resize(sources_per_bus.element_size());
    output.load_gen.resize(load_gens_per_bus.element_size());
    output.bus_injection.resize(sources_per_bus.size());

    output.bus_injection = y_bus.calculate_injection(output.u);
    for (auto const& [bus_number, sources, load_gens] : enumerated_zip_sequence(sources_per_bus, load_gens_per_bus)) {
        ComplexValue<sym> i_load_gen_bus{};
        calculate_load_gen_result<sym>(load_gens, bus_number, input, output, load_gen_func, i_load_gen_bus);
        calculate_source_result<sym>(sources, bus_number, y_bus, input, output, i_load_gen_bus);
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
