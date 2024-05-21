// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/exception.hpp"

namespace power_grid_model {

class Branch3 : public Base {
  public:
    using InputType = Branch3Input;
    using UpdateType = Branch3Update;
    template <symmetry_tag sym> using OutputType = Branch3Output<sym>;
    using ShortCircuitOutputType = Branch3ShortCircuitOutput;
    using SideType = Branch3Side;

    static constexpr char const* name = "branch3";
    ComponentType math_model_type() const final { return ComponentType::branch3; }

    explicit Branch3(Branch3Input const& branch3_input)
        : Base{branch3_input},
          node_1_{branch3_input.node_1},
          node_2_{branch3_input.node_2},
          node_3_{branch3_input.node_3},
          status_1_{static_cast<bool>(branch3_input.status_1)},
          status_2_{static_cast<bool>(branch3_input.status_2)},
          status_3_{static_cast<bool>(branch3_input.status_3)} {
        if (node_1_ == node_2_ || node_1_ == node_3_ || node_2_ == node_3_) {
            throw InvalidBranch3{id(), node_1_, node_2_, node_3_};
        }
    }

    // getter
    constexpr ID node_1() const { return node_1_; }
    constexpr ID node_2() const { return node_2_; }
    constexpr ID node_3() const { return node_3_; }
    constexpr ID node(Branch3Side side) const {
        using enum Branch3Side;

        switch (side) {
        case side_1:
            return node_1();
        case side_2:
            return node_2();
        case side_3:
            return node_3();
        default:
            throw MissingCaseForEnumError{"node(Branch3Side)", side};
        }
    }
    constexpr bool status_1() const { return status_1_; }
    constexpr bool status_2() const { return status_2_; }
    constexpr bool status_3() const { return status_3_; }
    constexpr bool branch3_status() const {
        return status_1() && status_2() && status_3(); // TODO: check if this makes sense for branch3
    }
    constexpr bool status(Branch3Side side) const {
        using enum Branch3Side;

        switch (side) {
        case side_1:
            return status_1();
        case side_2:
            return status_2();
        case side_3:
            return status_3();
        default:
            throw MissingCaseForEnumError{"status(Branch3Side)", side};
        }
    }

    // virtual getter
    constexpr bool energized(bool is_connected_to_source = true) const final {
        return is_connected_to_source && (status_1_ || status_2_ || status_3_);
    }
    virtual double base_i_1() const = 0;
    virtual double base_i_2() const = 0;
    virtual double base_i_3() const = 0;
    virtual double loading(double s_1, double s_2, double s_3) const = 0;
    virtual std::array<double, 3> phase_shift() const = 0;

    template <symmetry_tag sym>
    std::array<BranchCalcParam<sym>, 3> calc_param(bool is_connected_to_source = true) const {
        if (!energized(is_connected_to_source)) {
            return std::array<BranchCalcParam<sym>, 3>{};
        }
        if constexpr (is_symmetric_v<sym>) {
            return sym_calc_param();
        } else {
            return asym_calc_param();
        }
    }

    template <symmetry_tag sym>
    Branch3Output<sym> get_output(BranchSolverOutput<sym> const& branch_solver_output1,
                                  BranchSolverOutput<sym> const& branch_solver_output2,
                                  BranchSolverOutput<sym> const& branch_solver_output3) const {
        // result object
        Branch3Output<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(true);
        // calculate result
        output.p_1 = base_power<sym> * real(branch_solver_output1.s_f);
        output.q_1 = base_power<sym> * imag(branch_solver_output1.s_f);
        output.i_1 = base_i_1() * cabs(branch_solver_output1.i_f);
        output.s_1 = base_power<sym> * cabs(branch_solver_output1.s_f);

        output.p_2 = base_power<sym> * real(branch_solver_output2.s_f);
        output.q_2 = base_power<sym> * imag(branch_solver_output2.s_f);
        output.i_2 = base_i_2() * cabs(branch_solver_output2.i_f);
        output.s_2 = base_power<sym> * cabs(branch_solver_output2.s_f);

        output.p_3 = base_power<sym> * real(branch_solver_output3.s_f);
        output.q_3 = base_power<sym> * imag(branch_solver_output3.s_f);
        output.i_3 = base_i_3() * cabs(branch_solver_output3.i_f);
        output.s_3 = base_power<sym> * cabs(branch_solver_output3.s_f);

        output.loading = loading(sum_val(output.s_1), sum_val(output.s_2), sum_val(output.s_3));

        return output;
    }

