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
#include "../common/three_phase_tensor.hpp"

namespace power_grid_model {

class Appliance : public Base {
  public:
    using InputType = ApplianceInput;
    using UpdateType = ApplianceUpdate;
    template <symmetry_tag sym> using OutputType = ApplianceOutput<sym>;
    using ShortCircuitOutputType = ApplianceShortCircuitOutput;
    static constexpr char const* name = "appliance";

    Appliance(ApplianceInput const& appliance_input, double u)
        : Base{appliance_input},
          node_{appliance_input.node},
          status_{appliance_input.status != 0},
          base_i_{base_power_3p / u / sqrt3} {}

    // getter
    ID node() const { return node_; }
    bool status() const { return status_; }
    double base_i() const { return base_i_; }
    bool energized(bool is_connected_to_source) const final { return is_connected_to_source && status_; }

    // setter
    bool set_status(IntS new_status) {
        if (new_status == na_IntS) {
            return false;
        }
        if (static_cast<bool>(new_status) == status_) {
            return false;
        }
        status_ = static_cast<bool>(new_status);
        return true;
    }

    // empty output
    template <symmetry_tag sym> ApplianceOutput<sym> get_null_output() const {
        ApplianceOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }
    ApplianceShortCircuitOutput get_null_sc_output() const {
        ApplianceShortCircuitOutput output{};
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }

    template <symmetry_tag sym>
    ApplianceOutput<sym> get_output(ApplianceMathOutput<sym> const& appliance_math_output) const {
        ApplianceOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(energized(true));
        output.p = base_power<sym> * real(appliance_math_output.s) * injection_direction();
        output.q = base_power<sym> * imag(appliance_math_output.s) * injection_direction();
        output.s = base_power<sym> * cabs(appliance_math_output.s);
        output.i = base_i_ * cabs(appliance_math_output.i);
        // pf
        if constexpr (is_symmetric_v<sym>) {
            if (output.s < numerical_tolerance) {
                output.pf = 0.0;
            } else {
                output.pf = output.p / output.s;
            }
        } else {
            for (size_t j = 0; j != 3; ++j) {
                if (output.s(j) < numerical_tolerance) {
                    output.pf(j) = 0.0;
                } else {
                    output.pf(j) = output.p(j) / output.s(j);
                }
            }
        }
        return output;
    }
    ApplianceShortCircuitOutput get_sc_output(ComplexValue<asymmetric_t> const& i) const {
        ApplianceShortCircuitOutput output{};
        static_cast<BaseOutput&>(output) = base_output(energized(true));
        output.i = base_i_ * cabs(i);
        output.i_angle = arg(i * injection_direction());
        return output;
    }
    ApplianceShortCircuitOutput get_sc_output(ComplexValue<symmetric_t> const& i) const {
        ComplexValue<asymmetric_t> const iabc{i};
        return get_sc_output(iabc);
    }
    template <symmetry_tag sym> ApplianceOutput<sym> get_output(ComplexValue<sym> const& u) const {
        if constexpr (is_symmetric_v<sym>) {
            return get_output<symmetric_t>(sym_u2si(u));
        } else {
            return get_output<asymmetric_t>(asym_u2si(u));
        }
    }
    template <symmetry_tag sym>
    ApplianceShortCircuitOutput get_sc_output(ApplianceShortCircuitMathOutput<sym> const& appliance_math_output) const {
        return get_sc_output(appliance_math_output.i);
    }

  private:
    ID node_;
    bool status_;
    double base_i_;

    // pure virtual functions for translate from u to s/i
    virtual ApplianceMathOutput<symmetric_t> sym_u2si(ComplexValue<symmetric_t> const& u) const = 0;
    virtual ApplianceMathOutput<asymmetric_t> asym_u2si(ComplexValue<asymmetric_t> const& u) const = 0;

    virtual double injection_direction() const = 0;
};

} // namespace power_grid_model
