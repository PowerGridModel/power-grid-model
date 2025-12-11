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
#include <iostream>

namespace power_grid_model {

class VoltageRegulator : public Regulator {
  public:
    using InputType = VoltageRegulatorInput;
    using UpdateType = RegulatorUpdate;
    template <symmetry_tag sym> using OutputType = VoltageRegulatorOutput<sym>;

    static constexpr char const* name = "voltage_regulator";
    explicit VoltageRegulator(VoltageRegulatorInput const& voltage_regulator_input,
                           ComponentType regulated_object_type,
                            double injection_direction)
        : Regulator{voltage_regulator_input, regulated_object_type},
          injection_direction_{injection_direction},
          u_ref_{voltage_regulator_input.u_ref} {}

    // update for transformer tap regulator, hide default update for branch
    UpdateChange update(UpdateType const& update_data) {
        assert(update_data.id == this->id() || is_nan(update_data.id));
        set_status(update_data.status);
        return {.topo = false, .param = false};
    }

    UpdateType inverse(UpdateType update_data) const {
        assert(update_data.id == this->id() || is_nan(update_data.id));

        update_data = Regulator::inverse(update_data);
        return update_data;
    }

    constexpr RegulatorShortCircuitOutput get_null_sc_output() const { return {.id = id(), .energized = 0}; }

    template <symmetry_tag sym>
    constexpr VoltageRegulatorOutput<sym> get_null_output() const {
        return {.id = id(), .energized = 0, .limit_violated = 0, .q = RealValue<sym>{0}};
    }

    bool is_energized(bool is_connected_to_source = true) const {
        return is_connected_to_source && status();
    }

    template <symmetry_tag sym>
    VoltageRegulatorOutput<sym> get_output(VoltageRegulatorSolverOutput<sym> const& solver_output) const {
        VoltageRegulatorOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(is_energized(true) && solver_output.generator_status != 0);
        output.limit_violated = solver_output.limit_violated;
        output.q = base_power<sym> * solver_output.q * injection_direction();
        return output;
    }

    template <symmetry_tag sym>
    VoltageRegulatorCalcParam<sym> calc_param() const {
        return VoltageRegulatorCalcParam<sym>{
            .status = static_cast<IntS>(status()),
            .u_ref = {u_ref_, 0.0},
            .q_min = RealValue<sym>{q_min_}, // TODO: #185 divide by 3 for asymmetric case?
            .q_max = RealValue<sym>{q_max_}, // TODO: #185 divide by base_power?
            .generator_id = this->regulated_object()
        };
    }

    // getter
    double injection_direction() const { return injection_direction_; }
    double u_ref() const { return u_ref_; }
    double q_min() const { return q_min_; }
    double q_max() const { return q_max_; }

  private:
    double injection_direction_;
    double u_ref_;
    double q_min_;
    double q_max_;
};

} // namespace power_grid_model
