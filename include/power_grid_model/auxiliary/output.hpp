// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_OUTPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_OUTPUT_HPP

#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

struct BaseOutput {
    ID id;
    IntS energized;
};

template <bool sym>
struct NodeOutput : BaseOutput {
    RealValue<sym> u_pu, u, u_angle;
};

template <bool sym>
struct BranchOutput : BaseOutput {
    double loading;
    RealValue<sym> p_from, q_from, i_from, s_from;
    RealValue<sym> p_to, q_to, i_to, s_to;
};

template <bool sym>
struct ApplianceOutput : BaseOutput {
    RealValue<sym> p, q, i, s, pf;
};

template <bool sym>
struct VoltageSensorOutput : BaseOutput {
    RealValue<sym> u_residual;
    RealValue<sym> u_angle_residual;
};

template <bool sym>
struct PowerSensorOutput : BaseOutput {
    RealValue<sym> p_residual;
    RealValue<sym> q_residual;
};

using SymNodeOutput = NodeOutput<true>;
using AsymNodeOutput = NodeOutput<false>;
using SymBranchOutput = BranchOutput<true>;
using AsymBranchOutput = BranchOutput<false>;
using SymApplianceOutput = ApplianceOutput<true>;
using AsymApplianceOutput = ApplianceOutput<false>;

}  // namespace power_grid_model

#endif