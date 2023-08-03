// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_BRANCH_HPP
#define POWER_GRID_MODEL_COMPONENT_BRANCH_HPP

#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

class Branch : public Base {
   public:
    using InputType = BranchInput;
    using UpdateType = BranchUpdate;
    template <bool sym>
    using OutputType = BranchOutput<sym>;
    using ShortCircuitOutputType = BranchShortCircuitOutput;
    static constexpr char const* name = "branch";
    ComponentType math_model_type() const final {
        return ComponentType::branch;
    }

    explicit Branch(BranchInput const& branch_input)
        : Base{branch_input},
          from_node_{branch_input.from_node},
          to_node_{branch_input.to_node},
          from_status_{static_cast<bool>(branch_input.from_status)},
          to_status_{static_cast<bool>(branch_input.to_status)} {
        if (from_node_ == to_node_) {
            throw InvalidBranch{id(), from_node_};
        }
    }

    // getter
    ID from_node() const {
        return from_node_;
    }
    ID to_node() const {
        return to_node_;
    }
    bool from_status() const {
        return from_status_;
    }
    bool to_status() const {
        return to_status_;
    }
    bool branch_status() const {
        return from_status_ && to_status_;
    }
    template <bool sym>
    BranchCalcParam<sym> calc_param(bool is_connected_to_source = true) const {
        if (!energized(is_connected_to_source)) {
            return BranchCalcParam<sym>{};
        }
        if constexpr (sym) {
            return sym_calc_param();
        }
        else {
            return asym_calc_param();
        }
    }

    // virtual getter
    bool energized(bool is_connected_to_source) const final {
        return is_connected_to_source && (from_status_ || to_status_);
    }
    virtual double base_i_from() const = 0;
    virtual double base_i_to() const = 0;
    virtual double loading(double max_s, double max_i) const = 0;
    virtual double phase_shift() const = 0;  // shift theta_from - theta_to
    virtual bool is_param_mutable() const = 0;

    template <bool sym>
    BranchOutput<sym> get_output(ComplexValue<sym> const& u_f, ComplexValue<sym> const& u_t) const {
        // calculate flow
        BranchCalcParam<sym> const param = calc_param<sym>();
        BranchMathOutput<sym> branch_math_output{};
        branch_math_output.i_f = dot(param.yff(), u_f) + dot(param.yft(), u_t);
        branch_math_output.i_t = dot(param.ytf(), u_f) + dot(param.ytt(), u_t);
        branch_math_output.s_f = u_f * conj(branch_math_output.i_f);
        branch_math_output.s_t = u_t * conj(branch_math_output.i_t);
        // calculate result
        return get_output<sym>(branch_math_output);
    }

    template <bool sym>
    BranchOutput<sym> get_output(BranchMathOutput<sym> const& branch_math_output) const {
        // result object
        BranchOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(true);
        // calculate result
        output.p_from = base_power<sym> * real(branch_math_output.s_f);
        output.q_from = base_power<sym> * imag(branch_math_output.s_f);
        output.i_from = base_i_from() * cabs(branch_math_output.i_f);
        output.s_from = base_power<sym> * cabs(branch_math_output.s_f);
        output.p_to = base_power<sym> * real(branch_math_output.s_t);
        output.q_to = base_power<sym> * imag(branch_math_output.s_t);
        output.i_to = base_i_to() * cabs(branch_math_output.i_t);
        output.s_to = base_power<sym> * cabs(branch_math_output.s_t);
        double const max_s = std::max(sum_val(output.s_from), sum_val(output.s_to));
        double const max_i = std::max(max_val(output.i_from), max_val(output.i_to));
        output.loading = loading(max_s, max_i);
        return output;
    }

    BranchShortCircuitOutput get_sc_output(ComplexValue<true> const& i_f, ComplexValue<true> const& i_t) const {
        return get_sc_output(BranchShortCircuitMathOutput<true>{.i_f = i_f, .i_t = i_t});
    }
    BranchShortCircuitOutput get_sc_output(ComplexValue<false> const& i_f, ComplexValue<false> const& i_t) const {
        return get_sc_output(BranchShortCircuitMathOutput<false>{.i_f = i_f, .i_t = i_t});
    }

    BranchShortCircuitOutput get_sc_output(BranchShortCircuitMathOutput<false> const& branch_math_output) const {
        BranchShortCircuitOutput output{};
        static_cast<BaseOutput&>(output) = base_output(true);
        // calculate result
        output.i_from = base_i_from() * cabs(branch_math_output.i_f);
        output.i_to = base_i_to() * cabs(branch_math_output.i_t);
        output.i_from_angle = arg(branch_math_output.i_f);
        output.i_to_angle = arg(branch_math_output.i_t);
        return output;
    }

