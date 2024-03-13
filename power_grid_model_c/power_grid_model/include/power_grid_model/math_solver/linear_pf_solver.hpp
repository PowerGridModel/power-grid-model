// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

/*
Linear PF solver for constant impedance

Model load as constant impedance/element_admittance
I_inj =  -U * Y_load
S_inj = U * conj(I_inj) = - U * conj(U) * conj(Y_load) = -V^2 * conj(Y_load)
S_base = -conj(Y_load)
Y_load = -conj(S_base)
YBus_diag += Y_load
YBus_diag += -conj(S_base)

Linear equation
[YBus] [U] = [rhs] = [I]

if no source
    rhs_i = 0
if there are sources
    rhs_i = I_i = sum{j as source} (- Y_source_j * U_i + Y_source_j * U_ref_j)
    reform equation
    YBus_diag_i += sum{j as source} (Y_source_j)
    rhs_i +=  sum{j as source} (Y_source_j * U_ref_j)

*/

#include "common_solver_functions.hpp"
#include "sparse_lu_solver.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"
#include "../common/timer.hpp"

namespace power_grid_model::math_solver {

namespace linear_pf {

template <symmetry_tag sym> class LinearPFSolver {

  public:
    LinearPFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          load_gens_per_bus_{topo_ptr, &topo_ptr->load_gens_per_bus},
          sources_per_bus_{topo_ptr, &topo_ptr->sources_per_bus},
          mat_data_(y_bus.nnz_lu()),
          sparse_solver_{y_bus.shared_indptr_lu(), y_bus.shared_indices_lu(), y_bus.shared_diag_lu()},
          perm_(n_bus_) {}

    MathOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                   CalculationInfo& calculation_info) {
        // output
        MathOutput<sym> output;
        output.u.resize(n_bus_);

        Timer const main_timer(calculation_info, 2220, "Math solver");

        // prepare matrix
        Timer sub_timer(calculation_info, 2221, "Prepare matrix");
        detail::copy_y_bus<sym>(y_bus, mat_data_);
        prepare_matrix_and_rhs(y_bus, input, output);

        // solve
        // u vector will have I_injection for slack bus for now
        sub_timer = Timer(calculation_info, 2222, "Solve sparse linear equation");
        sparse_solver_.prefactorize_and_solve(mat_data_, perm_, output.u, output.u);

        // calculate math result
        sub_timer = Timer(calculation_info, 2223, "Calculate math result");
        calculate_result(y_bus, input, output);

        // output
        return output;
    }

  private:
    Idx n_bus_;
    // shared topo data
    std::shared_ptr<SparseGroupedIdxVector const> load_gens_per_bus_;
    std::shared_ptr<DenseGroupedIdxVector const> sources_per_bus_;
    // sparse linear equation
    ComplexTensorVector<sym> mat_data_;
    // sparse solver
    SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>> sparse_solver_;
    typename SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>>::BlockPermArray perm_;

    void prepare_matrix_and_rhs(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, MathOutput<sym>& output) {
        using detail::add_sources;

        IdxVector const& bus_entry = y_bus.lu_diag();
        for (auto const& [bus_number, load_gens, sources] :
             enumerated_zip_sequence(*load_gens_per_bus_, *sources_per_bus_)) {
            Idx const diagonal_position = bus_entry[bus_number];
            auto& diagonal_element = mat_data_[diagonal_position];
            auto& u_bus = output.u[bus_number];
            add_loads(load_gens, bus_number, input, diagonal_element);
            add_sources(sources, bus_number, y_bus, input.source, diagonal_element, u_bus);
        }
    }

    static void add_loads(boost::iterator_range<IdxCount> const& load_gens_per_bus, Idx /* bus_number */,
                          PowerFlowInput<sym> const& input, ComplexTensor<sym>& diagonal_element) {
        for (auto load_number : load_gens_per_bus) {
            // YBus_diag += -conj(S_base)
            add_diag(diagonal_element, -conj(input.s_injection[load_number]));
        }
    }

    void calculate_result(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, MathOutput<sym>& output) {
        detail::calculate_pf_result(y_bus, input, *sources_per_bus_, *load_gens_per_bus_, output,
                                    [](Idx /*i*/) { return LoadGenType::const_y; });
    }
};

template class LinearPFSolver<symmetric_t>;
template class LinearPFSolver<asymmetric_t>;
} // namespace linear_pf

using linear_pf::LinearPFSolver;

} // namespace power_grid_model::math_solver
