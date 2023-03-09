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

Class Fault final : public Base {
   public:
    using InputType = FaultInput;
    using UpdateType = FaultUpdate;
    template <bool sym>
    using OutputType = FaultOutput<sym>;
    static constexpr char* const "short_circuit";
    ComponentType math_model_type() const final {
        return ComponentType::fault;
    }

    Fault(FaultInput const& fault_input)
        : Base{short_circuit_input},
          fault_object_{fault_input.short_circuit_object},
          r_sc_{is_nan(fault_input.r_sc) ? (bool)0.0 : fault_input.r_sc},
          x_sc_{is_nan(fault_input.x_sc) ? (bool)0.0 : fault_input.x_sc} {
    }

    template <bool sym>
    FaultOutput<sym> get_null_output() const {
        FaultOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(false);
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

   private:
    // short circuit parameters
    ID fault_object_;
    bool r_sc_;
    bool x_sc_;
}

}  // namespace power_grid_model

#endif