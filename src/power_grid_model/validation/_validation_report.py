# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""Private adapter for the experimental native validation report."""

import json
from collections.abc import Callable
from ctypes import byref, sizeof
from math import isfinite
from typing import Any, NoReturn

from power_grid_model._core.dataset_definitions import AttributeType, ComponentType, DatasetType
from power_grid_model._core.error_handling import assert_no_error
from power_grid_model._core.index_integer import IdC, IdxC
from power_grid_model._core.power_grid_core import CharPtr, get_power_grid_core
from power_grid_model._core.power_grid_dataset import CConstDataset
from power_grid_model.data_types import Dataset
from power_grid_model.validation.errors import (
    InfinityError,
    InvalidIdError,
    MissingValueError,
    MultiComponentNotUniqueError,
    NotBooleanError,
    NotGreaterThanError,
    NotUniqueError,
    TwoValuesZeroError,
    ValidationError,
)

_SCHEMA_VERSION = 1
_ISSUE_CONSTRUCTORS: dict[str, Callable[..., ValidationError]] = {
    "missing_value": MissingValueError,
    "infinity": InfinityError,
    "not_unique": NotUniqueError,
    "not_greater_than": NotGreaterThanError,
    "not_boolean": NotBooleanError,
    "two_values_zero": TwoValuesZeroError,
    "invalid_id_reference": InvalidIdError,
    "cross_component_not_unique": MultiComponentNotUniqueError,
}
_ID_LIMIT = 1 << (sizeof(IdC) * 8 - 1)
_MIN_REFERENCES = 2


def _is_valid_id(value: Any) -> bool:
    return type(value) is int and -_ID_LIMIT <= value < _ID_LIMIT


def _decode_field_references(value: Any, kind: str) -> list[tuple[ComponentType, AttributeType]] | None:
    if not isinstance(value, list) or len(value) < _MIN_REFERENCES:
        raise ValueError(f"Validation issue {kind!r} fields must contain at least two field references.")
    fields: list[tuple[ComponentType, AttributeType]] = []
    for index, raw_reference in enumerate(value):
        reference = _as_object(raw_reference, f"issue {kind!r} field at index {index}")
        raw_component = reference.get("component")
        raw_field = reference.get("field")
        if not isinstance(raw_component, str) or not isinstance(raw_field, str):
            raise ValueError(f"Validation issue {kind!r} field references require component and field strings.")
        try:
            fields.append((ComponentType(raw_component), AttributeType(raw_field)))
        except ValueError:
            return None
    return fields


def _decode_object_references(value: Any, kind: str) -> list[tuple[ComponentType, int]] | None:
    if not isinstance(value, list) or len(value) < _MIN_REFERENCES:
        raise ValueError(f"Validation issue {kind!r} objects must contain at least two object references.")
    objects: list[tuple[ComponentType, int]] = []
    for index, raw_reference in enumerate(value):
        reference = _as_object(raw_reference, f"issue {kind!r} object at index {index}")
        raw_component = reference.get("component")
        raw_id = reference.get("id")
        if not isinstance(raw_component, str) or not _is_valid_id(raw_id):
            raise ValueError(f"Validation issue {kind!r} object references require a component and signed ID.")
        try:
            objects.append((ComponentType(raw_component), raw_id))
        except ValueError:
            return None
    return objects


def _decode_two_values_zero(
    issue: dict[str, Any], component: ComponentType, raw_ids: list[int] | None
) -> ValidationError:
    fields = _decode_field_references(issue.get("fields"), "two_values_zero")
    if fields is None:
        return _UnknownValidationError(issue)
    if any(field_component != component for field_component, _ in fields):
        raise ValueError("Validation issue 'two_values_zero' fields must belong to its component.")
    if raw_ids is None:
        raise ValueError("Validation issue 'two_values_zero' requires IDs.")
    return TwoValuesZeroError(component, [field for _, field in fields], raw_ids)


def _decode_invalid_id_reference(
    issue: dict[str, Any], component: ComponentType, field: AttributeType, raw_ids: list[int] | None
) -> ValidationError:
    raw_reference_component = issue.get("reference_component")
    if not isinstance(raw_reference_component, str):
        raise ValueError("Validation issue 'invalid_id_reference' requires a reference component.")
    try:
        reference_component = ComponentType(raw_reference_component)
    except ValueError:
        return _UnknownValidationError(issue)
    return InvalidIdError(component, field, raw_ids, reference_component)


def _decode_cross_component_not_unique(issue: dict[str, Any], raw_ids: list[int] | None) -> ValidationError:
    if raw_ids is not None:
        raise ValueError("Validation issue 'cross_component_not_unique' must use component-tagged objects.")
    fields = _decode_field_references(issue.get("fields"), "cross_component_not_unique")
    objects = _decode_object_references(issue.get("objects"), "cross_component_not_unique")
    if fields is None or objects is None:
        return _UnknownValidationError(issue)
    return MultiComponentNotUniqueError(fields, objects)


def _decode_extended_issue(
    kind: str,
    issue: dict[str, Any],
    component: ComponentType,
    field: AttributeType,
    raw_ids: list[int] | None,
) -> ValidationError | None:
    if kind == "not_boolean":
        return NotBooleanError(component, field, raw_ids)
    if kind == "two_values_zero":
        return _decode_two_values_zero(issue, component, raw_ids)
    if kind == "invalid_id_reference":
        return _decode_invalid_id_reference(issue, component, field, raw_ids)
    if kind == "cross_component_not_unique":
        return _decode_cross_component_not_unique(issue, raw_ids)
    return None


