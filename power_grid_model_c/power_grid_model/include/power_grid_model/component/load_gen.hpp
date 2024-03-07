// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "appliance.hpp"
#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"

namespace power_grid_model {

struct load_appliance_t {};
struct gen_appliance_t {};

template <typename T>
concept appliance_type_tag = std::same_as<T, load_appliance_t> || std::same_as<T, gen_appliance_t>;
template <appliance_type_tag T> constexpr bool is_generator_v = std::same_as<T, gen_appliance_t>;

static_assert(appliance_type_tag<load_appliance_t>);
static_assert(appliance_type_tag<gen_appliance_t>);
static_assert(!is_generator_v<load_appliance_t>);
static_assert(is_generator_v<gen_appliance_t>);

class GenericLoadGen : public Appliance {
  public:
    using InputType = GenericLoadGenInput;
    static constexpr char const* name = "generic_load_gen";
    ComponentType math_model_type() const final { return ComponentType::generic_load_gen; }

    explicit GenericLoadGen(GenericLoadGenInput const& generic_load_gen_input, double u)
        : Appliance{generic_load_gen_input, u}, type_{generic_load_gen_input.type} {}

    // getter for load type
    LoadGenType type() const { return type_; }
    // getter for calculation param, power injection
    template <symmetry_tag sym> ComplexValue<sym> calc_param(bool is_connected_to_source = true) const {
        if (!energized(is_connected_to_source)) {
            return ComplexValue<sym>{};
        }
        if constexpr (is_symmetric_v<sym>) {
            return sym_calc_param();
        } else {
            return asym_calc_param();
        }
    }

  private:
    LoadGenType type_;
    virtual ComplexValue<symmetric_t> sym_calc_param() const = 0;
    virtual ComplexValue<asymmetric_t> asym_calc_param() const = 0;
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

template <symmetry_tag loadgen_symmetry_, appliance_type_tag appliance_type_>
class LoadGen final : public std::conditional_t<is_generator_v<appliance_type_>, GenericGenerator, GenericLoad> {
  public:
    using loadgen_symmetry = loadgen_symmetry_;
    using appliance_type = appliance_type_;

    using InputType = LoadGenInput<loadgen_symmetry>;
    using UpdateType = LoadGenUpdate<loadgen_symmetry>;
    using BaseClass = std::conditional_t<is_generator_v<appliance_type>, GenericGenerator, GenericLoad>;
    static constexpr char const* name = [] {
        if constexpr (is_symmetric_v<loadgen_symmetry>) {
            return is_generator_v<appliance_type> ? "sym_gen" : "sym_load";
        } else {
            return is_generator_v<appliance_type> ? "asym_gen" : "asym_load";
        }
    }();

    LoadGen(LoadGenInput<loadgen_symmetry> const& load_gen_input, double u) : BaseClass{load_gen_input, u} {
        set_power(load_gen_input.p_specified, load_gen_input.q_specified);
    }

    void set_power(RealValue<loadgen_symmetry> const& new_p_specified,
                   RealValue<loadgen_symmetry> const& new_q_specified) {
        double const scalar = direction_ / base_power<loadgen_symmetry>;
        RealValue<loadgen_symmetry> ps = real(s_specified_);
        RealValue<loadgen_symmetry> qs = imag(s_specified_);
        update_real_value<loadgen_symmetry>(new_p_specified, ps, scalar);
        update_real_value<loadgen_symmetry>(new_q_specified, qs, scalar);

        s_specified_ = ComplexValue<loadgen_symmetry>{ps, qs};
    }

    // update for load_gen
    UpdateChange update(LoadGenUpdate<loadgen_symmetry> const& update_data) {
        assert(update_data.id == this->id());
        this->set_status(update_data.status);
        set_power(update_data.p_specified, update_data.q_specified);
        // change load connection and/or value will not change topology or parameters
        return {false, false};
    }

    LoadGenUpdate<loadgen_symmetry> inverse(LoadGenUpdate<loadgen_symmetry> update_data) const {
        double const scalar = direction_ * base_power<loadgen_symmetry>;

        assert(update_data.id == this->id());

        set_if_not_nan(update_data.status, static_cast<IntS>(this->status()));
        set_if_not_nan(update_data.p_specified, real(s_specified_) * scalar);
        set_if_not_nan(update_data.q_specified, imag(s_specified_) * scalar);

        return update_data;
    }

  private:
    ComplexValue<loadgen_symmetry> s_specified_{std::complex<double>{nan, nan}}; // specified power injection

    // direction of load_gen
    static constexpr double direction_ = is_generator_v<appliance_type> ? 1.0 : -1.0;

    // override calc_param
    ComplexValue<symmetric_t> sym_calc_param() const override {
        if constexpr (is_symmetric_v<loadgen_symmetry>) {
            if (is_nan(real(s_specified_)) || is_nan(imag(s_specified_))) {
                return {nan, nan};
            }
        }
        return mean_val(s_specified_);
    }
    ComplexValue<asymmetric_t> asym_calc_param() const override {
        if constexpr (is_symmetric_v<loadgen_symmetry>) {
            if (is_nan(real(s_specified_)) || is_nan(imag(s_specified_))) {
                return ComplexValue<asymmetric_t>{std::complex{nan, nan}};
            }
        }
        return piecewise_complex_value(s_specified_);
    }
    template <symmetry_tag calculation_symmetry>
    ApplianceMathOutput<calculation_symmetry> u2si(ComplexValue<calculation_symmetry> const& u) const {
        ApplianceMathOutput<calculation_symmetry> appliance_math_output;
        appliance_math_output.s = scale_power<calculation_symmetry>(u);
        appliance_math_output.i = conj(appliance_math_output.s / u);
        return appliance_math_output;
    }
    ApplianceMathOutput<symmetric_t> sym_u2si(ComplexValue<symmetric_t> const& u) const override {
        return u2si<symmetric_t>(u);
    }
    ApplianceMathOutput<asymmetric_t> asym_u2si(ComplexValue<asymmetric_t> const& u) const override {
        return u2si<asymmetric_t>(u);
    }

    double injection_direction() const override { return direction_; }

    // scale load
    template <symmetry_tag calculation_symmetry>
    ComplexValue<calculation_symmetry> scale_power(ComplexValue<calculation_symmetry> u) const {
        using enum LoadGenType;

        auto const params = [this] { return this->template calc_param<calculation_symmetry>(); };
        switch (this->type()) {
        case const_pq:
            return params();
        case const_y:
            return params() * abs2(u);
        case const_i:
            return params() * cabs(u);
        default:
            throw MissingCaseForEnumError(std::string(this->name) + " power scaling factor", this->type());
        }
    }
};

// explicit instantiation
template class LoadGen<symmetric_t, gen_appliance_t>;
template class LoadGen<symmetric_t, load_appliance_t>;
template class LoadGen<asymmetric_t, gen_appliance_t>;
template class LoadGen<asymmetric_t, load_appliance_t>;
// alias
using SymGenerator = LoadGen<symmetric_t, gen_appliance_t>;
using AsymGenerator = LoadGen<asymmetric_t, gen_appliance_t>;
using SymLoad = LoadGen<symmetric_t, load_appliance_t>;
using AsymLoad = LoadGen<asymmetric_t, load_appliance_t>;

} // namespace power_grid_model
