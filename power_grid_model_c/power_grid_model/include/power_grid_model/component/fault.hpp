// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_SHORT_CIRCUIT_HPP
#define POWER_GRID_MODEL_COMPONENT_SHORT_CIRCUIT_HPP

#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../enum.hpp"

namespace power_grid_model {

class Fault final : public Base {
   public:
    using InputType = FaultInput;
    using UpdateType = FaultUpdate;
    template <bool sym>
    using OutputType = FaultOutput;
    using ShortCircuitOutputType = FaultShortCircuitOutput;
    static constexpr char const* name = "fault";
    ComponentType math_model_type() const {
        return ComponentType::fault;
    }

    explicit Fault(FaultInput const& fault_input)
        : Base{fault_input},
          status_{static_cast<bool>(fault_input.status)},
          fault_type_{fault_input.fault_type},
          fault_phase_{fault_input.fault_phase},
          fault_object_{fault_input.fault_object},
          r_f_{is_nan(fault_input.r_f) ? double{} : fault_input.r_f},
          x_f_{is_nan(fault_input.x_f) ? double{} : fault_input.x_f} {
        check_sanity();
    }

    FaultCalcParam calc_param(double const& u_rated, bool const& is_connected_to_source = true) const {
        // param object
        FaultCalcParam param{};
        if (!energized(is_connected_to_source)) {
            return param;
        }
        // set the fault admittance to inf if the impedance is zero
        if (r_f_ == 0.0 && x_f_ == 0.0) {
            param.y_fault.real(std::numeric_limits<double>::infinity());
            param.y_fault.imag(std::numeric_limits<double>::infinity());
            return param;
        }
        // calculate the fault admittance in p.u.
        double const base_z = u_rated * u_rated / base_power_3p;
        param.y_fault = base_z / (r_f_ + 1.0i * x_f_);
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
    FaultShortCircuitOutput get_sc_output(ComplexValue<false> i_f, double const u_rated) const {
        // translate pu to A
        double const base_i = base_power_3p / u_rated / sqrt3;
        i_f = i_f * base_i;
        // result object
        FaultShortCircuitOutput output{};
        static_cast<BaseOutput&>(output) = base_output(true);
        output.i_f = cabs(i_f);
        output.i_f_angle = arg(i_f);
        return output;
    }

    FaultShortCircuitOutput get_sc_output(ComplexValue<true> i_f, double const u_rated) const {
        ComplexValue<false> iabc_f{i_f};
        return get_sc_output(iabc_f, u_rated);
    }

    // update faulted object
    UpdateChange update(FaultUpdate const& update) {
        assert(update.id == id());
        set_status(update.status);
        if (update.fault_type != FaultType::nan) {
            fault_type_ = update.fault_type;
        }
        if (update.fault_phase != FaultPhase::nan) {
            fault_phase_ = update.fault_phase;
        }
        if (update.fault_object != na_IntID) {
            fault_object_ = update.fault_object;
        }
        check_sanity();
        return {false, false};  // topology and parameters do not change
    }

    bool energized(bool is_connected_to_source) const {
        return is_connected_to_source;
    }

    bool status() const {
        return status_;
    }

    // setter
    bool set_status(IntS new_status) {
        if (new_status == na_IntS)
            return false;
        if (static_cast<bool>(new_status) == status_)
            return false;
        status_ = static_cast<bool>(new_status);
        return true;
    }

    // getters
    FaultType get_fault_type() const {
        return fault_type_;
    }
    FaultPhase get_fault_phase() const {
        using enum FaultPhase;

        if (fault_phase_ == default_value || fault_phase_ == nan) {
            switch (fault_type_) {
                case FaultType::three_phase:
                    return abc;
                case FaultType::single_phase_to_ground:
                    return a;
                case FaultType::two_phase:
                    [[fallthrough]];
                case FaultType::two_phase_to_ground:
                    return bc;
                case FaultType::nan:
                    return FaultPhase::nan;
            }
        }
        return fault_phase_;
    }
    ID get_fault_object() const {
        return fault_object_;
    }

   private:
    // short circuit parameters
    bool status_;
    FaultType fault_type_;
    FaultPhase fault_phase_;
    ID fault_object_;
    double r_f_;
    double x_f_;

    void check_sanity() const {
        auto supported = [](FaultType fault_type) -> std::vector<FaultPhase> {
            switch (fault_type) {
                case FaultType::three_phase:
                    return {FaultPhase::nan, FaultPhase::default_value, FaultPhase::abc};
                case FaultType::single_phase_to_ground:
                    return {FaultPhase::nan, FaultPhase::default_value, FaultPhase::a, FaultPhase::b, FaultPhase::c};
                case FaultType::two_phase:
                    [[fallthrough]];
                case FaultType::two_phase_to_ground:
                    return {FaultPhase::nan, FaultPhase::default_value, FaultPhase::ab, FaultPhase::ac, FaultPhase::bc};
                case FaultType::nan:
                    return {FaultPhase::nan, FaultPhase::default_value,
                            FaultPhase::abc, FaultPhase::a,
                            FaultPhase::b,   FaultPhase::c,
                            FaultPhase::ab,  FaultPhase::ac,
                            FaultPhase::bc};
                default:
                    return {};
            }
        }(fault_type_);
        if (std::find(cbegin(supported), cend(supported), fault_phase_) == cend(supported)) {
            throw InvalidShortCircuitPhases(fault_type_, fault_phase_);
        }
    }
};

}  // namespace power_grid_model

#endif
