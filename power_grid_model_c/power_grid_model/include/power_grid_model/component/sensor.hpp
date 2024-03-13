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
#include "../common/exception.hpp"

namespace power_grid_model {

class Sensor : public Base {
  public:
    static constexpr char const* name = "sensor";
    using InputType = SensorInput;
    using ShortCircuitOutputType = SensorShortCircuitOutput;

    ID measured_object() const { return measured_object_; };

    // sensor always energized
    bool energized(bool /* is_connected_to_source */) const final { return true; }
    ComponentType math_model_type() const final { return ComponentType::sensor; }

  protected:
    // constructor
    explicit Sensor(SensorInput const& sensor_input)
        : Base{sensor_input}, measured_object_{sensor_input.measured_object} {}
    Sensor(Sensor const&) = default;
    Sensor(Sensor&&) = default;
    Sensor& operator=(Sensor const&) = default;
    Sensor& operator=(Sensor&&) = default;

  public:
    ~Sensor() override = default;

  private:
    ID measured_object_;
};

} // namespace power_grid_model
