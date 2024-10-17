# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Error handling

[deprecated]
"""

from power_grid_model._core.error_handling import (  # pylint: disable=unused-import
    PGM_BATCH_ERROR,
    PGM_NO_ERROR,
    PGM_REGULAR_ERROR,
    PGM_SERIALIZATION_ERROR,
    VALIDATOR_MSG,
    AutomaticTapCalculationError,
    ConflictID,
    ConflictVoltage,
    IDNotFound,
    IDWrongType,
    IdxNp,
    InvalidArguments,
    InvalidBranch,
    InvalidBranch3,
    InvalidCalculationMethod,
    InvalidMeasuredObject,
    InvalidRegulatedObject,
    InvalidShortCircuitPhaseOrType,
    InvalidTransformerClock,
    IterationDiverge,
    MaxIterationReached,
    MissingCaseForEnumError,
    NotObservableError,
    Optional,
    PowerGridBatchError,
    PowerGridDatasetError,
    PowerGridError,
    PowerGridNotImplementedError,
    PowerGridSerializationError,
    PowerGridUnreachableHitError,
    SparseMatrixError,
    assert_no_error,
    find_error,
    handle_errors,
    np,
    pgc,
    re,
)