    BranchShortCircuitOutput get_sc_output(BranchShortCircuitMathOutput<true> const& branch_math_output) const {
        return get_sc_output(BranchShortCircuitMathOutput<false>{.i_f = ComplexValue<false>{branch_math_output.i_f},
                                                                 .i_t = ComplexValue<false>{branch_math_output.i_t}});
    }

    template <bool sym>
    BranchOutput<sym> get_null_output() const {
        BranchOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }

    BranchShortCircuitOutput get_null_sc_output() const {
        BranchShortCircuitOutput output{};
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }

    // setter
    bool set_status(IntS new_from_status, IntS new_to_status) {
        bool const set_from = new_from_status != na_IntS;
        bool const set_to = new_to_status != na_IntS;
        bool changed = false;
        if (set_from) {
            changed = changed || (from_status_ != static_cast<bool>(new_from_status));
            from_status_ = static_cast<bool>(new_from_status);
        }
        if (set_to) {
            changed = changed || (to_status_ != static_cast<bool>(new_to_status));
            to_status_ = static_cast<bool>(new_to_status);
        }
        return changed;
    }

    // default update for branch, will be hidden for transformer
    UpdateChange update(BranchUpdate const& update) {
        assert(update.id == id());
        bool const changed = set_status(update.from_status, update.to_status);
        // change branch connection will change both topo and param
        return {changed, changed};
    }

   protected:
    // calculate branch param based on symmetric component
    BranchCalcParam<true> calc_param_y_sym(
        DoubleComplex const& y_series,  // y_series must be converted to the "to" side of the branch
        DoubleComplex const& y_shunt,   // y_shunt must be converted to the "to" side of the branch
        DoubleComplex const& tap_ratio) const {
        double const tap = cabs(tap_ratio);
        BranchCalcParam<true> param{};
        // not both connected
        if (!(from_status_ && to_status_)) {
            // single connected
            if (from_status_ || to_status_) {
                DoubleComplex branch_shunt;
                // shunt value
                if (cabs(y_shunt) < numerical_tolerance) {
                    branch_shunt = 0.0;
                }
                else {
                    // branch_shunt = y_shunt/2 + 1/(1/y_series + 2/y_shunt)
                    branch_shunt = 0.5 * y_shunt + 1.0 / (1.0 / y_series + 2.0 / y_shunt);
                }
                // from or to connected
                param.yff() = from_status_ ? (1.0 / tap / tap) * branch_shunt : 0.0;
                param.ytt() = to_status_ ? branch_shunt : 0.0;
            }
        }
        // both connected
        else {
            param.ytt() = y_series + 0.5 * y_shunt;
            param.yff() = (1.0 / tap / tap) * param.ytt();
            param.yft() = (-1.0 / conj(tap_ratio)) * y_series;
            param.ytf() = (-1.0 / tap_ratio) * y_series;
        }
        return param;
    }
    // calculate branch param for asymmetric
    BranchCalcParam<false> calc_param_y_asym(DoubleComplex const& y1_series, DoubleComplex const& y1_shunt,
                                             DoubleComplex const& y0_series, DoubleComplex const& y0_shunt,
                                             DoubleComplex const& tap_ratio) const {
        BranchCalcParam<true> const param1 = calc_param_y_sym(y1_series, y1_shunt, tap_ratio);
        BranchCalcParam<true> const param0 = calc_param_y_sym(y0_series, y0_shunt, tap_ratio);
        // abc matrix
        // 1/3 *
        // [[2y1+y0, y0-y1, y0-y1],
        //  [y0-y1, 2y1+y0, y0-y1],
        //  [y0-y1, y0-y1, 2y1+y0]]
        BranchCalcParam<false> param{};
        for (size_t i = 0; i < 4; ++i) {
            param.value[i] = ComplexTensor<false>{(2.0 * param1.value[i] + param0.value[i]) / 3.0,
                                                  (param0.value[i] - param1.value[i]) / 3.0};
        }
        return param;
    }

   private:
    ID from_node_;
    ID to_node_;
    bool from_status_;
    bool to_status_;

    virtual BranchCalcParam<true> sym_calc_param() const = 0;
    virtual BranchCalcParam<false> asym_calc_param() const = 0;
};

}  // namespace power_grid_model

#endif
