// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_LINE_HPP
#define POWER_GRID_MODEL_COMPONENT_LINE_HPP

#include "branch.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

class Line final : public Branch {
   public:
    using InputType = LineInput;
    using UpdateType = BranchUpdate;
    static constexpr char const* name = "line";

    explicit Line(LineInput const& line_input, double system_frequency, double u1, double u2)
        : Branch{line_input}, i_n_{line_input.i_n}, base_i_{base_power_3p / u1 / sqrt3} {
        if (cabs(u1 - u2) > numerical_tolerance) {
            throw ConflictVoltage{id(), from_node(), to_node(), u1, u2};
        }
        double const base_y = base_i_ / (u1 / sqrt3);
        y1_series_ = 1.0 / (line_input.r1 + 1.0i * line_input.x1) / base_y;
        y1_shunt_ = 2.0 * pi * system_frequency * line_input.c1 / base_y * (line_input.tan1 + 1.0i);
        y0_series_ = 1.0 / (line_input.r0 + 1.0i * line_input.x0) / base_y;
        y0_shunt_ = 2.0 * pi * system_frequency * line_input.c0 / base_y * (line_input.tan0 + 1.0i);
    }

    // override getter
    double base_i_from() const final {
        return base_i_;
    }
    double base_i_to() const final {
        return base_i_;
    }
    double loading(double, double max_i) const final {
        return max_i / i_n_;
    };
    double phase_shift() const final {
        return 0.0;
    }
    bool is_param_mutable() const final {
        return false;
    }

   private:
    double i_n_;
    double base_i_;
    DoubleComplex y1_series_;
    DoubleComplex y1_shunt_;
    DoubleComplex y0_series_;
    DoubleComplex y0_shunt_;

    BranchCalcParam<true> sym_calc_param() const final {
        return calc_param_y_sym(y1_series_, y1_shunt_, 1.0);
    }
    BranchCalcParam<false> asym_calc_param() const final {
        return calc_param_y_asym(y1_series_, y1_shunt_, y0_series_, y0_shunt_, 1.0);
    }
};

}  // namespace power_grid_model

#endif
