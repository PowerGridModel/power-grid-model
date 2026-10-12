# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import json
from ctypes import cast, create_string_buffer

import numpy as np
import pytest

from power_grid_model import AttributeType as AT, ComponentType as CT, DatasetType, initialize_array
from power_grid_model._core.options import Options
from power_grid_model._core.power_grid_core import (
    CharDoublePtr,
    CharPtr,
    IdxPtr,
    ValidationResultPtr,
    get_power_grid_core,
)
from power_grid_model._core.utils import compatibility_convert_row_columnar_dataset
from power_grid_model.enum import ComponentAttributeFilterOptions
from power_grid_model.validation import validate_batch_data_json, validate_input_data_json

_SCHEMA_VERSION = 3


def _valid_node_dataset():
    nodes = initialize_array(DatasetType.input, CT.node, 1)
    nodes[AT.id] = [1]
    nodes[AT.u_rated] = [100.0]
    return {CT.node: nodes}


def _valid_node_line_dataset():
    nodes = initialize_array(DatasetType.input, CT.node, 2)
    nodes[AT.id] = [1, 2]
    nodes[AT.u_rated] = 100.0
    lines = initialize_array(DatasetType.input, CT.line, 1)
    lines[AT.id] = [3]
    lines[AT.from_node] = [1]
    lines[AT.to_node] = [2]
    lines[AT.from_status] = [1]
    lines[AT.to_status] = [1]
    lines[AT.r1] = [0.1]
    lines[AT.x1] = [0.2]
    lines[AT.c1] = [0.0]
    lines[AT.tan1] = [0.0]
    lines[AT.i_n] = [100.0]
    return {CT.node: nodes, CT.line: lines}


def _track_native_result_destruction(monkeypatch):
    core = get_power_grid_core()
    destroy_result = core.destroy_validation_result
    destroyed_results = []

    def track_destroy(result):
        destroyed_results.append(result)
        destroy_result(result)

    monkeypatch.setattr(core, "destroy_validation_result", track_destroy)
    return core, destroyed_results


def test_input_json_adapter_returns_pass_report_and_destroys_result(monkeypatch):
    core, destroyed_results = _track_native_result_destruction(monkeypatch)

    report_bytes = validate_input_data_json(_valid_node_dataset())

    assert isinstance(report_bytes, bytes)
    assert json.loads(report_bytes) == {
        "schema_version": 3,
        "validation_type": "input",
        "valid": True,
        "results": [],
    }
    assert len(destroyed_results) == 1
    core.clear_error()


def test_input_json_adapter_returns_findings_without_reconstructing_exceptions():
    nodes = initialize_array(DatasetType.input, CT.node, 1)
    nodes[AT.id] = [np.iinfo(np.int32).min]
    nodes[AT.u_rated] = [np.nan]
    original_nodes = nodes.copy()

    report = json.loads(validate_input_data_json({CT.node: nodes}))

    assert report["valid"] is False
    missing_issues = [issue for issue in report["results"][0]["issues"] if issue["kind"] == "missing_value"]
    assert [issue["kind"] for issue in missing_issues] == ["missing_value", "missing_value"]
    assert {(issue["component"], issue["field"], issue["id"], issue["row"]) for issue in missing_issues} == {
        ("node", "id", None, 0),
        ("node", "u_rated", None, 0),
    }
    np.testing.assert_array_equal(nodes[AT.id], original_nodes[AT.id])
    np.testing.assert_equal(nodes[AT.u_rated], original_nodes[AT.u_rated])


@pytest.mark.parametrize("symmetric", [True, False])
def test_native_json_adapter_passes_options_to_core(monkeypatch, symmetric: bool):
    core = get_power_grid_core()
    core.clear_error()
    options = Options()
    options.symmetric = symmetric
    received_options = []
    report_bytes = b"{}"
    report_buffer = create_string_buffer(report_bytes)

    def validate_input_data(_dataset, native_options):
        received_options.append(native_options.value)
        return ValidationResultPtr(1)

    def set_report(_result, data_pointer, size_pointer):
        cast(data_pointer, CharDoublePtr)[0] = cast(report_buffer, CharPtr)
        cast(size_pointer, IdxPtr)[0] = len(report_bytes)

    monkeypatch.setattr(core, "validate_input_data", validate_input_data)
    monkeypatch.setattr(core, "validation_result_get_json", set_report)
    monkeypatch.setattr(core, "destroy_validation_result", lambda _result: None)

    assert validate_input_data_json(_valid_node_dataset(), options=options) == report_bytes
    assert received_options == [options.opt.value]
    core.clear_error()


