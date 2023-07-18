// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_LOAD_GEN_HPP
#define POWER_GRID_MODEL_COMPONENT_LOAD_GEN_HPP

#include "appliance.hpp"
#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

class GenericLoadGen : public Appliance {
   public:
    using InputType = GenericLoadGenInput;
    static constexpr char const* name = "generic_load_gen";
    ComponentType math_model_type() const final {
        return ComponentType::generic_load_gen;
    }

    explicit GenericLoadGen(GenericLoadGenInput const& generic_load_gen_input, double u)
        : Appliance{generic_load_gen_input, u}, type_{generic_load_gen_input.type} {
    }

    // getter for load type
    LoadGenType type() const {
        return type_;
    }
    // getter for calculation param, power injection
    template <bool sym>
    ComplexValue<sym> calc_param(bool is_connected_to_source = true) const {
        if (!energized(is_connected_to_source)) {
            return ComplexValue<sym>{};
        }
        if constexpr (sym) {
            return sym_calc_param();
        }
        else {
            return asym_calc_param();
        }
    }

   private:
    LoadGenType type_;
    virtual ComplexValue<true> sym_calc_param() const = 0;
    virtual ComplexValue<false> asym_calc_param() const = 0;
};

// abstraction of load/generation
class GenericLoad : public GenericLoadGen {
   public:
    using GenericLoadGen::GenericLoadGen;
};
class GenericGenerator : public GenericLoadGen {
   public:
    using GenericLoadGen::GenericLoadGen;
};

template <bool sym, bool is_gen>
class LoadGen final : public std::conditional_t<is_gen, GenericGenerator, GenericLoad> {
   public:
    using InputType = LoadGenInput<sym>;
    using UpdateType = LoadGenUpdate<sym>;
    using BaseClass = std::conditional_t<is_gen, GenericGenerator, GenericLoad>;
    static constexpr char const* name = sym ? (is_gen ? "sym_gen" : "sym_load") : (is_gen ? "asym_gen" : "asym_load");

    LoadGen(LoadGenInput<sym> const& load_gen_input, double u) : BaseClass{load_gen_input, u} {
        set_power(load_gen_input.p_specified, load_gen_input.q_specified);
    }

    void set_power(RealValue<sym> const& new_p_specified, RealValue<sym> const& new_q_specified) {
        double const scalar = direction_ / base_power<sym>;
        RealValue<sym> ps = real(s_specified_);
        RealValue<sym> qs = imag(s_specified_);
        update_real_value<sym>(new_p_specified, ps, scalar);
        update_real_value<sym>(new_q_specified, qs, scalar);

        s_specified_ = ps + 1.0i * qs;
    }

    // update for load_gen
    UpdateChange update(LoadGenUpdate<sym> const& update) {
        assert(update.id == this->id());
        this->set_status(update.status);
        set_power(update.p_specified, update.q_specified);
        // change load connection and/or value will not change topology or parameters
        return {false, false};
    }

   private:
    ComplexValue<sym> s_specified_{};  // specified power injection

    // direction of load_gen
    static constexpr double direction_ = is_gen ? 1.0 : -1.0;

    // override calc_param
    ComplexValue<true> sym_calc_param() const final {
        return mean_val(s_specified_);
    }
    ComplexValue<false> asym_calc_param() const final {
        return piecewise_complex_value(s_specified_);
    }
    template <bool sym_calc>
    ApplianceMathOutput<sym_calc> u2si(ComplexValue<sym_calc> const& u) const {
        ApplianceMathOutput<sym_calc> appliance_math_output;
        appliance_math_output.s = scale_power<sym_calc>(u);
        appliance_math_output.i = conj(appliance_math_output.s / u);
        return appliance_math_output;
    }
    ApplianceMathOutput<true> sym_u2si(ComplexValue<true> const& u) const final {
        return u2si<true>(u);
    }
    ApplianceMathOutput<false> asym_u2si(ComplexValue<false> const& u) const final {
        return u2si<false>(u);
    }

    double injection_direction() const final {
        return direction_;
    }

    // scale load
    template <bool sym_calc>
    ComplexValue<sym_calc> scale_power(ComplexValue<sym_calc> u) const {
        using enum LoadGenType;

        ComplexValue<sym_calc> s = this->template calc_param<sym_calc>();
        switch (this->type()) {
            case const_pq:
                return s;
            case const_y:
                return s * abs2(u);
            case const_i:
                return s * cabs(u);
            default:
                throw MissingCaseForEnumError(std::string(this->name) + " power scaling factor", this->type());
        }
    }
};

// explicit instantiation
template class LoadGen<true, true>;
template class LoadGen<true, false>;
template class LoadGen<false, true>;
template class LoadGen<false, false>;
// alias
using SymGenerator = LoadGen<true, true>;
using AsymGenerator = LoadGen<false, true>;
using SymLoad = LoadGen<true, false>;
using AsymLoad = LoadGen<false, false>;

}  // namespace power_grid_model

#endif
