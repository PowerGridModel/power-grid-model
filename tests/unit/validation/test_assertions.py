# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from unittest.mock import MagicMock, patch

import numpy as np
import pytest

from power_grid_model.enum import CalculationType
from power_grid_model.validation.assertions import ValidationException, assert_valid_batch_data, assert_valid_input_data
from power_grid_model.validation.errors import ValidationError


@patch("power_grid_model.validation.assertions.errors_to_string")
def test_validation_exception(errors_to_string_mock: MagicMock):
    error = ValidationError()
    exception = ValidationException([error, error], name="dummy data")
    assert exception.errors == [error, error]
    assert exception.name == "dummy data"

    errors_to_string_mock.return_value = "Dummy Exception"
    assert str(exception) == "Dummy Exception"
    errors_to_string_mock.assert_called_once_with(errors=[error, error], name="dummy data")


@patch("power_grid_model.validation.assertions.validate_input_data")
def test_assert_valid_input_data(validate_mock: MagicMock):
    validate_mock.return_value = None
    assert_valid_input_data(
        input_data={"foo": np.array([1])}, calculation_type=CalculationType.state_estimation, symmetric=False
    )
    validate_mock.assert_called_once_with(
        input_data={"foo": np.array([1])}, calculation_type=CalculationType.state_estimation, symmetric=False
    )

    validate_mock.return_value = [ValidationError()]
    with pytest.raises(ValidationException):
        assert_valid_input_data({})


@patch("power_grid_model.validation.assertions.validate_batch_data")
def test_assert_valid_batch_data(validate_mock: MagicMock):
    validate_mock.return_value = None
    assert_valid_batch_data(
        input_data={"foo": np.array([1])},
        update_data={"bar": np.array([2])},
        calculation_type=CalculationType.state_estimation,
        symmetric=False,
    )
    validate_mock.assert_called_once_with(
        input_data={"foo": np.array([1])},
        update_data={"bar": np.array([2])},
        calculation_type=CalculationType.state_estimation,
        symmetric=False,
    )

    validate_mock.return_value = [ValidationError()]
    with pytest.raises(ValidationException):
        assert_valid_batch_data({}, {})
