// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_NODE_HPP
#define POWER_GRID_MODEL_COMPONENT_NODE_HPP

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../calculation_parameters.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "base.hpp"

namespace power_grid_model {

class Node final : public Base {
   public:
    using InputType = NodeInput;
    template <bool sym>
    using OutputType = NodeOutput<sym>;
    static constexpr char const* name = "node";
    ComponentType math_model_type() const final {
        return ComponentType::node;
    }

    Node(NodeInput const& node_input) : Base{node_input}, u_rated_{node_input.u_rated} {
    }

    // update node, nothing happens here
    UpdateChange update(BaseInput const&) {
        return {false, false};
    }

    // energized
    template <bool sym>
    NodeOutput<sym> get_output(ComplexValue<sym> const& u_pu) const {
        NodeOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(true);
        output.u_pu = cabs(u_pu);
        output.u = u_scale<sym> * u_rated_ * output.u_pu;
        output.u_angle = arg(u_pu);
        return output;
    }
    template <bool sym>
    NodeOutput<sym> get_null_output() const {
        NodeOutput<sym> output{};
        static_cast<BaseOutput&>(output) = base_output(false);
        return output;
    }

    double u_rated() {
        return u_rated_;
    }
    bool energized(bool is_connected_to_source) const final {
        return is_connected_to_source;
    }

   private:
    double u_rated_;
};

}  // namespace power_grid_model

#endif