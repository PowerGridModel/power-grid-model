// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "appliance.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/enum.hpp"
#include "../common/three_phase_tensor.hpp"
#include "base.hpp"
#include "component.hpp"

#include <cassert>
#include <cmath>
#include <complex>
#include <utility>

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
          u_ref_angle_{is_nan(source_input.u_ref_angle) ? 0.0 : source_input.u_ref_angle},
          sk_{is_nan(source_input.sk) ? default_source_sk : source_input.sk},
          rx_ratio_{is_nan(source_input.rx_ratio) ? default_source_rx_ratio : source_input.rx_ratio},
          z01_ratio_{is_nan(source_input.z01_ratio) ? default_source_z01_ratio : source_input.z01_ratio} {}

    template <symmetry_tag sym> SourceCalcParam math_param() const {
        // calculate y1 y0 ref
        double const z_abs = base_power_3p / sk_;
        double const x1 = z_abs / sqrt(rx_ratio_ * rx_ratio_ + 1.0);
        double const r1 = x1 * rx_ratio_;
        DoubleComplex const y1_ref = 1.0 / DoubleComplex{r1, x1};
        DoubleComplex const y0_ref = y1_ref / z01_ratio_;
        return SourceCalcParam{.y1 = y1_ref, .y0 = y0_ref};
    }

    // setter for u_ref
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
        assert(update_data.id == this->id() || is_nan(update_data.id));
        bool const topo_changed = set_status(update_data.status);
        bool const u_ref_changed = set_u_ref(update_data.u_ref, update_data.u_ref_angle);
        bool const param_changed_impedance =
            set_sk_rx_ratio_z01_ratio(update_data.sk, update_data.rx_ratio, update_data.z01_ratio);
        // change source connection will change both topo and param
        // change u ref will change param
        // change sk/rx_ratio/z01_ratio will change param
        return {.topo = topo_changed, .param = u_ref_changed || param_changed_impedance || topo_changed};
    }

    SourceUpdate inverse(SourceUpdate update_data) const {
        assert(update_data.id == this->id() || is_nan(update_data.id));

        set_if_not_nan(update_data.status, status_to_int(this->status()));
        set_if_not_nan(update_data.u_ref, u_ref_);
        set_if_not_nan(update_data.u_ref_angle, u_ref_angle_);
        set_if_not_nan(update_data.sk, sk_);
        set_if_not_nan(update_data.rx_ratio, rx_ratio_);
        set_if_not_nan(update_data.z01_ratio, z01_ratio_);

        return update_data;
    }

    constexpr double u_ref() const { return u_ref_; }

  private:
    double u_ref_;
    double u_ref_angle_;
    // source short circuit power
    double sk_;
    double rx_ratio_;
    double z01_ratio_;

    bool set_sk_rx_ratio_z01_ratio(double new_sk, double new_rx_ratio, double new_z01_ratio) {
        bool changed = false;
        if (!is_nan(new_sk)) {
            sk_ = new_sk;
            changed = true;
        }
        if (!is_nan(new_rx_ratio)) {
            rx_ratio_ = new_rx_ratio;
            changed = true;
        }
        if (!is_nan(new_z01_ratio)) {
            z01_ratio_ = new_z01_ratio;
            changed = true;
        }
        return changed;
    }

    double injection_direction() const final { return 1.0; }
};

} // namespace power_grid_model
