// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"

namespace power_grid_model {

class Fault final : public Base {
  public:
    using InputType = FaultInput;
    using UpdateType = FaultUpdate;
    template <symmetry_tag sym> using OutputType = FaultOutput;
    using ShortCircuitOutputType = FaultShortCircuitOutput;
    static constexpr char const* name = "fault";
    ComponentType math_model_type() const override { return ComponentType::fault; }

    explicit Fault(FaultInput const& fault_input)
        : Base{fault_input},
          status_{static_cast<bool>(fault_input.status)},
          fault_type_{fault_input.fault_type},
          fault_phase_{fault_input.fault_phase == FaultPhase::nan ? FaultPhase::default_value
                                                                  : fault_input.fault_phase},
          fault_object_{fault_input.fault_object},
          r_f_{is_nan(fault_input.r_f) ? double{} : fault_input.r_f},
          x_f_{is_nan(fault_input.x_f) ? double{} : fault_input.x_f} {
        check_sanity();
    }

    FaultCalcParam calc_param(double const u_rated, bool const& is_connected_to_source = true) const {
        // param object
        FaultCalcParam param{};
        param.fault_type = get_fault_type();
        param.fault_phase = get_fault_phase();
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

    FaultShortCircuitOutput get_null_sc_output() const {
        FaultShortCircuitOutput output{.i_f = {}, .i_f_angle = {}};
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }
    FaultShortCircuitOutput get_sc_output(ComplexValue<asymmetric_t> i_f, double const u_rated) const {
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

    FaultShortCircuitOutput get_sc_output(ComplexValue<symmetric_t> i_f, double const u_rated) const {
        ComplexValue<asymmetric_t> const iabc_f{i_f};
        return get_sc_output(iabc_f, u_rated);
    }

    template <symmetry_tag sym>
    FaultShortCircuitOutput get_sc_output(FaultShortCircuitSolverOutput<sym> const& solver_output,
                                          double const u_rated) const {
        return get_sc_output(solver_output.i_fault, u_rated);
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
        if (!is_nan(update.r_f)) {
            r_f_ = update.r_f;
        }
        if (!is_nan(update.x_f)) {
            x_f_ = update.x_f;
        }
        check_sanity();
        return {false, false}; // topology and parameters do not change
    }

    FaultUpdate inverse(FaultUpdate update_data) const {
        assert(update_data.id == id());

        set_if_not_nan(update_data.status, static_cast<IntS>(status_));
        set_if_not_nan(update_data.fault_type, fault_type_);
        set_if_not_nan(update_data.fault_phase, fault_phase_);
        set_if_not_nan(update_data.fault_object, fault_object_);
        set_if_not_nan(update_data.r_f, r_f_);
        set_if_not_nan(update_data.x_f, x_f_);

        return update_data;
    }

    constexpr bool energized(bool is_connected_to_source) const override { return is_connected_to_source; }

    constexpr bool status() const { return status_; }

    // setter
    constexpr bool set_status(IntS new_status) {
        if (new_status == na_IntS) {
            return false;
        }
        if (static_cast<bool>(new_status) == status_) {
            return false;
        }
        status_ = static_cast<bool>(new_status);
        return true;
    }

    // getters
    FaultType get_fault_type() const {
        using enum FaultType;

        constexpr auto supported = std::array{three_phase, single_phase_to_ground, two_phase, two_phase_to_ground};

        if (std::ranges::find(supported, fault_type_) == cend(supported)) {
            throw InvalidShortCircuitType(fault_type_);
        }

        return fault_type_;
    }
    FaultPhase get_fault_phase() const {
        using enum FaultType;

        if (fault_phase_ == FaultPhase::default_value) {
            auto const default_phase = [](FaultType fault_type) {
                switch (fault_type) {
                    using enum FaultPhase;

                case three_phase:
                    return abc;
                case single_phase_to_ground:
                    return a;
                case two_phase:
                    [[fallthrough]];
                case two_phase_to_ground:
                    return bc;
                default:
                    throw InvalidShortCircuitType(fault_type);
                }
            }(fault_type_);
            return default_phase;
        }
        return fault_phase_;
    }
    constexpr ID get_fault_object() const { return fault_object_; }

  private:
    // short circuit parameters
    bool status_;
    FaultType fault_type_;
    FaultPhase fault_phase_;
    ID fault_object_;
    double r_f_;
    double x_f_;

    void check_sanity() {
        using enum FaultPhase;

        auto const check_supported = [&](auto const& iterable) {
            if (std::ranges::find(iterable, fault_phase_) == cend(iterable)) {
                throw InvalidShortCircuitPhases(fault_type_, fault_phase_);
            }
        };
        switch (fault_type_) {
        case FaultType::three_phase:
            return check_supported(std::array{FaultPhase::nan, default_value, abc});
        case FaultType::single_phase_to_ground:
            return check_supported(std::array{FaultPhase::nan, default_value, a, b, c});
        case FaultType::two_phase:
            [[fallthrough]];
        case FaultType::two_phase_to_ground:
            return check_supported(std::array{FaultPhase::nan, default_value, ab, ac, bc});
        case FaultType::nan:
            return check_supported(std::array{FaultPhase::nan, default_value, abc, a, b, c, ab, ac, bc});
        default:
            throw InvalidShortCircuitType(fault_type_);
        }
    }
};

} // namespace power_grid_model
