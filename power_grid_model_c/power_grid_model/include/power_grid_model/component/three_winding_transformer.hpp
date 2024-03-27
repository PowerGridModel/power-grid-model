// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "branch3.hpp"
#include "transformer.hpp"
#include "transformer_utils.hpp"

namespace power_grid_model {

class ThreeWindingTransformer : public Branch3 {
  public:
    using InputType = ThreeWindingTransformerInput;
    using UpdateType = ThreeWindingTransformerUpdate;
    static constexpr char const* name = "three_winding_transformer";

    explicit ThreeWindingTransformer(ThreeWindingTransformerInput const& three_winding_transformer_input,
                                     double u1_rated, double u2_rated, double u3_rated)
        : Branch3{three_winding_transformer_input},
          u1_{three_winding_transformer_input.u1},
          u2_{three_winding_transformer_input.u2},
          u3_{three_winding_transformer_input.u3},
          u1_rated_{u1_rated},
          u2_rated_{u2_rated},
          u3_rated_{u3_rated},
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
          base_i_1_{base_power_3p / u1_rated / sqrt3},
          base_i_2_{base_power_3p / u2_rated / sqrt3},
          base_i_3_{base_power_3p / u3_rated / sqrt3},
          z_grounding_1_{three_winding_transformer_input.r_grounding_1, three_winding_transformer_input.x_grounding_1},
          z_grounding_2_{three_winding_transformer_input.r_grounding_2, three_winding_transformer_input.x_grounding_2},
          z_grounding_3_{three_winding_transformer_input.r_grounding_3, three_winding_transformer_input.x_grounding_3} {
        if (!is_valid_clock(clock_12_, winding_1_, winding_2_)) {
            throw InvalidTransformerClock{id(), clock_12_};
        }
        if (!is_valid_clock(clock_13_, winding_1_, winding_3_)) {
            throw InvalidTransformerClock{id(), clock_13_};
        }

        // set clock to zero if it is 12
        clock_12_ = static_cast<IntS>(clock_12_ % 12);
        clock_13_ = static_cast<IntS>(clock_13_ % 12);
        // check tap bounds
        tap_pos_ = tap_limit(tap_pos_);
    }

    // override getter
    double base_i_1() const final { return base_i_1_; }
    double base_i_2() const final { return base_i_2_; }
    double base_i_3() const final { return base_i_3_; }
    double loading(double s_1, double s_2, double s_3) const final {
        return std::max(std::max(s_1 / sn_1_, s_2 / sn_2_), s_3 / sn_3_);
    }
    // 3-way branch, phase shift = phase_node_x - phase_internal_node
    // the clock_12 and clock_13 is reverted
    // because clock_12 is the phase shift node_1 - node_2
    // and the phase shift in the math model is node_x - node_internal
    std::array<double, 3> phase_shift() const final { return {0.0, -clock_12_ * deg_30, -clock_13_ * deg_30}; }

    // getters
    IntS tap_pos() const { return tap_pos_; }
    Branch3Side tap_side() const { return tap_side_; }
    IntS tap_min() const { return tap_min_; }
    IntS tap_max() const { return tap_max_; }
    IntS tap_nom() const { return tap_nom_; }

    // setter
    bool set_tap(IntS new_tap) {
        if (new_tap == na_IntS || new_tap == tap_pos_) {
            return false;
        }
        tap_pos_ = tap_limit(new_tap);
        return true;
    }

    UpdateChange update(ThreeWindingTransformerUpdate const& update_data) {
        assert(update_data.id == id());
        bool const topo_changed = set_status(update_data.status_1, update_data.status_2, update_data.status_3);
        bool const param_changed = set_tap(update_data.tap_pos) || topo_changed;
        return {topo_changed, param_changed};
    }

    ThreeWindingTransformerUpdate inverse(ThreeWindingTransformerUpdate update_data) const {
        assert(update_data.id == id());

        update_data = Branch3::inverse(update_data);
        set_if_not_nan(update_data.tap_pos, tap_pos_);

        return update_data;
    }

