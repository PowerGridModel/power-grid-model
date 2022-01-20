// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_SOURCE_HPP
#define POWER_GRID_MODEL_COMPONENT_SOURCE_HPP

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../calculation_parameters.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "appliance.hpp"
#include "base.hpp"

namespace power_grid_model {

class Source : public Appliance {
   public:
    using InputType = SourceInput;
    using UpdateType = SourceUpdate;
    static constexpr char const* name = "source";
    ComponentType math_model_type() const final {
        return ComponentType::source;
    }

    Source(SourceInput const& source_input, double u)
        : Appliance{source_input, u}, u_ref_{source_input.u_ref}, y1_ref_{}, y0_ref_{} {
        double const sk{is_nan(source_input.sk) ? default_source_sk : source_input.sk};
        double const rx_ratio{is_nan(source_input.rx_ratio) ? default_source_rx_ratio : source_input.rx_ratio};
        double const z01_ratio{is_nan(source_input.z01_ratio) ? default_source_z01_ratio : source_input.z01_ratio};
        calculate_y_ref(sk, rx_ratio, z01_ratio);
    }

    // calculate y1 y0 ref
    void calculate_y_ref(double sk, double rx_ratio, double z01_ratio) {
        double const z_abs = base_power_3p / sk;  // s_pu = s/base_s, z = u^2/s = 1/s = base_s/s_pu
        double const x1 = z_abs / sqrt(rx_ratio * rx_ratio + 1.0);
        double const r1 = x1 * rx_ratio;
        y1_ref_ = 1.0 / DoubleComplex{r1, x1};
        y0_ref_ = y1_ref_ / z01_ratio;
    }

    // getter for calculation param, source voltage pu
    template <bool sym>
    SourceCalcParam<sym> calc_param(bool is_connected_to_source = true) const {
        if (!energized(is_connected_to_source)) {
            return SourceCalcParam<sym>{};
        }
        SourceCalcParam<sym> param{};
        param.u_ref = u_ref_;
        // internal element_admittance
        if constexpr (sym) {
            param.y_ref = y1_ref_;
        }
        else {
            ComplexTensor<false> const sym_matrix = get_sym_matrix();
            ComplexTensor<false> const sym_matrix_inv = get_sym_matrix_inv();
            ComplexTensor<false> y012;
            y012 << y1_ref_, 0.0, 0.0, 0.0, y1_ref_, 0.0, 0.0, 0.0, y0_ref_;
            param.y_ref = dot(sym_matrix, y012, sym_matrix_inv);
        }
        return param;
    }

    // setter
    void set_u_ref(double new_u_ref) {
        if (!is_nan(new_u_ref))
            u_ref_ = new_u_ref;
    }
    // getter
    double u_ref() const {
        return u_ref_;
    }

    // update for source
    UpdateChange update(SourceUpdate const& update) {
        assert(update.id == id());
        bool const changed = set_status(update.status);
        set_u_ref(update.u_ref);
        // change source connection will change both topo and param
        // change u ref will not change topo or param
        return {changed, changed};
    }

   private:
    double u_ref_;
    // positive and zero sequence ref
    DoubleComplex y1_ref_;
    DoubleComplex y0_ref_;

    template <bool sym_calc>
    ApplianceMathOutput<sym_calc> u2si(ComplexValue<sym_calc> const& u) const {
        ApplianceMathOutput<sym_calc> appliance_math_output;
        ComplexValue<sym_calc> const u_ref{u_ref_};
        ComplexTensor<sym_calc> const y_ref = calc_param<sym_calc>().y_ref;
        appliance_math_output.i = dot(y_ref, u_ref - u);
        appliance_math_output.s = u * conj(appliance_math_output.i);
        return appliance_math_output;
    }

    ApplianceMathOutput<true> sym_u2si(ComplexValue<true> const& u) const final {
        return u2si<true>(u);
    }
    ApplianceMathOutput<false> asym_u2si(ComplexValue<false> const& u) const final {
        return u2si<false>(u);
    }

    double injection_direction() const final {
        return 1.0;
    }
};

}  // namespace power_grid_model

#endif