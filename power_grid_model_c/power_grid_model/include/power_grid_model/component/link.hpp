// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_LINK_HPP
#define POWER_GRID_MODEL_COMPONENT_LINK_HPP

#include "branch.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

class Link final : public Branch {
   public:
    using InputType = LinkInput;
    using UpdateType = BranchUpdate;
    static constexpr char const* name = "link";

    explicit Link(LinkInput const& link_input, double u1, double u2)
        : Branch{link_input}, base_i_from_{base_power_3p / u1 / sqrt3}, base_i_to_{base_power_3p / u2 / sqrt3} {
    }

    // override getter
    double base_i_from() const final {
        return base_i_from_;
    }
    double base_i_to() const final {
        return base_i_to_;
    }
    double loading(double, double) const final {
        return 0.0;
    };
    double phase_shift() const final {
        return 0.0;
    }
    bool is_param_mutable() const final {
        return false;
    }

   private:
    double base_i_from_;
    double base_i_to_;
    BranchCalcParam<true> sym_calc_param() const final {
        return calc_param_y_sym(y_link, 0.0, 1.0);
    }
    BranchCalcParam<false> asym_calc_param() const final {
        return calc_param_y_asym(y_link, 0.0, y_link, 0.0, 1.0);
    }
};
}  // namespace power_grid_model

#endif
