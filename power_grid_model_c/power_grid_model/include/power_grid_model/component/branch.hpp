// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base.hpp"
#include "edge.hpp"

#include "../auxiliary/input.hpp"
#include "../auxiliary/output.hpp"
#include "../auxiliary/update.hpp"
#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"
#include "component.hpp"

#include <Eigen/Core>

#include <cassert>
#include <complex>
#include <concepts>
#include <cstddef>

namespace power_grid_model {

class Branch : public Edge { // TODO: not yet unit tested
  public:
    using InputType = BranchInput;
    using UpdateType = BranchUpdate;
    template <symmetry_tag sym> using OutputType = BranchOutput<sym>;

    static constexpr char const* name = "branch";
    ComponentType math_model_type() const final { return ComponentType::branch; }

    using Edge::Edge; // inherit constructor
};

} // namespace power_grid_model
