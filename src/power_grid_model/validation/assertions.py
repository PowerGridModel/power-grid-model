# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Helper functions to assert valid data. They basically call validate_input_data or validate_batch_data and raise a
ValidationException if the validation results in one or more errors.
"""
from typing import Dict, List, Optional, Union

from power_grid_model.data_types import BatchDataset, SingleDataset
from power_grid_model.enum import CalculationType
from power_grid_model.validation.errors import ValidationError
from power_grid_model.validation.utils import errors_to_string
from power_grid_model.validation.validation import validate_batch_data, validate_input_data


class ValidationException(ValueError):
    """
    An exception storing the name of the validated data, a list/dict of errors and a convenient conversion to string
    to display a summary of all the errors when printing the exception.
    """

    def __init__(self, errors: Union[List[ValidationError], Dict[int, List[ValidationError]]], name: str = "data"):
        super().__init__(f"Invalid {name}")
        self.errors = errors
        self.name = name

    def __str__(self):
        return errors_to_string(errors=self.errors, name=self.name)


def assert_valid_input_data(
    input_data: SingleDataset, calculation_type: Optional[CalculationType] = None, symmetric: bool = True
):
    """
    Validates the entire input dataset:

        1. Is the data structure correct? (checking data types and numpy array shapes)
        2. Are all required values provided? (checking NaNs)
        3. Are all ID's unique? (checking object identifiers across all components)
        4. Are the supplied values valid? (checking limits and other logic as described in "Graph Data Model")

    Args:
        input_data: A power-grid-model input dataset
        calculation_type: Supply a calculation method, to allow missing values for unused fields
        symmetric: A boolean to state whether input data will be used for a symmetric or asymmetric calculation

    Raises:
        KeyError | TypeError | ValueError: if the data structure is invalid.
        ValidationException: if the contents are invalid.
    """
    validation_errors = validate_input_data(
        input_data=input_data, calculation_type=calculation_type, symmetric=symmetric
    )
    if validation_errors:
        raise ValidationException(validation_errors, "input_data")


def assert_valid_batch_data(
    input_data: SingleDataset,
    update_data: BatchDataset,
    calculation_type: Optional[CalculationType] = None,
    symmetric: bool = True,
):
    """
    The input dataset is validated:

        1. Is the data structure correct? (checking data types and numpy array shapes)
        2. Are all input data ID's unique? (checking object identifiers across all components)

    For each batch the update data is validated:
        3. Is the update data structure correct? (checking data types and numpy array shapes)
        4. Are all update ID's valid? (checking object identifiers across update and input data)

    Then (for each batch independently) the input dataset is updated with the batch's update data and validated:
        5. Are all required values provided? (checking NaNs)
        6. Are the supplied values valid? (checking limits and other logic as described in "Graph Data Model")

    Args:
        input_data: a power-grid-model input dataset
        update_data: a power-grid-model update dataset (one or more batches)
        calculation_type: Supply a calculation method, to allow missing values for unused fields
        symmetric: A boolean to state whether input data will be used for a symmetric or asymmetric calculation

    Raises:
        KeyError | TypeError | ValueError: if the data structure is invalid.
        ValidationException: if the contents are invalid.
    """
    validation_errors = validate_batch_data(
        input_data=input_data, update_data=update_data, calculation_type=calculation_type, symmetric=symmetric
    )
    if validation_errors:
        raise ValidationException(validation_errors, "update_data")
