# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Main power grid model class

[deprecated]
"""

from power_grid_model._core.power_grid_model import (  # pylint: disable=unused-import
    CalculationMethod,
    CalculationType,
    ComponentAttributeMapping,
    ComponentType,
    ComponentTypeLike,
    ComponentTypeVar,
    ConstDatasetPtr,
    Dataset,
    IdNp,
    IDPtr,
    IdxNp,
    IdxPtr,
    IntEnum,
    ModelPtr,
    Optional,
    Options,
    PowerGridBatchError,
    PowerGridModel,
    ShortCircuitVoltageScaling,
    SingleDataset,
    TapChangingStrategy,
    Type,
    assert_no_error,
    compatibility_convert_row_columnar_dataset,
    create_output_data,
    get_output_type,
    handle_errors,
    np,
    pgc,
    prepare_input_view,
    prepare_output_view,
    prepare_update_view,
)
