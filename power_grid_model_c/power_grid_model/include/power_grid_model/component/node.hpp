// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_NODE_HPP
#define POWER_GRID_MODEL_COMPONENT_NODE_HPP

#include "base.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

class Node final : public Base {
   public:
    using InputType = NodeInput;
    template <bool sym>
    using OutputType = NodeOutput<sym>;
    using ShortCircuitOutputType = NodeShortCircuitOutput;
    static constexpr char const* name = "node";
    constexpr ComponentType math_model_type() const final {
        return ComponentType::node;
    }

    explicit Node(NodeInput const& node_input) : Base{node_input}, u_rated_{node_input.u_rated} {
    }

    // update node, nothing happens here
    constexpr UpdateChange update(BaseUpdate const&) {
        return {false, false};
    }

    // energized
    template <bool sym>
    NodeOutput<sym> get_output(ComplexValue<sym> const& u_pu, ComplexValue<sym> const& bus_injection) const {
        NodeOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(true);
        output.u_pu = cabs(u_pu);
        output.u = u_scale<sym> * u_rated_ * output.u_pu;
        output.u_angle = arg(u_pu);
        output.p = base_power<sym> * real(bus_injection);
        output.q = base_power<sym> * imag(bus_injection);
        return output;
    }

    NodeShortCircuitOutput get_sc_output(ComplexValue<false> const& u_pu) const {
        NodeShortCircuitOutput output{};
        static_cast<BaseOutput&>(output) = base_output(true);
        output.u_pu = cabs(u_pu);
        output.u = u_scale<false> * u_rated_ * output.u_pu;
        output.u_angle = arg(u_pu);

        return output;
    }
    NodeShortCircuitOutput get_sc_output(ComplexValue<true> const& u_pu) const {
        // Convert the input positive sequence voltage to phase voltage
        ComplexValue<false> const uabc_pu{u_pu};
        return get_sc_output(uabc_pu);
    }
    template <bool sym>
    NodeOutput<sym> get_null_output() const {
        NodeOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }

    NodeShortCircuitOutput get_null_sc_output() const {
        NodeShortCircuitOutput output{};
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }

    constexpr double u_rated() const {
        return u_rated_;
    }
    constexpr bool energized(bool is_connected_to_source) const final {
        return is_connected_to_source;
    }

   private:
    double u_rated_;
};

}  // namespace power_grid_model

#endif
