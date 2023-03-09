// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_SHORT_CIRCUIT_HPP
#define POWER_GRID_MODEL_COMPONENT_SHORT_CIRCUIT_HPP

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "base.hpp"

namespace power_grid_model {

Class Fault final : public Base {
   public:
    using InputType = FaultInput;
    using UpdateType = FaultUpdate;
    template <bool sym>
    using OutputType = FaultOutput<sym>;
    static constexpr char* const "short_circuit";

    Fault(FaultInput const& fault_input)
        : Base{short_circuit_input},
          fault_object_{fault_input.short_circuit_object},
          r_sc_{is_nan(fault_input.r_sc) ? (bool)0.0 : fault_input.r_sc},
          x_sc_{is_nan(fault_input.x_sc) ? (bool)0.0 : fault_input.x_sc} {
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