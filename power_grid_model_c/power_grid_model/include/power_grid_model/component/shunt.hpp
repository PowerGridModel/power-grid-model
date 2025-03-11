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

class Shunt : public Appliance {
  public:
    using InputType = ShuntInput;
    using UpdateType = ShuntUpdate;
    static constexpr char const* name = "shunt";
    ComponentType math_model_type() const final { return ComponentType::shunt; }

    explicit Shunt(ShuntInput const& shunt_input, double u)
        : Appliance{shunt_input, u}, base_y_{base_i() / (u / sqrt3)} {
        update_params(shunt_input);
    }

    // getter for calculation param, shunt y
    template <symmetry_tag sym> ComplexTensor<sym> calc_param(bool is_connected_to_source = true) const {
        if (!energized(is_connected_to_source)) {
            return ComplexTensor<sym>{};
        }
        if constexpr (is_symmetric_v<sym>) {
            return y1_;
        } else {
            // abc matrix
            // 1/3 *
            // [[2y1+y0, y0-y1, y0-y1],
            //  [y0-y1, 2y1+y0, y0-y1],
            //  [y0-y1, y0-y1, 2y1+y0]]
            return ComplexTensor<asymmetric_t>{(2.0 * y1_ + y0_) / 3.0, (y0_ - y1_) / 3.0};
        }
    }

    UpdateChange update(ShuntUpdate const& update_data) {
        assert(update_data.id == this->id() || is_nan(update_data.id));
        bool changed = set_status(update_data.status);
        changed = update_params(update_data) || changed;

        // change shunt connection will not change topology, but will change parameters
        return {.topo = false, .param = changed};
    }

    ShuntUpdate inverse(ShuntUpdate update_data) const {
        assert(update_data.id == this->id() || is_nan(update_data.id));

        set_if_not_nan(update_data.status, static_cast<IntS>(this->status()));
        set_if_not_nan(update_data.g1, g1_);
        set_if_not_nan(update_data.b1, b1_);
        set_if_not_nan(update_data.g0, g0_);
        set_if_not_nan(update_data.b0, b0_);

        return update_data;
    }

  private:
    double base_y_{nan};

    double g1_{nan};
    double b1_{nan};
    double g0_{nan};
    double b0_{nan};
    DoubleComplex y1_{nan};
    DoubleComplex y0_{nan};

    template <typename T>
        requires std::same_as<T, ShuntInput> || std::same_as<T, ShuntUpdate>
    bool update_params(T shunt_params) {
        bool changed = update_param(shunt_params.g1, g1_);
        changed = update_param(shunt_params.b1, b1_) || changed;
        changed = update_param(shunt_params.g0, g0_) || changed;
        changed = update_param(shunt_params.b0, b0_) || changed;

        if (changed) {
            y1_ = (g1_ + 1.0i * b1_) / base_y_;
            y0_ = (g0_ + 1.0i * b0_) / base_y_;
        }

        return changed;
    }

    static bool update_param(double const& value, double& target) {
        if (is_nan(value) || value == target) {
            return false;
        }

        target = value;
        return true;
    }

    template <symmetry_tag sym_calc> ApplianceSolverOutput<sym_calc> u2si(ComplexValue<sym_calc> const& u) const {
        ApplianceSolverOutput<sym_calc> appliance_solver_output;
        ComplexTensor<sym_calc> const param = calc_param<sym_calc>();
        // return value should be injection direction, therefore a negative sign for i
        appliance_solver_output.i = -dot(param, u);
        appliance_solver_output.s = u * conj(appliance_solver_output.i);
        return appliance_solver_output;
    }
    ApplianceSolverOutput<symmetric_t> sym_u2si(ComplexValue<symmetric_t> const& u) const final {
        return u2si<symmetric_t>(u);
    }
    ApplianceSolverOutput<asymmetric_t> asym_u2si(ComplexValue<asymmetric_t> const& u) const final {
        return u2si<asymmetric_t>(u);
    }

    double injection_direction() const final { return -1.0; }
};

} // namespace power_grid_model
