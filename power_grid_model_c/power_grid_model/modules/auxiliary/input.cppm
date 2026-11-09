// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <concepts>
#include <cstddef>
#include <type_traits>

import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.three_phase_tensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/static_asserts/input.hpp>

export module power_grid_model.auxiliary.input;

export namespace power_grid_model {

using power_grid_model::BaseInput;

using power_grid_model::NodeInput;

using power_grid_model::BranchInput;

using power_grid_model::Branch3Input;

using power_grid_model::SensorInput;

using power_grid_model::ApplianceInput;

using power_grid_model::LineInput;

using power_grid_model::AsymLineInput;

using power_grid_model::GenericBranchInput;

using power_grid_model::LinkInput;

using power_grid_model::TransformerInput;

using power_grid_model::ThreeWindingTransformerInput;

using power_grid_model::GenericLoadGenInput;

using power_grid_model::AsymLoadGenInput;
using power_grid_model::LoadGenInput;
using power_grid_model::SymLoadGenInput;

using power_grid_model::ShuntInput;

using power_grid_model::SourceInput;

using power_grid_model::GenericVoltageSensorInput;

using power_grid_model::AsymVoltageSensorInput;
using power_grid_model::SymVoltageSensorInput;
using power_grid_model::VoltageSensorInput;

using power_grid_model::GenericPowerSensorInput;

using power_grid_model::AsymPowerSensorInput;
using power_grid_model::PowerSensorInput;
using power_grid_model::SymPowerSensorInput;

using power_grid_model::FaultInput;

using power_grid_model::RegulatorInput;

using power_grid_model::TransformerTapRegulatorInput;

using power_grid_model::VoltageRegulatorInput;

using power_grid_model::GenericCurrentSensorInput;

using power_grid_model::AsymCurrentSensorInput;
using power_grid_model::CurrentSensorInput;
using power_grid_model::SymCurrentSensorInput;

} // namespace power_grid_model

// clang-format on
