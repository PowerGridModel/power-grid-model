# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""Core-backed input validation and validation-report conversion."""

import json
from collections.abc import Mapping
from typing import Any

from power_grid_model._core.dataset_definitions import (
    AttributeType as AT,
    ComponentType as CT,
    DatasetType,
    _map_to_component_types,
)
from power_grid_model._core.enum import _ExperimentalFeatures
from power_grid_model._core.error_handling import assert_no_error
from power_grid_model._core.options import Options
from power_grid_model._core.power_grid_core import (
    ConstDatasetPtr,
    ValidationReportPtr,
    get_power_grid_core as get_pgc,
)
from power_grid_model._core.power_grid_dataset import CConstDataset
from power_grid_model.data_types import BatchDataset, SingleDataset
from power_grid_model.enum import CalculationType
from power_grid_model.validation.errors import (
    InfinityError,
    MissingValueError,
    NotGreaterThanError,
    NotLessThanError,
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
    if issue["rule"] == "not_less_than_zero":
        return NotLessThanError(component, field, issue_ids, 0)
    if issue["rule"] == "not_greater_than_zero":
        return NotGreaterThanError(component, field, issue_ids, 0)
    raise ValueError(f"Unsupported validation rule '{issue['rule']}'.")


def _make_validation_options(calculation_type: CalculationType | None, symmetric: bool) -> Options:
    options = Options()
    effective_calculation_type = calculation_type if calculation_type is not None else CalculationType.power_flow
    options.calculation_type = int(effective_calculation_type)
    options.symmetric = int(symmetric)
    options.experimental_features = int(_ExperimentalFeatures.enabled)
    assert_no_error()
    return options


def _validate_input_data_core(
    input_dataset: SingleDataset,
    calculation_type: CalculationType | None = None,
    symmetric: bool = True,
) -> list[ValidationError] | None:
    input_dataset = _map_to_component_types(input_dataset)
    c_input_dataset = CConstDataset(input_dataset, DatasetType.input)
    options = _make_validation_options(calculation_type, symmetric)
    pgc = get_pgc()
    c_report: ValidationReportPtr = pgc.validate_data(c_input_dataset.get_dataset_ptr(), ConstDatasetPtr(), options.opt)
    try:
        assert_no_error()
        report_data = pgc.validation_report_get_to_zero_terminated_string(c_report)
        assert_no_error()
        report = json.loads(report_data)
        errors = [_validation_issue_json_to_error(issue) for result in report["results"] for issue in result["issues"]]
        return errors or None
    finally:
        pgc.destroy_validation_report(c_report)


def _validate_batch_data_core(
    input_dataset: SingleDataset,
    update_dataset: BatchDataset,
    calculation_type: CalculationType | None = None,
    symmetric: bool = True,
) -> dict[int, list[ValidationError]] | None:
    input_dataset = _map_to_component_types(input_dataset)
    update_dataset = _map_to_component_types(update_dataset)
    c_input_dataset = CConstDataset(input_dataset, DatasetType.input)
    c_update_dataset = CConstDataset(update_dataset, DatasetType.update)
    options = _make_validation_options(calculation_type, symmetric)
    pgc = get_pgc()
    c_report: ValidationReportPtr = pgc.validate_data(
        c_input_dataset.get_dataset_ptr(), c_update_dataset.get_dataset_ptr(), options.opt
    )
    try:
        assert_no_error()
        report_data = pgc.validation_report_get_to_zero_terminated_string(c_report)
        assert_no_error()
        report = json.loads(report_data)
        errors: dict[int, list[ValidationError]] = {}
        for result in report["results"]:
            scenario = result["scenario"]
            if scenario is not None:
                scenario_errors = [_validation_issue_json_to_error(issue) for issue in result["issues"]]
                errors.setdefault(scenario, []).extend(scenario_errors)
        return errors or None
    finally:
        pgc.destroy_validation_report(c_report)
