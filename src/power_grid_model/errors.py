# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Error classes used by the power-grid-model library.
"""


from power_grid_model._core.errors import (  # pylint: disable=unused-import
    AutomaticTapCalculationError,
    AutomaticTapInputError,
    ConflictID,
    ConflictingAngleMeasurementType,
    ConflictVoltage,
    IDNotFound,
    IDWrongType,
    InvalidArguments,
    InvalidBranch,
    InvalidBranch3,
    InvalidCalculationMethod,
    InvalidID,
    InvalidMeasuredObject,
    InvalidRegulatedObject,
    InvalidShortCircuitPhaseOrType,
    InvalidTransformerClock,
    IterationDiverge,
    MaxIterationReached,
    MissingCaseForEnumError,
    NotObservableError,
    PowerGridBatchError,
    PowerGridDatasetError,
    PowerGridError,
    PowerGridNotImplementedError,
    PowerGridSerializationError,
    PowerGridUnreachableHitError,
    SparseMatrixError,
)
