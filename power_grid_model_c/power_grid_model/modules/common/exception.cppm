// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/exception.hpp>

export module power_grid_model.common.exception;

export namespace power_grid_model {
using power_grid_model::AutomaticTapCalculationError;
using power_grid_model::AutomaticTapInputError;
using power_grid_model::BatchCalculationError;
using power_grid_model::CalculationError;
using power_grid_model::ConflictID;
using power_grid_model::ConflictingAngleMeasurementType;
using power_grid_model::ConflictingVoltageRegulatorURef;
using power_grid_model::ConflictVoltage;
using power_grid_model::DatasetError;
using power_grid_model::DuplicativelyRegulatedObject;
using power_grid_model::ExperimentalFeature;
using power_grid_model::IDNotFound;
using power_grid_model::IDWrongType;
using power_grid_model::Idx2DNotFound;
using power_grid_model::InvalidArguments;
using power_grid_model::InvalidCalculationMethod;
using power_grid_model::InvalidMeasuredObject;
using power_grid_model::InvalidMeasuredTerminalType;
using power_grid_model::InvalidRegulatedObject;
using power_grid_model::InvalidShortCircuitPhaseOrType;
using power_grid_model::InvalidShortCircuitPhases;
using power_grid_model::InvalidShortCircuitType;
using power_grid_model::InvalidTransformerClock;
using power_grid_model::IterationDiverge;
using power_grid_model::MaxIterationReached;
using power_grid_model::MissingCaseForEnumError;
using power_grid_model::NotImplementedError;
using power_grid_model::NotObservableError;
using power_grid_model::PowerGridError;
using power_grid_model::SerializationError;
using power_grid_model::SparseMatrixError;
using power_grid_model::TapSearchStrategyIncompatibleError;
using power_grid_model::UnreachableHit;
using power_grid_model::UnsupportedLoadGenTypeForVoltageRegulator;
using power_grid_model::UnsupportedRegulatorCombinationError;
using power_grid_model::UnsupportedVoltageRegulatorSourceCombinationError;
} // namespace power_grid_model
