// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_SHORT_CIRCUIT_HPP
#define POWER_GRID_MODEL_COMPONENT_SHORT_CIRCUIT_HPP

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "base.hpp"

namespace power_grid_model {

class Fault final : public Base {
   public:
    using InputType = FaultInput;
    using UpdateType = FaultUpdate;
    template <bool sym>
    using OutputType = FaultOutput;
    static constexpr char const* name = "fault";
    ComponentType math_model_type() const final {
        return ComponentType::fault;
    }

    Fault(FaultInput const& fault_input)
        : Base{fault_input},
          fault_object_{fault_input.fault_object},
          r_f_{is_nan(fault_input.r_f) ? (bool)0.0 : fault_input.r_f},
          x_f_{is_nan(fault_input.x_f) ? (bool)0.0 : fault_input.x_f} {
    }

    FaultCalcParam calc_param(double const& u_rated, bool const& is_connected_to_source = true) const {
        if (!energized(is_connected_to_source)) {
            return FaultCalcParam{};
        }
        // param object
        FaultCalcParam param{};
        // calculate the fault admittance in p.u.
        double const base_y = base_power_3p / u_rated / u_rated;
        DoubleComplex y_f = 1.0 / (r_f_ + 1.0i * x_f_) / base_y;
        param.y_fault = y_f;
        return param;
    }

    FaultOutput get_null_output() const {
        FaultOutput output{};
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }
    FaultOutput get_output() const {
        // During power flow and state estimation the fault object will have an empty output
        return get_null_output();
    }

    // energized
    template <bool sym>
    FaultShortCircuitOutput<sym> get_short_circuit_output(ComplexValue<sym> i_f, double const u_rated) {
        // translate pu to A
        double const base_i = base_power_3p / u_rated / sqrt3;
        i_f = i_f * base_i;
        // result object
        FaultShortCircuitOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(true);
        // calculate current magnitude and angle
        output.i_f = cabs(i_f);
        output.i_f_angle = arg(i_f);
        return output;
    }

    // update faulted object
    UpdateChange update(FaultUpdate const& update) {
        assert(update.id == id());
        if (update.fault_object != na_IntS) {
            fault_object_ = update.fault_object;
        }
        return {false, false};  // topology and parameters do not change
    }

    bool energized(bool is_connected_to_source) const final {
        return is_connected_to_source;
    }

    // getter
    ID get_fault_object() {
        return fault_object_;
    }

   private:
    // short circuit parameters
    ID fault_object_;
    double r_f_;
    double x_f_;
};

}  // namespace power_grid_model

#endif