@pytest.mark.parametrize("update_format", ["row", "columnar", "sparse"])
def test_native_batch_json_adapter_reports_only_failures(update_format: str):
    input_data = _valid_node_line_dataset()
    node_updates = initialize_array(DatasetType.update, CT.node, (3, 1))
    node_updates[AT.id] = [[1], [1], [99]]
    line_updates = initialize_array(DatasetType.update, CT.line, (3, 1))
    line_updates[AT.id] = [[3], [3], [99]]
    line_updates[AT.from_status] = [[2], [np.iinfo(np.int8).min], [np.iinfo(np.int8).min]]
    update_data = {CT.node: node_updates, CT.line: line_updates}
    if update_format == "columnar":
        update_data = compatibility_convert_row_columnar_dataset(
            update_data, ComponentAttributeFilterOptions.everything, DatasetType.update
        )
    elif update_format == "sparse":
        sparse_nodes = initialize_array(DatasetType.update, CT.node, 2)
        sparse_nodes[AT.id] = [1, 99]
        sparse_lines = initialize_array(DatasetType.update, CT.line, 2)
        sparse_lines[AT.id] = [3, 99]
        sparse_lines[AT.from_status] = [2, np.iinfo(np.int8).min]
        indptr = np.array([0, 1, 1, 2], dtype=np.int64)
        update_data = {
            CT.node: {"indptr": indptr, "data": sparse_nodes},
            CT.line: {"indptr": indptr, "data": sparse_lines},
        }

    report = json.loads(validate_batch_data_json(input_data, update_data))

    assert report["schema_version"] == _SCHEMA_VERSION
    assert report["validation_type"] == "batch"
    assert report["valid"] is False
    results = report["results"]
    assert [result["scenario"] for result in results] == [0, 2]
    assert all("status" not in result for result in results)
    assert results[0]["issues"][0]["kind"] == "not_boolean"
    assert results[0]["issues"][0]["row"] == 0
    assert [issue["kind"] for issue in results[1]["issues"]] == ["id_not_in_dataset", "id_not_in_dataset"]
    assert {(issue["component"], issue["id"], issue["row"]) for issue in results[1]["issues"]} == {
        ("node", 99, 0),
        ("line", 99, 0),
    }


def test_native_batch_json_adapter_keeps_zero_scenario_report_empty():
    input_data = _valid_node_dataset()
    input_data[CT.node][AT.u_rated] = [np.nan]

    report = json.loads(validate_batch_data_json(input_data, {}))

    assert report == {
        "schema_version": 3,
        "validation_type": "batch",
        "valid": True,
        "results": [],
    }


def test_native_adapter_destroys_result_when_json_getter_fails(monkeypatch):
    core, destroyed_results = _track_native_result_destruction(monkeypatch)

    def fail_get_report(*_args):
        raise RuntimeError("getter failed")

    monkeypatch.setattr(core, "validation_result_get_json", fail_get_report)
    nodes = initialize_array(DatasetType.input, CT.node, 1)
    nodes[AT.id] = [1]
    nodes[AT.u_rated] = [np.nan]
    with pytest.raises(RuntimeError, match="getter failed"):
        validate_input_data_json({CT.node: nodes})

    assert len(destroyed_results) == 1
    core.clear_error()


def test_native_adapter_returns_report_bytes_without_decoding_json(monkeypatch):
    core, destroyed_results = _track_native_result_destruction(monkeypatch)
    report_bytes = b"not parsed by the adapter"
    report_buffer = create_string_buffer(report_bytes)

    def set_report(_result, data_pointer, size_pointer):
        cast(data_pointer, CharDoublePtr)[0] = cast(report_buffer, CharPtr)
        cast(size_pointer, IdxPtr)[0] = len(report_bytes)

    monkeypatch.setattr(core, "validation_result_get_json", set_report)

    result = validate_input_data_json(_valid_node_dataset())

    assert result == report_bytes
    assert len(destroyed_results) == 1
    core.clear_error()
