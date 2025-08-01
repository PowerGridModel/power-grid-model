# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import json
from pathlib import Path
from unittest.mock import MagicMock, mock_open, patch

import numpy as np
import pytest

from power_grid_model import DatasetType, initialize_array
from power_grid_model._core.dataset_definitions import ComponentType
from power_grid_model._core.power_grid_meta import power_grid_meta_data
from power_grid_model.data_types import Dataset
from power_grid_model.utils import (
    LICENSE_TEXT,
    _make_test_case,
    get_component_batch_size,
    get_dataset_batch_size,
    get_dataset_scenario,
    json_deserialize_from_file,
    json_serialize_to_file,
    msgpack_deserialize_from_file,
    msgpack_serialize_to_file,
    self_test,
)


def test_get_dataset_scenario():
    data = {
        "foo": np.array([["bar", "baz"], ["foobar", "foobaz"]]),
        "hi": {
            "data": np.array(["hello", "hey"]),
            "indptr": np.array([0, 0, 2]),
        },
    }
    result = get_dataset_scenario(data, 0)
    assert result.keys() == data.keys()
    np.testing.assert_array_equal(result["foo"], data["foo"][0])
    np.testing.assert_array_equal(result["hi"], data["hi"]["data"][0:0])

    result = get_dataset_scenario(data, 1)
    assert result.keys() == data.keys()
    np.testing.assert_array_equal(result["foo"], data["foo"][1])
    np.testing.assert_array_equal(result["hi"], data["hi"]["data"][0:2])

    with pytest.raises(IndexError):
        get_dataset_scenario(data, 2)


def test_get_data_set_batch_size():
    line = initialize_array(DatasetType.update, ComponentType.line, (3, 2))
    line["id"] = [[5, 6], [6, 7], [7, 5]]
    line["from_status"] = [[1, 1], [1, 1], [1, 1]]

    asym_load = initialize_array(DatasetType.update, ComponentType.asym_load, (3, 2))
    asym_load["id"] = [[9, 10], [9, 10], [9, 10]]

    batch_data = {ComponentType.line: line, ComponentType.asym_load: asym_load}

    n_batch_size = 3

    assert get_dataset_batch_size(batch_data) == n_batch_size


def test_get_dataset_batch_size_sparse():
    data = {
        ComponentType.node: {
            "data": np.zeros(shape=3, dtype=power_grid_meta_data[DatasetType.input][ComponentType.node]),
            "indptr": np.array([0, 2, 3, 3]),
        },
        ComponentType.sym_load: {
            "data": np.zeros(shape=2, dtype=power_grid_meta_data[DatasetType.input][ComponentType.sym_load]),
            "indptr": np.array([0, 0, 1, 2]),
        },
        ComponentType.asym_load: {
            "data": np.zeros(shape=4, dtype=power_grid_meta_data[DatasetType.input][ComponentType.asym_load]),
            "indptr": np.array([0, 2, 3, 4]),
        },
    }

    n_batch_size = 3

    assert get_dataset_batch_size(data) == n_batch_size


def test_get_dataset_batch_size_mixed():
    line = initialize_array(DatasetType.update, ComponentType.line, (3, 2))
    line["id"] = [[5, 6], [6, 7], [7, 5]]
    line["from_status"] = [[1, 1], [1, 1], [1, 1]]

    asym_load = initialize_array(DatasetType.update, ComponentType.asym_load, (2, 2))
    asym_load["id"] = [[9, 10], [9, 10]]

    data_dense = {ComponentType.line: line, ComponentType.asym_load: asym_load}
    data_sparse = {
        ComponentType.node: {
            "data": np.zeros(shape=3, dtype=power_grid_meta_data[DatasetType.input][ComponentType.node]),
            "indptr": np.array([0, 2, 3, 3, 5]),
        },
        ComponentType.sym_load: {
            "data": np.zeros(shape=2, dtype=power_grid_meta_data[DatasetType.input][ComponentType.sym_load]),
            "indptr": np.array([0, 0, 1, 2]),
        },
        ComponentType.asym_load: {
            "data": np.zeros(shape=4, dtype=power_grid_meta_data[DatasetType.input][ComponentType.asym_load]),
            "indptr": np.array([0, 2, 3]),
        },
    }
    with pytest.raises(ValueError):
        get_dataset_batch_size(data_dense)
    with pytest.raises(ValueError):
        get_dataset_batch_size(data_sparse)


