# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import json
from ctypes import cast, create_string_buffer

import numpy as np
import pytest

from power_grid_model import AttributeType as AT, ComponentType as CT, DatasetType, initialize_array
from power_grid_model._core.power_grid_core import (
    CharDoublePtr,
    CharPtr,
    IdxPtr,
    ValidationResultPtr,
    get_power_grid_core,
)
from power_grid_model.validation._validation_report import (
    _decode_validation_report,
    _UnknownValidationError,
    _validate_input_data_native,
)
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


def _make_report(*issues: dict[str, object], schema_version: object = 1) -> bytes:
    return json.dumps({"schema_version": schema_version, "issues": issues}).encode("utf-8")


def _make_issue(kind: str, **attributes: object) -> dict[str, object]:
    issue: dict[str, object] = {
        "kind": kind,
        "component": "node",
        "field": "id" if kind == "not_unique" else "u_rated",
        "ids": [2, 2],
        "details": {},
    }
    issue.update(attributes)
    return issue


@pytest.mark.parametrize(
    ("kind", "error_type", "details"),
    [
        ("missing_value", MissingValueError, {}),
        ("infinity", InfinityError, {}),
        ("not_unique", NotUniqueError, {}),
        ("not_greater_than", NotGreaterThanError, {"ref_value": 0}),
    ],
)
def test_decode_validation_report_maps_known_kinds(kind: str, error_type: type[ValidationError], details):
    errors = _decode_validation_report(_make_report(_make_issue(kind, details=details)))

    assert errors is not None
    assert len(errors) == 1
    assert type(errors[0]) is error_type
    assert errors[0].component == CT.node
    assert errors[0].field == (AT.id if kind == "not_unique" else AT.u_rated)
    assert errors[0].ids == [2, 2]
    if isinstance(errors[0], NotGreaterThanError):
        assert errors[0].ref_value == 0


def test_decode_empty_report_returns_none():
    assert _decode_validation_report(_make_report()) is None


def test_decode_known_issue_ignores_additive_attributes_and_preserves_sentinel():
    issue = _make_issue("missing_value", field="id", ids=[-2147483648], added_attribute="ignored")
    issue["details"] = {"future_attribute": True}

    errors = _decode_validation_report(_make_report(issue))

    assert errors == [MissingValueError(CT.node, AT.id, [-2147483648])]


def test_decode_known_kind_for_another_component_and_field():
    issue = _make_issue("infinity", component="line", field="r1")

    errors = _decode_validation_report(_make_report(issue))

    assert errors == [InfinityError(CT.line, AT.r1, [2, 2])]


def test_decode_line_validation_issue_kinds():
    boolean_issue = _make_issue("not_boolean", component="line", field="from_status", ids=[3])
    pair_issue = _make_issue(
        "two_values_zero",
        component="line",
        field="r1",
        ids=[4],
        fields=[{"component": "line", "field": "r1"}, {"component": "line", "field": "x1"}],
    )
    reference_issue = _make_issue(
        "invalid_id_reference",
        component="line",
        field="from_node",
        ids=[5],
        reference_component="node",
    )
    cross_component_issue = _make_issue(
        "cross_component_not_unique",
        component="line",
        field="id",
        ids=None,
        fields=[{"component": "line", "field": "id"}, {"component": "node", "field": "id"}],
        objects=[{"component": "line", "id": 1}, {"component": "node", "id": 1}],
    )

    errors = _decode_validation_report(_make_report(boolean_issue, pair_issue, reference_issue, cross_component_issue))

    assert errors == [
        NotBooleanError(CT.line, AT.from_status, [3]),
        TwoValuesZeroError(CT.line, [AT.r1, AT.x1], [4]),
        InvalidIdError(CT.line, AT.from_node, [5], CT.node),
        MultiComponentNotUniqueError([(CT.line, AT.id), (CT.node, AT.id)], [(CT.line, 1), (CT.node, 1)]),
    ]


def test_decode_nullable_ids():
    issue = _make_issue("missing_value", ids=None)

    errors = _decode_validation_report(_make_report(issue))

    assert errors == [MissingValueError(CT.node, AT.u_rated, None)]


@pytest.mark.parametrize(
    ("ref_value", "expected"),
    [
        (1.5, 1.5),
        ("zero", "zero"),
        (["u_min", "u_max"], ("u_min", "u_max")),
        ({"type": "float", "value": "positive_infinity"}, float("inf")),
    ],
)
def test_decode_comparison_reference_values(ref_value, expected):
    issue = _make_issue("not_greater_than", details={"ref_value": ref_value})

    errors = _decode_validation_report(_make_report(issue))

    assert errors == [NotGreaterThanError(CT.node, AT.u_rated, [2, 2], expected)]


def test_decode_unknown_kind_preserves_raw_issue():
    issue = {"kind": "future_rule", "new_payload": {"value": 7}}

    errors = _decode_validation_report(_make_report(issue))

    assert errors is not None
    assert len(errors) == 1
    assert isinstance(errors[0], _UnknownValidationError)
    assert errors[0].raw_issue == issue
    assert "future_rule" in str(errors[0])


