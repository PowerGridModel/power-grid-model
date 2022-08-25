// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// clang-format off


#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_OUTPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_OUTPUT_HPP

#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "meta_data.hpp"

namespace power_grid_model {

POWER_GRID_MODEL_DATA_STRUCT_DEF(BaseOutput, 0, _,
    ID, id,
    IntS, energized);

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(NodeOutput, 1, BaseOutput,
    RealValue<sym>, u_pu, RealValue<sym>, u, RealValue<sym>, u_angle);

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(BranchOutput, 1, BaseOutput,
    double, loading,
    RealValue<sym>, p_from, RealValue<sym>, q_from, RealValue<sym>, i_from, RealValue<sym>, s_from,
    RealValue<sym>, p_to, RealValue<sym>, q_to, RealValue<sym>, i_to, RealValue<sym>, s_to);

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(Branch3Output, 1, BaseOutput,
    double, loading,
    RealValue<sym>, p_1, RealValue<sym>, q_1, RealValue<sym>, i_1, RealValue<sym>, s_1,
    RealValue<sym>, p_2, RealValue<sym>, q_2, RealValue<sym>, i_2, RealValue<sym>, s_2,
    RealValue<sym>, p_3, RealValue<sym>, q_3, RealValue<sym>, i_3, RealValue<sym>, s_3);

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(ApplianceOutput, 1, BaseOutput,
    RealValue<sym>, p, RealValue<sym>, q, RealValue<sym>, i, RealValue<sym>, s, RealValue<sym>, pf);

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(VoltageSensorOutput, 1, BaseOutput,
    RealValue<sym>, u_residual,
    RealValue<sym>, u_angle_residual);

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(PowerSensorOutput, 1, BaseOutput,
    RealValue<sym>, p_residual,
    RealValue<sym>, q_residual);

using SymNodeOutput = NodeOutput<true>;
using AsymNodeOutput = NodeOutput<false>;
using SymBranchOutput = BranchOutput<true>;
using AsymBranchOutput = BranchOutput<false>;
using SymApplianceOutput = ApplianceOutput<true>;
using AsymApplianceOutput = ApplianceOutput<false>;

}  // namespace power_grid_model

#endif

// clang-format off