  private:
    // three winding transformer parameters
    double u1_;
    double u2_;
    double u3_;
    double u1_rated_;
    double u2_rated_;
    double u3_rated_;
    double sn_1_;
    double sn_2_;
    double sn_3_;
    double uk_12_;
    double uk_13_;
    double uk_23_;
    double pk_12_;
    double pk_13_;
    double pk_23_;
    double i0_;
    double p0_;
    WindingType winding_1_;
    WindingType winding_2_;
    WindingType winding_3_;
    IntS clock_12_;
    IntS clock_13_;
    Branch3Side tap_side_;
    IntS tap_pos_;
    IntS tap_min_;
    IntS tap_max_;
    IntS tap_nom_;
    IntS tap_direction_;
    double tap_size_;
    double uk_12_min_;
    double uk_12_max_;
    double uk_13_min_;
    double uk_13_max_;
    double uk_23_min_;
    double uk_23_max_;
    double pk_12_min_;
    double pk_12_max_;
    double pk_13_min_;
    double pk_13_max_;
    double pk_23_min_;
    double pk_23_max_;

    // calculation parameters
    double base_i_1_;
    double base_i_2_;
    double base_i_3_;
    DoubleComplex z_grounding_1_;
    DoubleComplex z_grounding_2_;
    DoubleComplex z_grounding_3_;

    IntS tap_limit(IntS new_tap) const {
        new_tap = std::min(new_tap, std::max(tap_max_, tap_min_));
        new_tap = std::max(new_tap, std::min(tap_max_, tap_min_));
        return new_tap;
    }

    std::tuple<double, double, double> calculate_uk() const {
        // adjust uk for tap from min to max range
        double const uk_12_tap =
            tap_adjust_impedance(tap_pos_, tap_min_, tap_max_, tap_nom_, uk_12_, uk_12_min_, uk_12_max_);
        double const uk_13_tap =
            tap_adjust_impedance(tap_pos_, tap_min_, tap_max_, tap_nom_, uk_13_, uk_13_min_, uk_13_max_);
        double const uk_23_tap =
            tap_adjust_impedance(tap_pos_, tap_min_, tap_max_, tap_nom_, uk_23_, uk_23_min_, uk_23_max_);

        // convert all short circuit voltages relative to side 1
        double const uk_12 = uk_12_tap * sn_1_ / std::min(sn_1_, sn_2_);
        double const uk_13 = uk_13_tap * sn_1_ / std::min(sn_1_, sn_3_);
        double const uk_23 = uk_23_tap * sn_1_ / std::min(sn_2_, sn_3_);

        // delta-wye conversion (12, 13, 23 -> 1, 2, 3)
        double const uk_T1_prime = 0.5 * (uk_12 + uk_13 - uk_23);
        double const uk_T2_prime = 0.5 * (uk_12 + uk_23 - uk_13);
        double const uk_T3_prime = 0.5 * (uk_13 + uk_23 - uk_12);

        // transform short circuit voltages back to their own voltage level
        double const uk_T1 = uk_T1_prime;
        double const uk_T2 = uk_T2_prime * (sn_2_ / sn_1_);
        double const uk_T3 = uk_T3_prime * (sn_3_ / sn_1_);
        return std::make_tuple(uk_T1, uk_T2, uk_T3);
    }