@pytest.mark.parametrize(
    "report_data",
    [
        b"not json",
        b"\xff",
        b'{"schema_version":1,"issues":[NaN]}',
        _make_report(schema_version=True),
        _make_report(schema_version=2),
        json.dumps({"schema_version": 1, "issues": {}}).encode(),
        _make_report(_make_issue("infinity", ids=[True])),
        _make_report(_make_issue("missing_value", ids=[])),
        _make_report(_make_issue("missing_value", ids=[2**31])),
        _make_report(_make_issue("missing_value", ids=[-(2**31) - 1])),
        _make_report(_make_issue("not_greater_than", details={})),
        _make_report(_make_issue("not_greater_than", details={"ref_value": False})),
        _make_report(_make_issue("not_greater_than", details={"ref_value": float("inf")})),
    ],
)
def test_decode_validation_report_rejects_invalid_payloads(report_data: bytes):
    with pytest.raises(ValueError, match=r".+"):
        _decode_validation_report(report_data)


def _valid_node_dataset():
    nodes = initialize_array(DatasetType.input, CT.node, 1)
    nodes[AT.id] = [1]
    nodes[AT.u_rated] = [100.0]
    return {CT.node: nodes}


def _track_native_result_destruction(monkeypatch):
    core = get_power_grid_core()
    destroy_result = core.destroy_validation_result
    destroyed_results = []

    def track_destroy(result):
        destroyed_results.append(result)
        destroy_result(result)

    monkeypatch.setattr(core, "destroy_validation_result", track_destroy)
    return core, destroyed_results


def test_native_adapter_decodes_report_and_destroys_result(monkeypatch):
    core, destroyed_results = _track_native_result_destruction(monkeypatch)

    def fail_get_report(*_args):
        raise AssertionError("a passed result should not be serialized")

    monkeypatch.setattr(core, "validation_result_get_json", fail_get_report)
    errors = _validate_input_data_native(_valid_node_dataset())

    assert errors is None
    assert len(destroyed_results) == 1
    core.clear_error()


@pytest.mark.parametrize(("symmetric", "expected"), [(True, 1), (False, 0)])
def test_native_adapter_passes_symmetry_to_core(monkeypatch, symmetric: bool, expected: int):
    core = get_power_grid_core()
    core.clear_error()
    received_symmetry = []

    def validate_input_data(_dataset, native_symmetric):
        received_symmetry.append(native_symmetric)
        return ValidationResultPtr(1)

    monkeypatch.setattr(core, "validate_input_data", validate_input_data)
    monkeypatch.setattr(core, "validation_result_passed", lambda _result: 1)
    monkeypatch.setattr(core, "destroy_validation_result", lambda _result: None)

    assert _validate_input_data_native(_valid_node_dataset(), symmetric=symmetric) is None
    assert received_symmetry == [expected]
    core.clear_error()


def test_native_adapter_destroys_result_when_getter_fails(monkeypatch):
    core, destroyed_results = _track_native_result_destruction(monkeypatch)

    def fail_get_report(*_args):
        raise RuntimeError("getter failed")

    monkeypatch.setattr(core, "validation_result_get_json", fail_get_report)
    nodes = initialize_array(DatasetType.input, CT.node, 1)
    nodes[AT.id] = [1]
    nodes[AT.u_rated] = [np.nan]
    with pytest.raises(RuntimeError, match="getter failed"):
        _validate_input_data_native({CT.node: nodes})

    assert len(destroyed_results) == 1
    core.clear_error()


def test_native_adapter_destroys_result_when_json_decode_fails(monkeypatch):
    core, destroyed_results = _track_native_result_destruction(monkeypatch)
    invalid_report = b"not json"
    report_buffer = create_string_buffer(invalid_report)

    def set_invalid_report(_result, data_pointer, size_pointer):
        cast(data_pointer, CharDoublePtr)[0] = cast(report_buffer, CharPtr)
        cast(size_pointer, IdxPtr)[0] = len(invalid_report)

    monkeypatch.setattr(core, "validation_result_get_json", set_invalid_report)
    nodes = initialize_array(DatasetType.input, CT.node, 1)
    nodes[AT.id] = [1]
    nodes[AT.u_rated] = [np.nan]
    with pytest.raises(ValueError, match="not valid JSON"):
        _validate_input_data_native({CT.node: nodes})

    assert len(destroyed_results) == 1
    core.clear_error()


def test_native_adapter_decodes_node_findings():
    nodes = initialize_array(DatasetType.input, CT.node, 5)
    nodes[AT.id] = [np.iinfo(np.int32).min, 4, 4, 9, 10]
    nodes[AT.u_rated] = [np.nan, np.inf, -np.inf, 0.0, -1.0]
    original_nodes = nodes.copy()

    errors = _validate_input_data_native({CT.node: nodes})

    assert errors is not None
    assert [type(error) for error in errors] == [
        MissingValueError,
        MissingValueError,
        InfinityError,
        NotUniqueError,
        NotGreaterThanError,
    ]
    assert errors[2].ids == [4, 4]
    assert isinstance(errors[4], NotGreaterThanError)
    assert errors[4].ref_value == 0
    np.testing.assert_array_equal(nodes[AT.id], original_nodes[AT.id])
    np.testing.assert_equal(nodes[AT.u_rated], original_nodes[AT.u_rated])


def test_native_adapter_accepts_columnar_node_data():
    nodes = {
        AT.id: np.array([1, 2, 3], dtype=np.int32),
        AT.u_rated: np.array([np.inf, -np.inf, 100.0], dtype=np.float64),
    }
    original_nodes = {attribute: values.copy() for attribute, values in nodes.items()}

    errors = _validate_input_data_native({CT.node: nodes})

    assert errors == [
        InfinityError(CT.node, AT.u_rated, [1, 2]),
        NotGreaterThanError(CT.node, AT.u_rated, [2], 0),
    ]
    for attribute, values in nodes.items():
        np.testing.assert_array_equal(values, original_nodes[attribute])
