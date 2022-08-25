// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_BRANCH3_HPP
#define POWER_GRID_MODEL_COMPONENT_BRANCH3_HPP

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../calculation_parameters.hpp"
#include "base.hpp"

namespace power_grid_model {

class Branch3 : public Base {
   public:
    using InputType = Branch3Input;
    using UpdateType = Branch3Update;
    template <bool sym>
    using OutputType = Branch3Output<sym>;
    static constexpr char const* name = "branch3";
    ComponentType math_model_type() const final {
        return ComponentType::branch3;
    }

    Branch3(Branch3Input const& branch3_input)
        : Base{branch3_input},
          node_1_{branch3_input.node_1},
          node_2_{branch3_input.node_2},
          node_3_{branch3_input.node_3},
          status_1_{(bool)branch3_input.status_1},
          status_2_{(bool)branch3_input.status_2},
          status_3_{(bool)branch3_input.status_3} {
        if (node_1_ == node_2_ || node_1_ == node_3_ || node_2_ == node_3_) {
            throw InvalidBranch3{id(), node_1_, node_2_, node_3_};
        }
    }

    // getter
    ID node_1() const {
        return node_1_;
    }
    ID node_2() const {
        return node_2_;
    }
    ID node_3() const {
        return node_3_;
    }
    bool status_1() const {
        return status_1_;
    }
    bool status_2() const {
        return status_2_;
    }
    bool status_3() const {
        return status_3_;
    }
    bool branch3_status() const {
        return status_1_ && status_2_ && status_3_;  // TODO: check if this makes sense for branch3
    }

    // virtual getter
    bool energized(bool is_connected_to_source = true) const final {
        return is_connected_to_source && (status_1_ || status_2_ || status_3_);
    }
    virtual double base_i_1() const = 0;
    virtual double base_i_2() const = 0;
    virtual double base_i_3() const = 0;
    virtual double loading(double s_1, double s_2, double s_3) const = 0;
    virtual std::array<double, 3> phase_shift() const = 0;

    template <bool sym>
    std::array<BranchCalcParam<sym>, 3> calc_param(bool is_connected_to_source = true) const {
        if (!energized(is_connected_to_source)) {
            return std::array<BranchCalcParam<sym>, 3>{};
        }
        if constexpr (sym) {
            return sym_calc_param();
        }
        else {
            return asym_calc_param();
        }
    }

    template <bool sym>
    Branch3Output<sym> get_output(BranchMathOutput<sym> const& branch_math_output1,
                                  BranchMathOutput<sym> const& branch_math_output2,
                                  BranchMathOutput<sym> const& branch_math_output3) const {
        // result object
        Branch3Output<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(true);
        // calculate result
        output.p_1 = base_power<sym> * real(branch_math_output1.s_f);
        output.q_1 = base_power<sym> * imag(branch_math_output1.s_f);
        output.i_1 = base_i_1() * cabs(branch_math_output1.i_f);
        output.s_1 = base_power<sym> * cabs(branch_math_output1.s_f);

        output.p_2 = base_power<sym> * real(branch_math_output2.s_f);
        output.q_2 = base_power<sym> * imag(branch_math_output2.s_f);
        output.i_2 = base_i_2() * cabs(branch_math_output2.i_f);
        output.s_2 = base_power<sym> * cabs(branch_math_output2.s_f);

        output.p_3 = base_power<sym> * real(branch_math_output3.s_f);
        output.q_3 = base_power<sym> * imag(branch_math_output3.s_f);
        output.i_3 = base_i_3() * cabs(branch_math_output3.i_f);
        output.s_3 = base_power<sym> * cabs(branch_math_output3.s_f);

        output.loading = loading(sum_val(output.s_1), sum_val(output.s_2), sum_val(output.s_3));

        return output;
    }

    template <bool sym>
    Branch3Output<sym> get_null_output() const {
        Branch3Output<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }

    // setter
    bool set_status(IntS new_status_1, IntS new_status_2, IntS new_status_3) {
        bool const set_1 = new_status_1 != na_IntS;
        bool const set_2 = new_status_2 != na_IntS;
        bool const set_3 = new_status_3 != na_IntS;
        bool changed = false;
        if (set_1) {
            changed = changed || (status_1_ != (bool)new_status_1);
            status_1_ = (bool)new_status_1;
        }
        if (set_2) {
            changed = changed || (status_2_ != (bool)new_status_2);
            status_2_ = (bool)new_status_2;
        }
        if (set_3) {
            changed = changed || (status_3_ != (bool)new_status_3);
            status_3_ = (bool)new_status_3;
        }
        return changed;
    }

    // default update for branch3, will be hidden for three winding transformer
    UpdateChange update(Branch3Update const& update) {
        assert(update.id == id());
        bool const changed = set_status(update.status_1, update.status_2, update.status_3);
        // change in branch3 connection will change both topo and param
        return {changed, changed};
    }

   private:
    ID node_1_;
    ID node_2_;
    ID node_3_;
    bool status_1_;
    bool status_2_;
    bool status_3_;

    virtual std::array<BranchCalcParam<true>, 3> sym_calc_param() const = 0;
    virtual std::array<BranchCalcParam<false>, 3> asym_calc_param() const = 0;
};

}  // namespace power_grid_model

#endif