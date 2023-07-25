// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_TRANSFORMER_HPP
#define POWER_GRID_MODEL_COMPONENT_TRANSFORMER_HPP

#include "branch.hpp"
#include "transformer_utils.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

class Transformer : public Branch {
   public:
    using InputType = TransformerInput;
    using UpdateType = TransformerUpdate;
    static constexpr char const* name = "transformer";

    explicit Transformer(TransformerInput const& transformer_input, double u1_rated, double u2_rated)
        : Branch{transformer_input},
          u1_{transformer_input.u1},
          u2_{transformer_input.u2},
          sn_{transformer_input.sn},
          tap_size_{transformer_input.tap_size},
          uk_{transformer_input.uk},
          pk_{transformer_input.pk},
          i0_{transformer_input.i0},
          p0_{transformer_input.p0},
          winding_from_{transformer_input.winding_from},
          winding_to_{transformer_input.winding_to},
          clock_{transformer_input.clock},
          tap_side_{transformer_input.tap_side},
          tap_pos_{transformer_input.tap_pos},
          tap_min_{transformer_input.tap_min},
          tap_max_{transformer_input.tap_max},
          tap_nom_{transformer_input.tap_nom == na_IntS ? (IntS)0 : transformer_input.tap_nom},
          tap_direction_{tap_max_ > tap_min_ ? (IntS)1 : (IntS)-1},
          uk_min_{is_nan(transformer_input.uk_min) ? uk_ : transformer_input.uk_min},
          uk_max_{is_nan(transformer_input.uk_max) ? uk_ : transformer_input.uk_max},
          pk_min_{is_nan(transformer_input.pk_min) ? pk_ : transformer_input.pk_min},
          pk_max_{is_nan(transformer_input.pk_max) ? pk_ : transformer_input.pk_max},
          base_i_from_{base_power_3p / u1_rated / sqrt3},
          base_i_to_{base_power_3p / u2_rated / sqrt3},
          nominal_ratio_{u1_rated / u2_rated},
          z_grounding_from_{
              calculate_z_pu(transformer_input.r_grounding_from, transformer_input.x_grounding_from, u1_rated)},
          z_grounding_to_{
              calculate_z_pu(transformer_input.r_grounding_to, transformer_input.x_grounding_to, u2_rated)} {
        if (!is_valid_clock(clock_, winding_from_, winding_to_)) {
            throw InvalidTransformerClock{id(), clock_};
        }

        // set clock to zero if it is 12
        clock_ = static_cast<IntS>(clock_ % 12);
        // check tap bounds
        tap_pos_ = tap_limit(tap_pos_);
    }

    // override getter
    double base_i_from() const final {
        return base_i_from_;
    }
    double base_i_to() const final {
        return base_i_to_;
    }
    double loading(double max_s, double) const final {
        return max_s / sn_;
    };
    // phase shift is theta_from - theta_to
    double phase_shift() const final {
        return clock_ * deg_30;
    }
    bool is_param_mutable() const final {
        return true;
    }
    // get tap
    IntS tap_pos() const {
        return tap_pos_;
    }
    // setter
    bool set_tap(IntS new_tap) {
        if (new_tap == na_IntS || new_tap == tap_pos_) {
            return false;
        }
        tap_pos_ = tap_limit(new_tap);
        return true;
    }

    // update for transformer, hide default update for branch
    UpdateChange update(TransformerUpdate const& update) {
        assert(update.id == id());
        bool const topo_changed = set_status(update.from_status, update.to_status);
        bool const param_changed = set_tap(update.tap_pos) || topo_changed;
        return {topo_changed, param_changed};
    }

   private:
    // transformer parameter
    double u1_;
    double u2_;
    double sn_;
    double tap_size_;
    double uk_;
    double pk_;
    double i0_;
    double p0_;
    WindingType winding_from_;
    WindingType winding_to_;
    IntS clock_;
    BranchSide tap_side_;
    IntS tap_pos_;
    IntS tap_min_;
    IntS tap_max_;
    IntS tap_nom_;
    IntS tap_direction_;
    double uk_min_;
    double uk_max_;
    double pk_min_;
    double pk_max_;

    // calculation parameter
    double base_i_from_;
    double base_i_to_;
    double nominal_ratio_;
    DoubleComplex z_grounding_from_;
    DoubleComplex z_grounding_to_;

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

