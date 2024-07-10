// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "appliance.hpp"
#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/three_phase_tensor.hpp"

namespace power_grid_model {

class Source : public Appliance {
  public:
    using InputType = SourceInput;
    using UpdateType = SourceUpdate;
    static constexpr char const* name = "source";
    ComponentType math_model_type() const final { return ComponentType::source; }

    explicit Source(SourceInput const& source_input, double u)
        : Appliance{source_input, u},
          u_ref_{source_input.u_ref},
          u_ref_angle_{is_nan(source_input.u_ref_angle) ? 0.0 : source_input.u_ref_angle} {
        double const sk{is_nan(source_input.sk) ? default_source_sk : source_input.sk};
        double const rx_ratio{is_nan(source_input.rx_ratio) ? default_source_rx_ratio : source_input.rx_ratio};
        double const z01_ratio{is_nan(source_input.z01_ratio) ? default_source_z01_ratio : source_input.z01_ratio};
        calculate_y_ref(sk, rx_ratio, z01_ratio);
    }

    // calculate y1 y0 ref
    void calculate_y_ref(double sk, double rx_ratio, double z01_ratio) {
        double const z_abs = base_power_3p / sk; // s_pu = s/base_s, z = u^2/s = 1/s = base_s/s_pu
        double const x1 = z_abs / sqrt(rx_ratio * rx_ratio + 1.0);
        double const r1 = x1 * rx_ratio;
        y1_ref_ = 1.0 / DoubleComplex{r1, x1};
        y0_ref_ = y1_ref_ / z01_ratio;
    }

    // getter for calculation param, y_ref
    template <symmetry_tag sym> ComplexTensor<sym> math_param() const {
        // internal element_admittance
        if constexpr (is_symmetric_v<sym>) {
            return y1_ref_;
        } else {
            ComplexTensor<asymmetric_t> const sym_matrix = get_sym_matrix();
            ComplexTensor<asymmetric_t> const sym_matrix_inv = get_sym_matrix_inv();
            ComplexTensor<asymmetric_t> y012;
            y012 << y1_ref_, 0.0, 0.0, 0.0, y1_ref_, 0.0, 0.0, 0.0, y0_ref_;
            ComplexTensor<asymmetric_t> yabc = dot(sym_matrix, y012, sym_matrix_inv);
            return yabc;
        }
    }

    // setter
    bool set_u_ref(double new_u_ref, double new_u_ref_angle) {
        bool changed = false;
        if (!is_nan(new_u_ref)) {
            u_ref_ = new_u_ref;
            changed = true;
        }
        if (!is_nan(new_u_ref_angle)) {
            u_ref_angle_ = new_u_ref_angle;
            changed = true;
        }
        return changed;
    }
    // getter for u_ref for calc_param
    DoubleComplex calc_param() const { return u_ref_ * std::exp(1.0i * u_ref_angle_); }
    // This function receives the nominal voltage of the node and a min/max scaling enum
    // and returns the reference voltage based on the voltage scaling factor c.
    // the scaling factor is determined according to the IEC 60909 standard, which is shown in the table below:
    //                   |   c_max   |   c_min   |
    // Unom <= 1000V     |   1.10    |   0.95    |
    // Unom > 1kV        |   1.10    |   1.00    |
    // NOTE: for low voltage there is a difference in c for systems with a voltage tolerance of 6% or 10%.
    // Here, a voltage tolerance of 10% is assumed.
    DoubleComplex calc_param(std::pair<double, ShortCircuitVoltageScaling> const& data) const {
        double const voltage_scaling_c = [&data] {
            if (data.second == ShortCircuitVoltageScaling::maximum) {
                return 1.1;
            }
            if (data.first <= 1000.0) {
                return 0.95;
            }
            return 1.0;
        }();
        return voltage_scaling_c * std::exp(1.0i * u_ref_angle_);
    }

    // update for source
    UpdateChange update(SourceUpdate const& update_data) {
        assert(update_data.id == id());
        bool const topo_changed = set_status(update_data.status);
        bool const param_changed = set_u_ref(update_data.u_ref, update_data.u_ref_angle);
        // change source connection will change both topo and param
        // change u ref will change param
        return {topo_changed, param_changed || topo_changed};
    }

    SourceUpdate inverse(SourceUpdate update_data) const {
        assert(update_data.id == id());

        set_if_not_nan(update_data.status, static_cast<IntS>(this->status()));
        set_if_not_nan(update_data.u_ref, u_ref_);
        set_if_not_nan(update_data.u_ref_angle, u_ref_angle_);

        return update_data;
    }

  private:
    double u_ref_;
    double u_ref_angle_;
    // positive and zero sequence ref
    DoubleComplex y1_ref_{};
    DoubleComplex y0_ref_{};

    template <symmetry_tag sym_calc> ApplianceSolverOutput<sym_calc> u2si(ComplexValue<sym_calc> const& u) const {
        ApplianceSolverOutput<sym_calc> appliance_solver_output;
        ComplexValue<sym_calc> const u_ref{u_ref_};
        ComplexTensor<sym_calc> const y_ref = math_param<sym_calc>();
        appliance_solver_output.i = dot(y_ref, u_ref - u);
        appliance_solver_output.s = u * conj(appliance_solver_output.i);
        return appliance_solver_output;
    }

    ApplianceSolverOutput<symmetric_t> sym_u2si(ComplexValue<symmetric_t> const& u) const final {
        return u2si<symmetric_t>(u);
    }
    ApplianceSolverOutput<asymmetric_t> asym_u2si(ComplexValue<asymmetric_t> const& u) const final {
        return u2si<asymmetric_t>(u);
    }

    double injection_direction() const final { return 1.0; }
};

} // namespace power_grid_model
