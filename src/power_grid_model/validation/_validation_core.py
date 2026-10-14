# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""Core-backed input validation and validation-report conversion."""

import json
from collections.abc import Mapping
from ctypes import byref
from typing import Any

from power_grid_model._core.dataset_definitions import (
    AttributeType as AT,
    ComponentType as CT,
    DatasetType,
    _map_to_component_types,
)
from power_grid_model._core.enum import _ExperimentalFeatures
from power_grid_model._core.error_handling import assert_no_error
from power_grid_model._core.index_integer import IdxC
from power_grid_model._core.options import Options
from power_grid_model._core.power_grid_core import CStr, get_power_grid_core as get_pgc
from power_grid_model._core.power_grid_dataset import CConstDataset
from power_grid_model.data_types import SingleDataset
from power_grid_model.enum import CalculationType
from power_grid_model.validation.errors import (
    InfinityError,
    MissingValueError,
    NotGreaterThanError,
    ValidationError,
)


def _validation_issue_json_to_error(issue: Mapping[str, Any]) -> ValidationError:
    component = CT(issue["component"])
    field = AT(issue["field"])
    issue_ids = issue["ids"]

    if issue["rule"] == "missing_value":
        return MissingValueError(component, field, issue_ids)
    if issue["rule"] == "infinity":
        return InfinityError(component, field, issue_ids)
    if issue["rule"] == "not_greater_than_zero":
        return NotGreaterThanError(component, field, issue_ids, 0)
    raise ValueError(f"Unsupported validation rule '{issue['rule']}'.")


def validate_input_data_core(
    input_data: SingleDataset,
    calculation_type: CalculationType | None = None,
    symmetric: bool = True,
) -> list[ValidationError] | None:
    input_data = _map_to_component_types(input_data)
    dataset = CConstDataset(input_data, DatasetType.input)
    options = Options()

    effective_calculation_type = calculation_type if calculation_type is not None else CalculationType.power_flow
    options.calculation_type = int(effective_calculation_type)
    options.symmetric = int(symmetric)
    options.experimental_features = int(_ExperimentalFeatures.enabled)
    assert_no_error()

    get_pgc().validate_input_data(dataset.get_dataset_ptr(), options.opt)
    assert_no_error()
    report_data = CStr()
    report_size = IdxC()
    get_pgc().validation_report_get_json(byref(report_data), byref(report_size))
    assert_no_error()
    validation_report_json = report_data.value
    report = json.loads(validation_report_json[: report_size.value].decode() if validation_report_json else "")
    errors = [_validation_issue_json_to_error(issue) for result in report["results"] for issue in result["issues"]]
    return errors or None
