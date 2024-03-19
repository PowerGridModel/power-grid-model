// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base.hpp"

#include "../auxiliary/input.hpp"

namespace power_grid_model {

class Regulator : public Base {

  public:
    static constexpr char const* name = "regulator";

    ID regulated_object() const { return regulated_object_; };

    // regulator always energized
    bool energized(bool /* is_connected_to_source */) const final { return true; }
    ComponentType math_model_type() const final { return ComponentType::regulator; }

  protected:
    // constructor
    explicit Regulator(RegulatorInput const& regulator_input)
        : Base{regulator_input}, regulated_object_{regulator_input.regulated_object} {}
    Regulator(Regulator const&) = default;
    Regulator(Regulator&&) = default;
    Regulator& operator=(Regulator const&) = default;
    Regulator& operator=(Regulator&&) = default;

  public:
    ~Regulator() override = default;

  private:
    ID regulated_object_;
};

} // namespace power_grid_model