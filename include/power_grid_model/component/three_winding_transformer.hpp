// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_THREE_WINDING_TRANSFORMER_HPP
#define POWER_GRID_MODEL_COMPONENT_THREE_WINDING_TRANSFORMER_HPP

#include "branch3.hpp"

namespace power_grid_model {

class ThreeWindingTransformer : public Branch3 {
   public:
    using InputType = ThreeWindingTransformerInput;
    using UpdateType = ThreeWindingTransformerUpdate;
    static constexpr char const* name = "three_winding_transformer";

    ThreeWindingTransformer(ThreeWindingTransformerInput const& three_winding_transformer_input, double u1_rated,
                            double u2_rated, double u3_rated)
        : Branch3{three_winding_transformer_input},
          u1_{three_winding_transformer_input.u1},
          u2_{three_winding_transformer_input.u2},
          u3_{three_winding_transformer_input.u3},
          sn_1_{three_winding_transformer_input.sn_1},
          sn_2_{three_winding_transformer_input.sn_2},
          sn_3_{three_winding_transformer_input.sn_3},
          uk_12_{three_winding_transformer_input.uk_12},
          uk_13_{three_winding_transformer_input.uk_13},
          uk_23_{three_winding_transformer_input.uk_23},
          pk_12_{three_winding_transformer_input.pk_12},
          pk_13_{three_winding_transformer_input.pk_13},
          pk_23_{three_winding_transformer_input.pk_23},
          i0_{three_winding_transformer_input.i0},
          p0_{three_winding_transformer_input.p0},
          winding_1_{three_winding_transformer_input.winding_1},
          winding_2_{three_winding_transformer_input.winding_2},
          winding_3_{three_winding_transformer_input.winding_3},
          clock_12_{three_winding_transformer_input.clock_12},
          clock_13_{three_winding_transformer_input.clock_13},
          clock_23_{three_winding_transformer_input.clock_23},
          tap_side_{three_winding_transformer_input.tap_side},
          tap_pos_{three_winding_transformer_input.tap_pos},
          tap_min_{three_winding_transformer_input.tap_min},
          tap_max_{three_winding_transformer_input.tap_max},
          tap_nom_{three_winding_transformer_input.tap_nom == na_IntS ? (IntS)0
                                                                      : three_winding_transformer_input.tap_nom},
          tap_direction_{tap_max_ > tap_min_ ? (IntS)1 : (IntS)-1},
          tap_size_{three_winding_transformer_input.tap_size},
          uk_12_min_{is_nan(three_winding_transformer_input.uk_12_min) ? uk_12_
                                                                       : three_winding_transformer_input.uk_12_min},
          uk_12_max_{is_nan(three_winding_transformer_input.uk_12_max) ? uk_12_
                                                                       : three_winding_transformer_input.uk_12_max},
          uk_13_min_{is_nan(three_winding_transformer_input.uk_13_min) ? uk_13_
                                                                       : three_winding_transformer_input.uk_13_min},
          uk_13_max_{is_nan(three_winding_transformer_input.uk_13_max) ? uk_13_
                                                                       : three_winding_transformer_input.uk_13_max},
          uk_23_min_{is_nan(three_winding_transformer_input.uk_23_min) ? uk_23_
                                                                       : three_winding_transformer_input.uk_23_min},
          uk_23_max_{is_nan(three_winding_transformer_input.uk_23_max) ? uk_23_
                                                                       : three_winding_transformer_input.uk_23_max},
          pk_12_min_{is_nan(three_winding_transformer_input.pk_12_min) ? pk_12_
                                                                       : three_winding_transformer_input.pk_12_min},
          pk_12_max_{is_nan(three_winding_transformer_input.pk_12_max) ? pk_12_
                                                                       : three_winding_transformer_input.pk_12_max},
          pk_13_min_{is_nan(three_winding_transformer_input.pk_13_min) ? pk_13_
                                                                       : three_winding_transformer_input.pk_13_min},
          pk_13_max_{is_nan(three_winding_transformer_input.pk_13_max) ? pk_13_
                                                                       : three_winding_transformer_input.pk_13_max},
          pk_23_min_{is_nan(three_winding_transformer_input.pk_23_min) ? pk_23_
                                                                       : three_winding_transformer_input.pk_23_min},
          pk_23_max_{is_nan(three_winding_transformer_input.pk_23_max) ? pk_23_
                                                                       : three_winding_transformer_input.pk_23_max},
          base_i_1{base_power_3p / u1_rated / sqrt3},
          base_i_2{base_power_3p / u2_rated / sqrt3},
          base_i_3{base_power_3p / u3_rated / sqrt3},
          z_grounding_1{calculate_z_pu(three_winding_transformer_input.r_grounding_1,
                                       three_winding_transformer_input.x_grounding_1, u1_rated)},
          z_grounding_2{calculate_z_pu(three_winding_transformer_input.r_grounding_2,
                                       three_winding_transformer_input.x_grounding_2, u2_rated)},
          z_grounding_3{calculate_z_pu(three_winding_transformer_input.r_grounding_3,
                                       three_winding_transformer_input.x_grounding_3, u3_rated)} {
        // TODO
    }

    // override getter
    double base_i_1() const final {
        return base_i_1;
    }
    double base_i_2() const final {
        return base_i_2;
    }
    double base_i_3() const final {
        return base_i_3;
    }

    // setter
    bool set_tap(IntS new_tap) {
        if (new_tap == na_IntS || new_tap == tap_pos_) {
            return false;
        }
        tap_pos_ = tap_limit(new_tap);
        return true;
    }

    UpdateChange update(ThreeWindingTransformerUpdate const& update) {
        assert(update.id = id());
        bool topo_changed = set_status(update.status_1, update.status_2, update.status_3);
        bool param_changed = set_tap(update.tap_pos) || topo_changed;
        return {topo_changed, param_changed};
    }

   private:
    double u1_, u2_, u3_;
    double sn_1_, sn_2_, sn_3_;
    double uk_12_, uk_13_, uk_23_, pk_12_, pk_13_, pk_23_, i0_, p0_;
    WindingType winding_1_, winding_2_, winding_3_;
    IntS clock_12_, clock_13_, clock_23_;
    Branch3Side tap_side_;
    IntS tap_pos_, tap_min_, tap_max_, tap_nom_, tap_direction_;
    double tap_size_;
    double uk_12_min_, uk_12_max_, uk_13_min_, uk_13_max_, uk_23_min_, uk_23_max_;
    double pk_12_min_, pk_12_max_, pk_13_min_, pk_13_max_, pk_23_min_, pk_23_max_;

    // calculate z in per unit with NaN detection
    DoubleComplex calculate_z_pu(double r, double x, double u) {
        r = is_nan(r) ? 0 : r;
        x = is_nan(x) ? 0 : x;
        double const base_z = u * u / base_power_3p;
        return {r / base_z, x / base_z};
    }

    IntS tap_limit(IntS new_tap) const {
        new_tap = std::min(new_tap, std::max(tap_max_, tap_min_));
        new_tap = std::max(new_tap, std::min(tap_max_, tap_min_));
        return new_tap;
    }
};

}  // namespace power_grid_model

#endif