class _UnknownValidationError(ValidationError):
    __hash__ = None  # type: ignore[assignment]

    def __init__(self, issue: dict[str, Any]):
        self.kind: str = issue["kind"]
        self.raw_issue = issue

    def __str__(self) -> str:
        return f"Unknown validation finding {self.kind!r}."

    def __eq__(self, other: object) -> bool:
        return isinstance(other, _UnknownValidationError) and self.raw_issue == other.raw_issue


def _reject_json_constant(value: str) -> NoReturn:
    raise ValueError(f"Invalid non-standard JSON constant: {value}")


def _as_object(value: Any, location: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ValueError(f"Validation report {location} must be a JSON object.")
    return value


def _decode_ref_value_scalar(value: Any) -> int | float | str:
    if type(value) in {int, str}:
        return value
    if type(value) is float:
        if isfinite(value):
            return value
    elif isinstance(value, dict) and value.get("type") == "float":
        special_value = value.get("value")
        special_values = {
            "nan": float("nan"),
            "positive_infinity": float("inf"),
            "negative_infinity": float("-inf"),
        }
        if isinstance(special_value, str) and special_value in special_values:
            return special_values[special_value]
    raise ValueError("Validation reference values must be finite scalars or tagged non-finite floats.")


def _decode_ref_value(value: Any) -> int | float | str | tuple[int | float | str, ...]:
    if isinstance(value, list):
        return tuple(_decode_ref_value_scalar(item) for item in value)
    return _decode_ref_value_scalar(value)


def _decode_known_issue(kind: str, issue: dict[str, Any]) -> ValidationError:
    required = ("component", "field", "ids", "details")
    missing = [key for key in required if key not in issue]
    if missing:
        raise ValueError(f"Validation issue {kind!r} is missing required attributes: {', '.join(missing)}.")

    raw_component = issue["component"]
    raw_field = issue["field"]
    raw_ids = issue["ids"]
    details = _as_object(issue["details"], f"issue {kind!r} details")
    if not isinstance(raw_component, str):
        raise ValueError(f"Validation issue {kind!r} component must be a string.")
    if not isinstance(raw_field, str):
        raise ValueError(f"Validation issue {kind!r} field must be a string.")
    if raw_ids is not None and (
        not isinstance(raw_ids, list) or not raw_ids or any(not _is_valid_id(identifier) for identifier in raw_ids)
    ):
        raise ValueError(f"Validation issue {kind!r} ids must be null or a non-empty array of signed IDs.")

    ref_value = None
    if kind == "not_greater_than":
        if "ref_value" not in details:
            raise ValueError("Validation issue 'not_greater_than' requires details.ref_value.")
        ref_value = _decode_ref_value(details["ref_value"])

    try:
        component = ComponentType(raw_component)
        field = AttributeType(raw_field)
    except ValueError:
        return _UnknownValidationError(issue)

    extended_error = _decode_extended_issue(kind, issue, component, field, raw_ids)
    if extended_error is not None:
        return extended_error

    constructor = _ISSUE_CONSTRUCTORS[kind]
    if kind == "not_greater_than":
        return constructor(component, field, raw_ids, ref_value)
    return constructor(component, field, raw_ids)


def _decode_validation_report(report_data: bytes) -> list[ValidationError] | None:
    """Decode a v1 report into existing Python validation errors."""
    try:
        report_text = report_data.decode("utf-8")
    except UnicodeDecodeError as error:
        raise ValueError("Validation report must be UTF-8.") from error

    try:
        raw_report = json.loads(report_text, parse_constant=_reject_json_constant)
    except json.JSONDecodeError as error:
        raise ValueError("Validation report is not valid JSON.") from error

    report = _as_object(raw_report, "envelope")
    version = report.get("schema_version")
    if type(version) is not int or version != _SCHEMA_VERSION:
        raise ValueError(f"Unsupported validation report schema version: {version!r}.")
    raw_issues = report.get("issues")
    if not isinstance(raw_issues, list):
        raise ValueError("Validation report issues must be a JSON array.")

    errors: list[ValidationError] = []
    for index, raw_issue in enumerate(raw_issues):
        issue = _as_object(raw_issue, f"issue at index {index}")
        kind = issue.get("kind")
        if not isinstance(kind, str):
            raise ValueError(f"Validation report issue at index {index} must have a string kind.")
        if kind in _ISSUE_CONSTRUCTORS:
            errors.append(_decode_known_issue(kind, issue))
        else:
            errors.append(_UnknownValidationError(issue))

    return errors or None


def _validate_input_data_native(input_data: Dataset, symmetric: bool = True) -> list[ValidationError] | None:
    """Validate a supported input dataset using the experimental C API."""
    native_dataset = CConstDataset(data=input_data, dataset_type=DatasetType.input)
    core = get_power_grid_core()
    result = core.validate_input_data(native_dataset.get_dataset_ptr(), int(symmetric))
    try:
        assert_no_error()
        if not result:
            raise RuntimeError("Native input validation returned no result without setting a handle error.")

        passed = core.validation_result_passed(result)
        assert_no_error()
        if passed:
            return None

        report_data = CharPtr()
        report_size = IdxC()
        core.validation_result_get_json(result, byref(report_data), byref(report_size))
        assert_no_error()
        if not report_data or report_size.value < 0:
            raise RuntimeError("Native input validation returned an invalid JSON report buffer.")
        raw_report = report_data[: report_size.value]
        if not isinstance(raw_report, bytes):
            raise RuntimeError("Native input validation returned non-byte report data.")
        return _decode_validation_report(raw_report)
    finally:
        if result:
            core.destroy_validation_result(result)