    std::tuple<double, double, double> calculate_pk() const {
        // adjust pk for tap from min to max range
        double const pk_12_tap =
            tap_adjust_impedance(tap_pos_, tap_min_, tap_max_, tap_nom_, pk_12_, pk_12_min_, pk_12_max_);
        double const pk_13_tap =
            tap_adjust_impedance(tap_pos_, tap_min_, tap_max_, tap_nom_, pk_13_, pk_13_min_, pk_13_max_);
        double const pk_23_tap =
            tap_adjust_impedance(tap_pos_, tap_min_, tap_max_, tap_nom_, pk_23_, pk_23_min_, pk_23_max_);

        // convert all short circuit losses relative to side 1
        double const pk_12 = pk_12_tap * (sn_1_ / std::min(sn_1_, sn_2_)) * (sn_1_ / std::min(sn_1_, sn_2_));
        double const pk_13 = pk_13_tap * (sn_1_ / std::min(sn_1_, sn_3_)) * (sn_1_ / std::min(sn_1_, sn_3_));
        double const pk_23 = pk_23_tap * (sn_1_ / std::min(sn_2_, sn_3_)) * (sn_1_ / std::min(sn_2_, sn_3_));

        // delta-wye conversion (12, 13, 23 -> 1, 2, 3)
        double const pk_T1_prime = 0.5 * (pk_12 + pk_13 - pk_23);
        double const pk_T2_prime = 0.5 * (pk_12 + pk_23 - pk_13);
        double const pk_T3_prime = 0.5 * (pk_13 + pk_23 - pk_12);

        // transform short circuit losses back to their own power level
        double const pk_T1 = pk_T1_prime;
        double const pk_T2 = pk_T2_prime * (sn_2_ / sn_1_) * (sn_2_ / sn_1_);
        double const pk_T3 = pk_T3_prime * (sn_3_ / sn_1_) * (sn_3_ / sn_1_);
        return std::make_tuple(pk_T1, pk_T2, pk_T3);
    }