    // calculate transformer parameter
    std::tuple<DoubleComplex, DoubleComplex, double> transformer_params() const {
        double const base_y_to = base_i_to_ * base_i_to_ / base_power_1p;
        // off nominal tap ratio
        auto const [u1, u2] = [this]() {
            double result_u1 = u1_;
            double result_u2 = u2_;
            if (tap_side_ == BranchSide::from) {
                result_u1 += tap_direction_ * (tap_pos_ - tap_nom_) * tap_size_;
            }
            else {
                result_u2 += tap_direction_ * (tap_pos_ - tap_nom_) * tap_size_;
            }
            return std::pair{result_u1, result_u2};
        }();
        double const k = (u1 / u2) / nominal_ratio_;
        // pk and uk
        double const uk = tap_adjust_impedance(tap_pos_, tap_min_, tap_max_, tap_nom_, uk_, uk_min_, uk_max_);
        double const pk = tap_adjust_impedance(tap_pos_, tap_min_, tap_max_, tap_nom_, pk_, pk_min_, pk_max_);

        // series
        DoubleComplex z_series{};
        DoubleComplex y_series{};
        // sign of uk
        // uk can be negative for artificial transformer from 3-winding
        // in this case, the imaginary part of z_series should be negative
        double const uk_sign = (uk >= 0) ? 1.0 : -1.0;
        // Z = abs(uk)*U2^2/S
        double const z_series_abs = cabs(uk) * u2 * u2 / sn_;
        // R = pk * U2^2/S^2
        // pk can be negative for aritificual transformer from 3-winding
        // in this case, the real part of z_series should be negative
        z_series.real(pk * u2 * u2 / sn_ / sn_);
        // X = uk_sign * sqrt(Z^2 - R^2)
        z_series.imag(uk_sign * std::sqrt(z_series_abs * z_series_abs - z_series.real() * z_series.real()));
        // y series
        y_series = (1.0 / z_series) / base_y_to;
        // shunt
        DoubleComplex y_shunt;
        // Y = I0_2 / (U2/sqrt(3)) = i0 * (S / sqrt(3) / U2) / (U2/sqrt(3)) = i0 * S * / U2 / U2
        double const y_shunt_abs = i0_ * sn_ / u2 / u2;
        // G = P0 / (U2^2)
        y_shunt.real(p0_ / u2 / u2);
        if (y_shunt.real() > y_shunt_abs) {
            y_shunt.imag(0.0);
        }
        else {
            y_shunt.imag(-std::sqrt(y_shunt_abs * y_shunt_abs - y_shunt.real() * y_shunt.real()));
        }
        // y shunt
        y_shunt = y_shunt / base_y_to;
        // return
        return std::make_tuple(y_series, y_shunt, k);
    }

    // branch param
    BranchCalcParam<true> sym_calc_param() const final {
        auto const [y_series, y_shunt, k] = transformer_params();
        return calc_param_y_sym(y_series, y_shunt, k * std::exp(1.0i * (clock_ * deg_30)));
    }
    BranchCalcParam<false> asym_calc_param() const final {
        auto const [y_series, y_shunt, k] = transformer_params();
        // positive sequence
        auto const param1 = calc_param_y_sym(y_series, y_shunt, k * std::exp(1.0i * (clock_ * deg_30)));
        // negative sequence
        auto const param2 = calc_param_y_sym(y_series, y_shunt, k * std::exp(1.0i * (-clock_ * deg_30)));
        // zero sequence, default zero
        BranchCalcParam<true> param0{};
        // YNyn
        if (winding_from_ == WindingType::wye_n && winding_to_ == WindingType::wye_n) {
            double phase_shift_0 = 0.0;
            // flip sign for reverse connected
            if (clock_ == 2 || clock_ == 6 || clock_ == 10) {
                phase_shift_0 = 6.0 * deg_30;
            }
            DoubleComplex const z0_series = 1.0 / y_series + 3.0 * (z_grounding_to_ + z_grounding_from_ / k / k);
            DoubleComplex const y0_series = 1.0 / z0_series;
            param0 = calc_param_y_sym(y0_series, y_shunt, k * std::exp(1.0i * phase_shift_0));
        }
        // YNd
        if (winding_from_ == WindingType::wye_n && winding_to_ == WindingType::delta && from_status()) {
            DoubleComplex const z0_series = 1.0 / y_series + 3.0 * z_grounding_from_ / k / k;
            DoubleComplex const y0_series = 1.0 / z0_series;
            param0.yff() = (y0_series + y_shunt) / k / k;
        }
        // Dyn
        if (winding_from_ == WindingType::delta && winding_to_ == WindingType::wye_n && to_status()) {
            DoubleComplex const z0_series = 1.0 / y_series + 3.0 * z_grounding_to_;
            DoubleComplex const y0_series = 1.0 / z0_series;
            param0.ytt() = (y0_series + y_shunt);
        }
        // ZN*
        if (winding_from_ == WindingType::zigzag_n && from_status()) {
            DoubleComplex const z0_series = (1.0 / y_series) * 0.1 + 3.0 * z_grounding_from_ / k / k;
            DoubleComplex const y0_series = 1.0 / z0_series;
            param0.yff() = y0_series / k / k;
        }
        // *zn
        if (winding_to_ == WindingType::zigzag_n && to_status()) {
            DoubleComplex const z0_series = (1.0 / y_series) * 0.1 + 3.0 * z_grounding_to_;
            DoubleComplex const y0_series = 1.0 / z0_series;
            param0.ytt() = y0_series;
        }

        // for the rest param0 is zero
        // calculate yabc
        ComplexTensor<false> const sym_matrix = get_sym_matrix();
        ComplexTensor<false> const sym_matrix_inv = get_sym_matrix_inv();
        BranchCalcParam<false> param;
        for (size_t i = 0; i != 4; ++i) {
            // Yabc = A * Y012 * A^-1
            ComplexTensor<false> y012;
            y012 << param0.value[i], 0.0, 0.0, 0.0, param1.value[i], 0.0, 0.0, 0.0, param2.value[i];
            param.value[i] = dot(sym_matrix, y012, sym_matrix_inv);
        }
        return param;
    }
};

}  // namespace power_grid_model

#endif
