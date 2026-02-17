// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "regulator.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/enum.hpp"
#include "../common/three_phase_tensor.hpp"
#include "component.hpp"

#include <cassert>

namespace power_grid_model {

class VoltageRegulator : public Regulator {
  public:
    using InputType = VoltageRegulatorInput;
    using UpdateType = VoltageRegulatorUpdate;
    template <symmetry_tag sym> using OutputType = VoltageRegulatorOutput;

    static constexpr char const* name = "voltage_regulator";
    explicit VoltageRegulator(VoltageRegulatorInput const& voltage_regulator_input, ComponentType regulated_object_type)
        : Regulator{voltage_regulator_input, regulated_object_type},
          u_ref_{voltage_regulator_input.u_ref},
          q_min_{voltage_regulator_input.q_min},
          q_max_{voltage_regulator_input.q_max} {}

    UpdateChange update(VoltageRegulatorUpdate const& update_data) {
        assert(update_data.id == this->id() || is_nan(update_data.id));
        set_status(update_data.status);
        set_u_ref(update_data.u_ref);
        set_q_limits(update_data.q_min, update_data.q_max);
        return {.topo = false, .param = false};
    }

    VoltageRegulatorUpdate inverse(VoltageRegulatorUpdate update_data) const {
        assert(update_data.id == this->id() || is_nan(update_data.id));

        update_data = Regulator::inverse(update_data);
        set_if_not_nan(update_data.u_ref, u_ref_);
        set_if_not_nan(update_data.q_min, q_min_);
        set_if_not_nan(update_data.q_max, q_max_);
        return update_data;
    }

    constexpr RegulatorShortCircuitOutput get_null_sc_output() const { return {.id = id(), .energized = 0}; }

    constexpr VoltageRegulatorOutput get_null_output() const {
        return {.id = id(), .energized = 0, .limit_violated = 0};
    }

    constexpr bool is_energized(bool is_connected_to_source = true) const { return is_connected_to_source && status(); }

    VoltageRegulatorOutput get_output(VoltageRegulatorSolverOutput const& solver_output) const {
        VoltageRegulatorOutput output{};
        static_cast<BaseOutput&>(output) = base_output(is_energized(true) && solver_output.generator_status != 0);
        output.limit_violated = solver_output.limit_violated;
        return output;
    }

    template <symmetry_tag sym> VoltageRegulatorCalcParam<sym> calc_param() const {
        return VoltageRegulatorCalcParam<sym>{.status = static_cast<IntS>(status()),
                                              .u_ref = {u_ref_, 0.0},
                                              .q_min = RealValue<sym>{q_min_ / base_power_3p},
                                              .q_max = RealValue<sym>{q_max_ / base_power_3p},
                                              .generator_id = this->regulated_object()};
    }

    // setter
    void set_u_ref(double new_u_ref) {
        if (!is_nan(new_u_ref)) {
            u_ref_ = new_u_ref;
        }
    }

    void set_q_limits(double new_q_min, double new_q_max) {
        if (!is_nan(new_q_min)) {
            q_min_ = new_q_min;
        }
        if (!is_nan(new_q_max)) {
            q_max_ = new_q_max;
        }
    }

    // getter
    constexpr double u_ref() const { return u_ref_; }
    constexpr double q_min() const { return q_min_; }
    constexpr double q_max() const { return q_max_; }

  private:
    double u_ref_;
    double q_min_;
    double q_max_;
};

} // namespace power_grid_model
