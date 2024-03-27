// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "branch.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"

namespace power_grid_model {

class Link final : public Branch {
  public:
    using InputType = LinkInput;
    using UpdateType = BranchUpdate;
    static constexpr char const* name = "link";

    explicit Link(LinkInput const& link_input, double u1, double u2)
        : Branch{link_input}, base_i_from_{base_power_3p / u1 / sqrt3}, base_i_to_{base_power_3p / u2 / sqrt3} {}

    // override getter
    double base_i_from() const override { return base_i_from_; }
    double base_i_to() const override { return base_i_to_; }
    double loading(double /* max_s */, double /* max_i */) const override { return 0.0; };
    double phase_shift() const override { return 0.0; }
    bool is_param_mutable() const override { return false; }

  private:
    double base_i_from_;
    double base_i_to_;
    BranchCalcParam<symmetric_t> sym_calc_param() const override { return calc_param_y_sym(y_link, 0.0, 1.0); }
    BranchCalcParam<asymmetric_t> asym_calc_param() const override {
        return calc_param_y_asym(y_link, 0.0, y_link, 0.0, 1.0);
    }
};
} // namespace power_grid_model