    Branch3ShortCircuitOutput get_sc_output(ComplexValue<asymmetric_t> const& i_1,
                                            ComplexValue<asymmetric_t> const& i_2,
                                            ComplexValue<asymmetric_t> const& i_3) const {
        // result object
        Branch3ShortCircuitOutput output{};
        static_cast<BaseOutput&>(output) = base_output(true);
        // calculate result
        output.i_1 = base_i_1() * cabs(i_1);
        output.i_2 = base_i_2() * cabs(i_2);
        output.i_3 = base_i_3() * cabs(i_3);
        output.i_1_angle = arg(i_1);
        output.i_2_angle = arg(i_2);
        output.i_3_angle = arg(i_3);
        return output;
    }
    Branch3ShortCircuitOutput get_sc_output(ComplexValue<symmetric_t> const& i_1, ComplexValue<symmetric_t> const& i_2,
                                            ComplexValue<symmetric_t> const& i_3) const {
        ComplexValue<asymmetric_t> const iabc_1{i_1};
        ComplexValue<asymmetric_t> const iabc_2{i_2};
        ComplexValue<asymmetric_t> const iabc_3{i_3};
        return get_sc_output(iabc_1, iabc_2, iabc_3);
    }
    template <symmetry_tag sym>
    Branch3ShortCircuitOutput get_sc_output(BranchShortCircuitSolverOutput<sym> const& branch_solver_output1,
                                            BranchShortCircuitSolverOutput<sym> const& branch_solver_output2,
                                            BranchShortCircuitSolverOutput<sym> const& branch_solver_output3) const {
        return get_sc_output(branch_solver_output1.i_f, branch_solver_output2.i_f, branch_solver_output3.i_f);
    }

    template <symmetry_tag sym> Branch3Output<sym> get_null_output() const {
        Branch3Output<sym> output{
            .loading = {},
            .p_1 = {},
            .q_1 = {},
            .i_1 = {},
            .s_1 = {},
            .p_2 = {},
            .q_2 = {},
            .i_2 = {},
            .s_2 = {},
            .p_3 = {},
            .q_3 = {},
            .i_3 = {},
            .s_3 = {},
        };
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }

    Branch3ShortCircuitOutput get_null_sc_output() const {
        Branch3ShortCircuitOutput output{
            .i_1 = {}, .i_1_angle = {}, .i_2 = {}, .i_2_angle = {}, .i_3 = {}, .i_3_angle = {}};
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
            changed = changed || (status_1_ != static_cast<bool>(new_status_1));
            status_1_ = static_cast<bool>(new_status_1);
        }
        if (set_2) {
            changed = changed || (status_2_ != static_cast<bool>(new_status_2));
            status_2_ = static_cast<bool>(new_status_2);
        }
        if (set_3) {
            changed = changed || (status_3_ != static_cast<bool>(new_status_3));
            status_3_ = static_cast<bool>(new_status_3);
        }
        return changed;
    }

    // default update for branch3, will be hidden for three winding transformer
    UpdateChange update(Branch3Update const& update_data) {
        assert(update_data.id == id());
        bool const changed = set_status(update_data.status_1, update_data.status_2, update_data.status_3);
        // change in branch3 connection will change both topo and param
        return {changed, changed};
    }

    auto inverse(std::convertible_to<Branch3Update> auto update_data) const {
        assert(update_data.id == id());

        set_if_not_nan(update_data.status_1, static_cast<IntS>(status_1_));
        set_if_not_nan(update_data.status_2, static_cast<IntS>(status_2_));
        set_if_not_nan(update_data.status_3, static_cast<IntS>(status_3_));

        return update_data;
    }

  private:
    ID node_1_;
    ID node_2_;
    ID node_3_;
    bool status_1_;
    bool status_2_;
    bool status_3_;

    virtual std::array<BranchCalcParam<symmetric_t>, 3> sym_calc_param() const = 0;
    virtual std::array<BranchCalcParam<asymmetric_t>, 3> asym_calc_param() const = 0;
};

} // namespace power_grid_model
