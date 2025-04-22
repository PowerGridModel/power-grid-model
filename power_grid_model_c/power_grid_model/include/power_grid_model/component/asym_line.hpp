// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <numeric>

#include "branch.hpp"
#include <iostream>

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/matrix_utils.hpp"
#include "../common/three_phase_tensor.hpp"
#include "line_utils.hpp"

namespace power_grid_model {

class AsymLine : public Branch {
  public:
    using InputType = AsymLineInput;
    using UpdateType = BranchUpdate;
    static constexpr char const* name = "asym_line";

    explicit AsymLine(AsymLineInput const& asym_line_input, double system_frequency, double u1, double u2)
        : Branch{asym_line_input}, i_n_{asym_line_input.i_n}, base_i_{base_power_3p / u1 / sqrt3} {
        if (cabs(u1 - u2) > numerical_tolerance) {
            throw ConflictVoltage{id(), from_node(), to_node(), u1, u2};
        }

        ComplexTensor<asymmetric_t> c_matrix = compute_c_matrix_from_input(asym_line_input);
        ComplexTensor<asymmetric_t> z_series = compute_z_series_from_input(asym_line_input);

        double const base_y = base_i_ / (u1 / sqrt3);

        y_series_abc_ = 1 / base_y * inv(z_series);
        y_shunt_abc_ = 1 / base_y * (2.0i * pi * system_frequency * c_matrix);
    }

    // override getter
    constexpr double base_i_from() const override { return base_i_; }
    constexpr double base_i_to() const override { return base_i_; }
    constexpr double loading(double /* max_s */, double max_i) const override { return max_i / i_n_; };
    constexpr double phase_shift() const override { return 0.0; }
    constexpr bool is_param_mutable() const override { return false; }

  private:
    double i_n_{};
    double base_i_{};
    ComplexTensor<asymmetric_t> y_series_abc_{};
    ComplexTensor<asymmetric_t> y_shunt_abc_{};

    ComplexTensor<asymmetric_t> compute_z_series_from_input(const power_grid_model::AsymLineInput& asym_line_input) {
        ComplexTensor<asymmetric_t> z_series_abc;
        if (is_nan(asym_line_input.r_na) && is_nan(asym_line_input.x_na)) {
            ComplexTensor<asymmetric_t> r_matrix =
                ComplexTensor<asymmetric_t>(asym_line_input.r_aa, asym_line_input.r_bb, asym_line_input.r_cc,
                                            asym_line_input.r_ba, asym_line_input.r_ca, asym_line_input.r_cb);
            ComplexTensor<asymmetric_t> x_matrix =
                ComplexTensor<asymmetric_t>(asym_line_input.x_aa, asym_line_input.x_bb, asym_line_input.x_cc,
                                            asym_line_input.x_ba, asym_line_input.x_ca, asym_line_input.x_cb);
            z_series_abc = r_matrix + 1.0i * x_matrix;
        } else {
            ComplexTensor4 r_matrix =
                ComplexTensor4(asym_line_input.r_aa, asym_line_input.r_bb, asym_line_input.r_cc, asym_line_input.r_nn,
                               asym_line_input.r_ba, asym_line_input.r_ca, asym_line_input.r_na, asym_line_input.r_cb,
                               asym_line_input.r_nb, asym_line_input.r_nc);
            ComplexTensor4 x_matrix =
                ComplexTensor4(asym_line_input.x_aa, asym_line_input.x_bb, asym_line_input.x_cc, asym_line_input.x_nn,
                               asym_line_input.x_ba, asym_line_input.x_ca, asym_line_input.x_na, asym_line_input.x_cb,
                               asym_line_input.x_nb, asym_line_input.x_nc);
            ComplexTensor4 z = r_matrix + 1.0i * x_matrix;
            z_series_abc = kron_reduction(z);
        }
        return z_series_abc;
    }

    ComplexTensor<asymmetric_t> compute_c_matrix_from_input(const power_grid_model::AsymLineInput& asym_line_input) {
        ComplexTensor<asymmetric_t> c_matrix;
        if (!is_nan(asym_line_input.c0) && !is_nan(asym_line_input.c1)) {
            c_matrix = ComplexTensor<asymmetric_t>{(2.0 * asym_line_input.c1 + asym_line_input.c0) / 3.0,
                                                   (asym_line_input.c0 - asym_line_input.c1) / 3.0};
        } else {
            c_matrix = ComplexTensor<asymmetric_t>(asym_line_input.c_aa, asym_line_input.c_bb, asym_line_input.c_cc,
                                                   asym_line_input.c_ba, asym_line_input.c_ca, asym_line_input.c_cb);
        }
        return c_matrix;
    }

    BranchCalcParam<symmetric_t> sym_calc_param() const override final {
        DoubleComplex y1_series =
            average_of_diagonal_of_matrix(y_series_abc_) - average_of_off_diagonal_of_matrix(y_series_abc_);
        DoubleComplex y1_shunt =
            average_of_diagonal_of_matrix(y_shunt_abc_) - average_of_off_diagonal_of_matrix(y_shunt_abc_);
        return calc_param_y_sym(y1_series, y1_shunt, 1.0);
    }

    BranchCalcParam<asymmetric_t> asym_calc_param() const override final {
        BranchCalcParam<asymmetric_t> param{};
        // not both connected
        if (!branch_status()) {
            // single connected
            if (from_status() || to_status()) {
                // branch_shunt = 0.5 * y_shunt + 1.0 / (1.0 / y_series + 2.0 / y_shunt);
                ComplexTensor<asymmetric_t> branch_shunt = ComplexTensor<asymmetric_t>();
                if ((cabs(y_shunt_abc_) >= numerical_tolerance).all()) {
                    branch_shunt = 0.5 * y_shunt_abc_ + inv(inv(y_series_abc_) + 2.0 * inv(y_shunt_abc_));
                }
                // from or to connected
                param.yff() = from_status() ? branch_shunt : ComplexTensor<asymmetric_t>();
                param.ytt() = to_status() ? branch_shunt : ComplexTensor<asymmetric_t>();
            }
        }
        // both connected
        else {
            param.ytt() = y_series_abc_ + 0.5 * y_shunt_abc_;
            param.yff() = param.ytt();
            param.yft() = -y_series_abc_;
            param.ytf() = -y_series_abc_;
        }
        return param;
    }
};
} // namespace power_grid_model
