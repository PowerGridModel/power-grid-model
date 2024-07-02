// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "component.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../common/common.hpp"
#include "../common/enum.hpp"

namespace power_grid_model {

class Base {
  public:
    using InputType = BaseInput;
    using UpdateType = BaseUpdate;
    template <symmetry_tag sym> using OutputType = BaseOutput;
    using ShortCircuitOutputType = BaseOutput;

    static constexpr char const* name = "base";
    virtual ComponentType math_model_type() const = 0;

    explicit Base(BaseInput const& base_input) : id_{base_input.id} {}
    virtual ~Base() = default;
    constexpr ID id() const noexcept { return id_; }
    constexpr BaseOutput base_output(bool is_energized) const {
        return BaseOutput{id_, static_cast<IntS>(is_energized)};
    }
    virtual bool energized(bool is_connected_to_source) const = 0;

    Base(Base&&) = default;
    Base& operator=(Base&&) = default;

  protected:
    Base(const Base&) = default;
    Base& operator=(const Base&) = default;

  private:
    ID id_;
};

} // namespace power_grid_model
