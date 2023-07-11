// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_SENSOR_HPP
#define POWER_GRID_MODEL_COMPONENT_SENSOR_HPP

#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"

namespace power_grid_model {

class Sensor : public Base {
   public:
    static constexpr char const* name = "sensor";
    using InputType = SensorInput;
    using ShortCircuitOutputType = SensorShortCircuitOutput;

    // constructor
    explicit Sensor(SensorInput const& sensor_input)
        : Base{sensor_input}, measured_object_{sensor_input.measured_object} {
    }

    ID measured_object() const {
        return measured_object_;
    };

    // sensor always energized
    bool energized(bool) const final {
        return true;
    }
    ComponentType math_model_type() const final {
        return ComponentType::sensor;
    }

    // getter for calculation param
    template <bool sym>
    SensorCalcParam<sym> calc_param() const {
        if constexpr (sym) {
            return sym_calc_param();
        }
        else {
            return asym_calc_param();
        }
    }

   private:
    ID measured_object_;

    // virtual function getter for sym and asym param
    // override them in real sensors function
    virtual SensorCalcParam<true> sym_calc_param() const = 0;
    virtual SensorCalcParam<false> asym_calc_param() const = 0;
};

}  // namespace power_grid_model

#endif
