// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_BRANCH3_HPP
#define POWER_GRID_MODEL_COMPONENT_BRANCH3_HPP

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
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
          status_1_{branch3_input.status_1},
          status_2_{branch3_input.status_2},
          status_3_{branch3_input.status_3} {
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
    bool status_1() {
        return status_1_;
    }
    bool status_2() {
        return status_2_;
    }
    bool status_3() {
        return status_3_;
    }
    bool branch3_status() {
        return status_1_ && status_2_ && status_3_;  // TODO: check if this makes sense for branch3
    }

    // TODO calc_param()

    // TODO energized()

   private:
    ID node_1_;
    ID node_2_;
    ID node_3_;
    bool status_1_;
    bool status_2_;
    bool status_3_;
};

}  // namespace power_grid_model

#endif