    /*
    A three winding transformer can be modelled as three two winding transformers, between the three nodes and the
    dummy node:
        - T1: node 1 -> dummy node
        - T2: node 2 -> dummy node
        - T3: node 3 -> dummy node

    The three two winding transformers look as follows:

                                node_2
                                 /
                                T2
                               /
    node_1 -- T1 -- dummy_node
                               \
                                T3
                                 \
                                node_3

    - Each two winding transformer has a dummy id (2) and dummy nodes (0 and 1).
    - The from status is the actual status of the threewinding transformer with teh corresponding node,
      the to status is always true.
    - The voltage at the dummy node is the same as on node 1
    - i0 and p0 are only applicable to T1
    - T1 will always be YNyn0 transformer
    - The voltage levels will be calculated in advance, so tap_pos/min/max/nom/size can all be set to zero
    - uk and pk are calculated in advance, so uk_min/max and pk_min/max can be set to nan
    */
    std::array<Transformer, 3> convert_to_two_winding_transformers() const {
        // off nominal tap ratio
        auto const [u1, u2, u3] = [this]() {
            double result_u1 = u1_;
            double result_u2 = u2_;
            double result_u3 = u3_;
            if (tap_side_ == Branch3Side::side_1) {
                result_u1 += tap_direction_ * (tap_pos_ - tap_nom_) * tap_size_;
            } else if (tap_side_ == Branch3Side::side_2) {
                result_u2 += tap_direction_ * (tap_pos_ - tap_nom_) * tap_size_;
            } else {
                result_u3 += tap_direction_ * (tap_pos_ - tap_nom_) * tap_size_;
            }
            return std::make_tuple(result_u1, result_u2, result_u3);
        }();

        auto const [uk_T1, uk_T2, uk_T3] = calculate_uk();
        auto const [pk_T1, pk_T2, pk_T3] = calculate_pk();

        TransformerInput const transformer_input_T1{.id = 2,
                                                    .from_node = 0,
                                                    .to_node = 1,
                                                    .from_status = static_cast<IntS>(status_1()),
                                                    .to_status = 1,
                                                    .u1 = u1,
                                                    .u2 = u1,
                                                    .sn = sn_1_,
                                                    .uk = uk_T1,
                                                    .pk = pk_T1,
                                                    .i0 = i0_,
                                                    .p0 = p0_,
                                                    .winding_from = WindingType::wye_n,
                                                    .winding_to = WindingType::wye_n,
                                                    .clock = 0,
                                                    .tap_side = BranchSide::from,
                                                    .tap_pos = 0,
                                                    .tap_min = 0,
                                                    .tap_max = 0,
                                                    .tap_nom = 0,
                                                    .tap_size = 0.0,
                                                    .uk_min = nan,
                                                    .uk_max = nan,
                                                    .pk_min = nan,
                                                    .pk_max = nan,
                                                    .r_grounding_from = z_grounding_1_.real(),
                                                    .x_grounding_from = z_grounding_1_.imag(),
                                                    .r_grounding_to = 0,
                                                    .x_grounding_to = 0};
        TransformerInput const transformer_input_T2{.id = 2,
                                                    .from_node = 0,
                                                    .to_node = 1,
                                                    .from_status = static_cast<IntS>(status_2()),
                                                    .to_status = 1,
                                                    .u1 = u2,
                                                    .u2 = u1,
                                                    .sn = sn_2_,
                                                    .uk = uk_T2,
                                                    .pk = pk_T2,
                                                    .i0 = 0.0,
                                                    .p0 = 0.0,
                                                    .winding_from = winding_2_,
                                                    .winding_to = winding_1_,
                                                    .clock = static_cast<IntS>(12 - clock_12_), // reversed
                                                    .tap_side = BranchSide::from,
                                                    .tap_pos = 0,
                                                    .tap_min = 0,
                                                    .tap_max = 0,
                                                    .tap_nom = 0,
                                                    .tap_size = 0.0,
                                                    .uk_min = nan,
                                                    .uk_max = nan,
                                                    .pk_min = nan,
                                                    .pk_max = nan,
                                                    .r_grounding_from = z_grounding_2_.real(),
                                                    .x_grounding_from = z_grounding_2_.imag(),
                                                    .r_grounding_to = 0,
                                                    .x_grounding_to = 0};
        TransformerInput const transformer_input_T3{.id = 2,
                                                    .from_node = 0,
                                                    .to_node = 1,
                                                    .from_status = static_cast<IntS>(status_3()),
                                                    .to_status = 1,
                                                    .u1 = u3,
                                                    .u2 = u1,
                                                    .sn = sn_3_,
                                                    .uk = uk_T3,
                                                    .pk = pk_T3,
                                                    .i0 = 0.0,
                                                    .p0 = 0.0,
                                                    .winding_from = winding_3_,
                                                    .winding_to = winding_1_,
                                                    .clock = static_cast<IntS>(12 - clock_13_), // reversed
                                                    .tap_side = BranchSide::from,
                                                    .tap_pos = 0,
                                                    .tap_min = 0,
                                                    .tap_max = 0,
                                                    .tap_nom = 0,
                                                    .tap_size = 0.0,
                                                    .uk_min = nan,
                                                    .uk_max = nan,
                                                    .pk_min = nan,
                                                    .pk_max = nan,
                                                    .r_grounding_from = z_grounding_3_.real(),
                                                    .x_grounding_from = z_grounding_3_.imag(),
                                                    .r_grounding_to = 0,
                                                    .x_grounding_to = 0};

        Transformer const T1{transformer_input_T1, u1_rated_, u1_rated_};
        Transformer const T2{transformer_input_T2, u2_rated_, u1_rated_};
        Transformer const T3{transformer_input_T3, u3_rated_, u1_rated_};

        return {T1, T2, T3};
    }

    // calculate branch parameters
    std::array<BranchCalcParam<symmetric_t>, 3> sym_calc_param() const final {
        std::array<Transformer, 3> const transformer_array = convert_to_two_winding_transformers();
        std::array<BranchCalcParam<symmetric_t>, 3> transformer_params{};
        for (size_t i = 0; i < transformer_array.size(); i++) {
            transformer_params[i] = transformer_array[i].calc_param<symmetric_t>();
        }
        return transformer_params;
    }
    std::array<BranchCalcParam<asymmetric_t>, 3> asym_calc_param() const final {
        std::array<Transformer, 3> const transformer_array = convert_to_two_winding_transformers();
        std::array<BranchCalcParam<asymmetric_t>, 3> transformer_params{};
        for (size_t i = 0; i < transformer_array.size(); i++) {
            transformer_params[i] = transformer_array[i].calc_param<asymmetric_t>();
        }
        return transformer_params;
    }
};

} // namespace power_grid_model
