// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_SHUNT_HPP
#define POWER_GRID_MODEL_COMPONENT_SHUNT_HPP

#include "appliance.hpp"
#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

class Shunt : public Appliance {
   public:
    using InputType = ShuntInput;
    using UpdateType = ApplianceUpdate;
    static constexpr char const* name = "shunt";
    ComponentType math_model_type() const final {
        return ComponentType::shunt;
    }

    explicit Shunt(ShuntInput const& shunt_input, double u) : Appliance{shunt_input, u} {
        double const base_y = base_i() / (u / sqrt3);
        y1_ = (shunt_input.g1 + 1.0i * shunt_input.b1) / base_y;
        y0_ = (shunt_input.g0 + 1.0i * shunt_input.b0) / base_y;
    }

    // getter for calculation param, shunt y
    template <bool sym>
    ComplexTensor<sym> calc_param(bool is_connected_to_source = true) const {
        if (!energized(is_connected_to_source)) {
            return ComplexTensor<sym>{};
        }
        if constexpr (sym) {
            return y1_;
        }
        else {
            // abc matrix
            // 1/3 *
            // [[2y1+y0, y0-y1, y0-y1],
            //  [y0-y1, 2y1+y0, y0-y1],
            //  [y0-y1, y0-y1, 2y1+y0]]
            return ComplexTensor<false>{(2.0 * y1_ + y0_) / 3.0, (y0_ - y1_) / 3.0};
        }
    }

    // update for shunt
    UpdateChange update(ApplianceUpdate const& update) {
        assert(update.id == id());
        bool const changed = set_status(update.status);
        // change shunt connection will not change topology, but will change parameters
        return {false, changed};
    }

   private:
    DoubleComplex y1_, y0_;

    template <bool sym_calc>
    ApplianceMathOutput<sym_calc> u2si(ComplexValue<sym_calc> const& u) const {
        ApplianceMathOutput<sym_calc> appliance_math_output;
        ComplexTensor<sym_calc> const param = calc_param<sym_calc>();
        // return value should be injection direction, therefore a negative sign for i
        appliance_math_output.i = -dot(param, u);
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
        return -1.0;
    }
};

}  // namespace power_grid_model

#endif
