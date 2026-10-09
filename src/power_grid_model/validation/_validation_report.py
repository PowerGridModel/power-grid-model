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
from power_grid_model._core.options import Options
from power_grid_model._core.power_grid_core import CharPtr, ConstDatasetPtr, get_power_grid_core
from power_grid_model._core.power_grid_dataset import CConstDataset
from power_grid_model.data_types import Dataset
from power_grid_model.validation.errors import (
    IdNotInDatasetError,
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


def _decode_report_envelope(report_data: bytes) -> dict[str, Any]:
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
    return report


_BATCH_ISSUE_KINDS = {"id_not_in_dataset", "invalid_update_id"}


def _decode_issue_array(raw_issues: list[Any], location: str = "issue") -> list[ValidationError]:
    errors: list[ValidationError] = []
    for index, raw_issue in enumerate(raw_issues):
        issue = _as_object(raw_issue, f"{location} at index {index}")
        kind = issue.get("kind")
        if not isinstance(kind, str):
            raise ValueError(f"Validation report {location} at index {index} must have a string kind.")
        if kind in _ISSUE_CONSTRUCTORS or kind in _BATCH_ISSUE_KINDS:
            errors.append(_decode_known_issue(kind, issue))
        else:
            errors.append(_UnknownValidationError(issue))
    return errors


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


def _decode_batch_issue(
    kind: str,
    issue: dict[str, Any],
    component: ComponentType,
    field: AttributeType,
    raw_ids: list[int] | None,
) -> ValidationError:
    if kind == "id_not_in_dataset":
        raw_ref_dataset = _as_object(issue["details"], "issue 'id_not_in_dataset' details").get("ref_dataset")
        if raw_ids is None or not isinstance(raw_ref_dataset, str):
            raise ValueError("Validation issue 'id_not_in_dataset' requires IDs and a reference dataset.")
        try:
            ref_dataset = DatasetType(raw_ref_dataset)
        except ValueError:
            return _UnknownValidationError(issue)
        return IdNotInDatasetError(component, raw_ids, ref_dataset)
    if raw_ids is not None:
        raise ValueError("Validation issue 'invalid_update_id' requires null IDs.")
    return InvalidIdError(component, field, None)


def _decode_known_issue_value(
    issue: dict[str, Any],
    component: ComponentType,
    field: AttributeType,
    raw_ids: list[int] | None,
) -> ValidationError:
    kind = issue["kind"]
    details = _as_object(issue["details"], f"issue {kind!r} details")
    if kind == "not_greater_than":
        if "ref_value" not in details:
            raise ValueError("Validation issue 'not_greater_than' requires details.ref_value.")
        return _ISSUE_CONSTRUCTORS[kind](component, field, raw_ids, _decode_ref_value(details["ref_value"]))

    extended_error = _decode_extended_issue(kind, issue, component, field, raw_ids)
    if extended_error is not None:
        return extended_error
    if kind in _BATCH_ISSUE_KINDS:
        return _decode_batch_issue(kind, issue, component, field, raw_ids)
    return _ISSUE_CONSTRUCTORS[kind](component, field, raw_ids)


def _decode_known_issue(kind: str, issue: dict[str, Any]) -> ValidationError:
    required = ("component", "field", "ids", "details")
    missing = [key for key in required if key not in issue]
    if missing:
        raise ValueError(f"Validation issue {kind!r} is missing required attributes: {', '.join(missing)}.")

    raw_component = issue["component"]
    raw_field = issue["field"]
    raw_ids = issue["ids"]
    if not isinstance(raw_component, str):
        raise ValueError(f"Validation issue {kind!r} component must be a string.")
    if not isinstance(raw_field, str):
        raise ValueError(f"Validation issue {kind!r} field must be a string.")
    if raw_ids is not None and (
        not isinstance(raw_ids, list) or not raw_ids or any(not _is_valid_id(identifier) for identifier in raw_ids)
    ):
        raise ValueError(f"Validation issue {kind!r} ids must be null or a non-empty array of signed IDs.")

    try:
        component = ComponentType(raw_component)
        field = AttributeType(raw_field)
    except ValueError:
        return _UnknownValidationError(issue)

    return _decode_known_issue_value(issue, component, field, raw_ids)


def _decode_validation_report(report_data: bytes) -> list[ValidationError] | None:
    """Decode a v1 report into existing Python validation errors."""
    report = _decode_report_envelope(report_data)
    raw_issues = report.get("issues")
    if not isinstance(raw_issues, list):
        raise ValueError("Validation report issues must be a JSON array.")
    return _decode_issue_array(raw_issues) or None


def _decode_batch_validation_report(report_data: bytes) -> dict[int, list[ValidationError]] | None:
    """Decode a v1 scenario-grouped batch validation report."""
    report = _decode_report_envelope(report_data)
    if report.get("is_batch") is not True:
        raise ValueError("Validation report is not a batch report.")
    scenarios = report.get("data")
    if not isinstance(scenarios, list):
        raise ValueError("Batch validation report data must be a JSON array.")

    batch_errors: dict[int, list[ValidationError]] = {}
    for scenario_index, raw_scenario in enumerate(scenarios):
        scenario = _as_object(raw_scenario, f"scenario at index {scenario_index}")
        errors: list[ValidationError] = []
        for component in scenario:
            if component not in {"node", "line"}:
                raise ValueError(f"Batch validation report contains unsupported component {component!r}.")
        for component in ("node", "line"):
            raw_issues = scenario.get(component, [])
            if not isinstance(raw_issues, list):
                raise ValueError(f"Batch validation report component {component!r} must contain an issue array.")
            errors.extend(_decode_issue_array(raw_issues, f"scenario {scenario_index} component {component!r} issue"))
        if errors:
            batch_errors[scenario_index] = errors
    return batch_errors or None


def _native_validation_report(core: Any, result: Any) -> bytes | None:
    assert_no_error()
    if not result:
        raise RuntimeError("Native validation returned no result without setting a handle error.")

    passed = core.validation_result_passed(result)
    assert_no_error()
    if passed:
        return None

    report_data = CharPtr()
    report_size = IdxC()
    core.validation_result_get_json(result, byref(report_data), byref(report_size))
    assert_no_error()
    if not report_data or report_size.value < 0:
        raise RuntimeError("Native validation returned an invalid JSON report buffer.")
    raw_report = report_data[: report_size.value]
    if not isinstance(raw_report, bytes):
        raise RuntimeError("Native validation returned non-byte report data.")
    return raw_report


def _validate_input_data_native(input_data: Dataset, options: Options | None = None) -> list[ValidationError] | None:
    """Validate a supported input dataset using the experimental C API."""
    native_dataset = CConstDataset(data=input_data, dataset_type=DatasetType.input)
    core = get_power_grid_core()
    options = options or Options()
    result = core.validate_input_data(native_dataset.get_dataset_ptr(), options.opt)
    try:
        raw_report = _native_validation_report(core, result)
        return None if raw_report is None else _decode_validation_report(raw_report)
    finally:
        if result:
            core.destroy_validation_result(result)


def _create_empty_batch_update_dataset(core: Any) -> ConstDatasetPtr:
    mutable_dataset = core.create_dataset_mutable(DatasetType.update.value, 1, 0)
    const_dataset = ConstDatasetPtr()
    try:
        assert_no_error()
        if not mutable_dataset:
            raise RuntimeError("Native update dataset creation returned no dataset without setting a handle error.")
        const_dataset = core.create_dataset_const_from_mutable(mutable_dataset)
        assert_no_error()
        if not const_dataset:
            raise RuntimeError("Native update dataset conversion returned no dataset without setting a handle error.")
        return const_dataset
    except Exception:
        if const_dataset:
            core.destroy_dataset_const(const_dataset)
        raise
    finally:
        if mutable_dataset:
            core.destroy_dataset_mutable(mutable_dataset)


def _validate_batch_data_native(
    input_data: Dataset, update_data: Dataset, options: Options | None = None
) -> dict[int, list[ValidationError]] | None:
    """Validate node and line data against each update scenario using the experimental C API."""
    native_input = CConstDataset(data=input_data, dataset_type=DatasetType.input)
    core = get_power_grid_core()
    options = options or Options()
    empty_update_dataset = None
    native_updates = None
    try:
        if update_data:
            native_updates = CConstDataset(data=update_data, dataset_type=DatasetType.update)
            update_dataset_pointer = native_updates.get_dataset_ptr()
        else:
            empty_update_dataset = _create_empty_batch_update_dataset(core)
            update_dataset_pointer = empty_update_dataset

        result = core.validate_batch_data(native_input.get_dataset_ptr(), update_dataset_pointer, options.opt)
        try:
            raw_report = _native_validation_report(core, result)
            return None if raw_report is None else _decode_batch_validation_report(raw_report)
        finally:
            if result:
                core.destroy_validation_result(result)
    finally:
        if empty_update_dataset:
            core.destroy_dataset_const(empty_update_dataset)
