// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/common.hpp"

namespace power_grid_model {

class TransformerTapRegulator : public Base {
    using InputType = TransformerTapRegulatorInput;
    // using UpdateType = ...
    static constexpr char const* name = "transformer_tap_regulator";

    explicit TransformerTapRegulator(TransformerTapRegulatorInput const& transformer_tap_regulator_input,
                                     double u_rated)
        : Base{transformer_tap_regulator_input},
          transformer_id_{transformer_tap_regulator_input.transformer_id},
          control_side_{transformer_tap_regulator_input.control_side},
          u_set_{transformer_tap_regulator_input.u_set},
          u_band_{transformer_tap_regulator_input.u_band},
          enabled_{transformer_tap_regulator_input.enabled},
          line_drop_compensation_r_{transformer_tap_regulator_input.line_drop_compensation_r},
          line_drop_compensation_x_{transformer_tap_regulator_input.line_drop_compensation_x} {}

    // update for transformer tap regulator, hide default update for branch
    void update(TransformerTapRegulatorUpdate const& update_data) {
        assert(update_data.id == id());
        u_set_ = update_data.u_set;
        u_band_ = update_data.u_band;
        enabled_ = update_data.enabled;
        line_drop_compensation_r_ = update_data.line_drop_compensation_r;
        line_drop_compensation_x_ = update_data.line_drop_compensation_x;
    }

    TransformerTapRegulatorOutput get_output(IntS const& tap_pos) {
        TransformerTapRegulatorOutput output{};
        output.id = id();
        output.tap_pos = tap_pos;
    }

    template <symmetry_tag sym> TransformerTapRegulatorCalcParam calc_param(double const& u_rated) {
        TransformerTapRegulatorCalcParam param{};
        param.u_set = u_set_ / u_rated;
        param.u_band = u_band_ / u_rated;
        double z_base = u_rated * u_rated / base_power<sym>;
        DoubleComplex z_compensation{line_drop_compensation_r_, line_drop_compensation_x_};
        param.z_compensation = z_compensation / z_base;
        return param;
    }

  private:
    // transformer tap regulator parameters
    ID transformer_id_;
    ControlSide control_side_;
    double u_set_;
    double u_band_;
    bool enabled_;
    double line_drop_compensation_r_;
    double line_drop_compensation_x_;
};

} // namespace power_grid_model
