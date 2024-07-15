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
inline void truncate_to_zero(double const& theta, ComplexValue<sym> const& val1, ComplexValue<sym> const& val2,
                             ComplexValue<sym>& result) {
    auto precision = theta * std::numeric_limits<double>::epsilon();
    if (max_val(cabs(result)) < (precision * ((max_val(cabs(val1)) + max_val(cabs(val2))) / 2.0))) {
        result = ComplexValue<sym>{0.0};
    }
}

template <symmetry_tag sym>
inline void calculate_source_result(IdxRange const& sources, Idx bus_number, YBus<sym> const& y_bus,
                                    PowerFlowInput<sym> const& input, SolverOutput<sym>& output,
                                    ComplexValue<sym> const& i_load_gen_bus) {
    ComplexValue<sym> const i_inj_t = conj(output.bus_injection[bus_number] / output.u[bus_number]) - i_load_gen_bus;
    std::vector<Idx> sources_acc(sources.size());
    std::ranges::transform(sources, sources_acc.begin(), [&](Idx const source) { return source; });
    if (sources_acc.size() == 1) {
        output.source[sources_acc[0]].i = i_inj_t;
        output.source[sources_acc[0]].s = output.u[bus_number] * conj(output.source[sources_acc[0]].i);
    } else if (!sources_acc.empty()) {
        std::vector<ComplexTensor<sym>> y_ref_acc(sources.size());
        std::vector<ComplexValue<sym>> i_ref_acc(sources.size());
        std::ranges::transform(sources, y_ref_acc.begin(), [&](Idx const source) -> ComplexTensor<sym> {
            return y_bus.math_model_param().source_param[source];
        });
        std::ranges::transform(sources, i_ref_acc.begin(), [&](Idx const source) -> ComplexValue<sym> {
            ComplexValue<sym> const u_ref{input.source[source]};
            ComplexTensor<sym> const y_ref = y_bus.math_model_param().source_param[source];
            return dot(y_ref, u_ref);
        });
        ComplexTensor<sym> const y_ref_t = std::accumulate(y_ref_acc.begin(), y_ref_acc.end(), ComplexTensor<sym>{});
        ComplexTensor<sym> z_ref_t = inv_y<sym>(y_ref_t); // s_E = 15E, m_Z = 6E truncation errors.
        ComplexValue<sym> const i_ref_t = std::accumulate(i_ref_acc.begin(), i_ref_acc.end(), ComplexValue<sym>{});
        for (size_t i = 0; i < sources.size(); ++i) {
            Idx const source = sources_acc[i];
            ComplexValue<sym> const u_ref_t = dot(z_ref_t, i_ref_t); // 54E truncation error.
            ComplexValue<sym> const u_ref_i{input.source[i]};
            ComplexValue<sym> delta_u = (u_ref_i - u_ref_t);
            // This truncation needs further discussion.
            constexpr double theta = 54.0; // From counting floating point operations involving E.
            truncate_to_zero<sym>(theta, u_ref_i, u_ref_t, delta_u);
            ComplexValue<sym> const i_inj_rhs = dot(dot(y_ref_acc[i], z_ref_t), i_inj_t);
            output.source[source].i = dot(y_ref_acc[i], delta_u) + i_inj_rhs;
            output.source[source].s = output.u[bus_number] * conj(output.source[source].i);
        }
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