def test_get_component_batch_size():
    asym_load = initialize_array(DatasetType.update, ComponentType.asym_load, (3, 2))
    asym_load["id"] = [[9, 10], [9, 10], [9, 10]]

    sym_load = {
        "data": np.zeros(shape=2, dtype=power_grid_meta_data[DatasetType.input][ComponentType.sym_load]),
        "indptr": np.array([0, 0, 1, 2]),
    }

    asym_load_batch_size = 3
    sym_load_batch_size = 3
    assert get_component_batch_size(asym_load) == asym_load_batch_size
    assert get_component_batch_size(sym_load) == sym_load_batch_size


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.json_deserialize")
def test_json_deserialize_from_file(deserialize_mock: MagicMock, open_mock: MagicMock):
    handle = open_mock()
    handle.read.return_value = '{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    deserialize_mock.return_value = {"foo": [{"val": 123}]}
    assert json_deserialize_from_file(file_path=Path("output.json")) == deserialize_mock.return_value
    handle.read.assert_called_once()
    deserialize_mock.assert_called_once_with(handle.read.return_value, data_filter=None)


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.json_serialize")
def test_json_serialize(serialize_mock: MagicMock, open_mock: MagicMock):
    serialize_mock.return_value = '{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    data: Dataset = {}
    json_serialize_to_file(file_path=Path("output.json"), data=data, use_compact_list=False, indent=2)
    serialize_mock.assert_called_once_with(data=data, dataset_type=None, use_compact_list=False, indent=2)
    handle = open_mock()
    handle.write.assert_called_once_with(serialize_mock.return_value)


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.msgpack_deserialize")
def test_msgpack_deserialize_from_file(deserialize_mock: MagicMock, open_mock: MagicMock):
    handle = open_mock()
    handle.read.return_value = b'{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    deserialize_mock.return_value = {"foo": [{"val": 123}]}
    assert msgpack_deserialize_from_file(file_path=Path("output.msgpack")) == deserialize_mock.return_value
    handle.read.assert_called_once()
    deserialize_mock.assert_called_once_with(handle.read.return_value, data_filter=None)


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.msgpack_serialize")
def test_msgpack_serialize(serialize_mock: MagicMock, open_mock: MagicMock):
    serialize_mock.return_value = b'{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    data: Dataset = {}
    msgpack_serialize_to_file(file_path=Path("output.msgpack"), data=data, use_compact_list=False)
    serialize_mock.assert_called_once_with(data=data, dataset_type=None, use_compact_list=False)
    handle = open_mock()
    handle.write.assert_called_once_with(serialize_mock.return_value)


def test_self_test():
    self_test()


@pytest.mark.parametrize(
    ("output_dataset_type", "output_file_name", "update_data"),
    (
        (DatasetType.sym_output, "sym_output_batch.json", {"version": "1.0", "data": "update_data"}),
        (DatasetType.sym_output, "sym_output.json", None),
        (DatasetType.asym_output, "asym_output_batch.json", {"version": "1.0", "data": "update_data"}),
        (DatasetType.asym_output, "asym_output.json", None),
        (DatasetType.sc_output, "sc_output_batch.json", {"version": "1.0", "data": "update_data"}),
        (DatasetType.sc_output, "sc_output.json", None),
    ),
)
@patch.object(Path, "write_text", autospec=True)
@patch("power_grid_model.utils.json_serialize_to_file")
def test__make_test_case(
    serialize_to_file_mock: MagicMock, write_text_mock: MagicMock, output_dataset_type, output_file_name, update_data
):
    input_data: Dataset = {"version": "1.0", "data": "input_data"}
    output_data: Dataset = {"version": "1.0", "data": "output_data"}
    output_path = Path("test_path")
    params = {"param1": "value1", "param2": "value2"}
    write_update_call_count = 5
    serialize_update_call_count = 3
    write_call_count = 4
    serialize_call_count = 2

    _make_test_case(
        output_path=output_path,
        input_data=input_data,
        output_data=output_data,
        params=params,
        output_dataset_type=output_dataset_type,
        update_data=update_data,
    )

    serialize_to_file_mock.assert_any_call(
        file_path=output_path / "input.json", data=input_data, dataset_type=DatasetType.input
    )
    serialize_to_file_mock.assert_any_call(
        file_path=output_path / output_file_name, data=output_data, dataset_type=output_dataset_type
    )
    write_text_mock.assert_any_call(output_path / "params.json", data=json.dumps(params, indent=2), encoding="utf-8")
    for file_name in ["input.json.license", f"{output_file_name}.license", "params.json.license"]:
        write_text_mock.assert_any_call(output_path / file_name, data=LICENSE_TEXT, encoding="utf-8")
    if update_data is not None:
        write_text_mock.assert_any_call(output_path / "update_batch.json.license", data=LICENSE_TEXT, encoding="utf-8")
        serialize_to_file_mock.assert_any_call(
            file_path=output_path / "update_batch.json", data=update_data, dataset_type=DatasetType.update
        )
        assert write_text_mock.call_count == write_update_call_count
        assert serialize_to_file_mock.call_count == serialize_update_call_count
    else:
        assert write_text_mock.call_count == write_call_count
        assert serialize_to_file_mock.call_count == serialize_call_count
