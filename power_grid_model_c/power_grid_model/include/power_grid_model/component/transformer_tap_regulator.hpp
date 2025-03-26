// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base.hpp"
#include "regulator.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/common.hpp"

namespace power_grid_model {

class TransformerTapRegulator : public Regulator {
  public:
    using InputType = TransformerTapRegulatorInput;
    using UpdateType = TransformerTapRegulatorUpdate;
    template <symmetry_tag sym> using OutputType = TransformerTapRegulatorOutput;

    static constexpr char const* name = "transformer_tap_regulator";

    explicit TransformerTapRegulator(TransformerTapRegulatorInput const& transformer_tap_regulator_input,
                                     ComponentType regulated_object_type, double u_rated)
        : Regulator{transformer_tap_regulator_input, regulated_object_type},
          control_side_{transformer_tap_regulator_input.control_side},
          u_rated_{u_rated},
          u_set_{transformer_tap_regulator_input.u_set},
          u_band_{transformer_tap_regulator_input.u_band},
          line_drop_compensation_r_{transformer_tap_regulator_input.line_drop_compensation_r},
          line_drop_compensation_x_{transformer_tap_regulator_input.line_drop_compensation_x} {}

    // update for transformer tap regulator, hide default update for branch
    UpdateChange update(TransformerTapRegulatorUpdate const& update_data) {
        assert(update_data.id == this->id() || is_nan(update_data.id));
        set_status(update_data.status);
        update_real_value<symmetric_t>(update_data.u_set, u_set_, 1.0);
        update_real_value<symmetric_t>(update_data.u_band, u_band_, 1.0);
        update_real_value<symmetric_t>(update_data.line_drop_compensation_r, line_drop_compensation_r_, 1.0);
        update_real_value<symmetric_t>(update_data.line_drop_compensation_x, line_drop_compensation_x_, 1.0);
        return {.topo = false, .param = false};
    }

    TransformerTapRegulatorUpdate inverse(TransformerTapRegulatorUpdate update_data) const {
        assert(update_data.id == this->id() || is_nan(update_data.id));

        update_data = Regulator::inverse(update_data);
        set_if_not_nan(update_data.u_set, u_set_);
        set_if_not_nan(update_data.u_band, u_band_);
        set_if_not_nan(update_data.line_drop_compensation_r, line_drop_compensation_r_);
        set_if_not_nan(update_data.line_drop_compensation_x, line_drop_compensation_x_);

        return update_data;
    }

    constexpr TransformerTapRegulatorOutput get_null_output() const {
        return {.id = id(), .energized = 0, .tap_pos = na_IntS};
    }
    TransformerTapRegulatorOutput get_output(IntS const& tap_pos) const {
        TransformerTapRegulatorOutput output{};
        output.id = id();
        output.energized = static_cast<IntS>(energized(true));
        output.tap_pos = tap_pos;
        return output;
    }
    constexpr RegulatorShortCircuitOutput get_null_sc_output() const { return {.id = id(), .energized = 0}; }

    template <symmetry_tag sym> TransformerTapRegulatorCalcParam calc_param() const {
        TransformerTapRegulatorCalcParam param{};
        param.u_set = u_set_ / u_rated_;
        param.u_band = u_band_ / u_rated_;
        double const z_base = u_rated_ * u_rated_ / base_power<sym>;
        DoubleComplex const z_compensation{is_nan(line_drop_compensation_r_) ? 0.0 : line_drop_compensation_r_,
                                           is_nan(line_drop_compensation_x_) ? 0.0 : line_drop_compensation_x_};
        param.z_compensation = z_compensation / z_base;
        param.status = static_cast<IntS>(Regulator::status());
        return param;
    }

    // getter
    ControlSide control_side() const { return control_side_; }

  private:
    // transformer tap regulator parameters
    ControlSide control_side_;
    double u_rated_;
    double u_set_;
    double u_band_;
    double line_drop_compensation_r_;
    double line_drop_compensation_x_;
};

} // namespace power_grid_model
