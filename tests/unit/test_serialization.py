# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import itertools
import json
from typing import Any, Dict, Optional

import msgpack
import numpy as np
import pytest

from power_grid_model import JsonDeserializer, JsonSerializer, MsgpackDeserializer, MsgpackSerializer
from power_grid_model.core.error_handling import assert_no_error


def sort_dict(data: Dict):
    return dict(sorted(data.items()))


def to_json(data, raw_buffer: bool = False, indent: Optional[int] = None):
    indent = None if indent is None or indent < 0 else indent
    separators = (",", ":") if indent is None else None
    result = json.dumps(sort_dict(data), indent=indent, separators=separators)

    if raw_buffer:
        return result.encode("utf-8")

    return result


def to_msgpack(data):
    return msgpack.packb(sort_dict(data))


def from_msgpack(data):
    return sort_dict(msgpack.unpackb(data))


def empty_dataset(dataset_type: str = "input"):
    return {"version": "1.0", "type": dataset_type, "is_batch": False, "data": {}, "attributes": {}}


def simple_input_dataset():
    return {
        "attributes": {},
        "data": {"node": [{"id": 5}], "source": [{"id": 6}, {"id": 7}]},
        "is_batch": False,
        "type": "input",
        "version": "1.0",
    }


def full_input_dataset():
    result = empty_dataset("input")
    result["attributes"] = {
        "node": ["id", "u_rated"],
        "sym_load": ["id", "node", "status", "type", "p_specified", "q_specified"],
        "source": ["id", "node", "status", "u_ref", "sk"],
    }
    result["data"] = {
        "node": [[1, 10.5e3], [2, 10.5e3], [3, 10.5e3]],
        "line": [
            {
                "id": 4,
                "from_node": 1,
                "to_node": 2,
                "from_status": 1,
                "to_status": 1,
                "r1": 0.11,
                "x1": 0.12,
                "c1": 4e-05,
                "tan1": 0.1,
                "i_n": 500.0,
            },
            {
                "id": 5,
                "from_node": 2,
                "to_node": 3,
                "from_status": 1,
                "to_status": 1,
                "r1": 0.15,
                "x1": 0.16,
                "c1": 5e-05,
                "tan1": 0.12,
                "i_n": 550.0,
            },
        ],
        "source": [
            [15, 1, 1, 1.03, 1e20],
            [16, 1, 1, 1.04, None],
            {"id": 17, "node": 1, "status": 1, "u_ref": 1.03, "sk": 1e10, "rx_ratio": 0.2},
        ],
        "sym_load": [[7, 2, 1, 0, 1.01e6, 0.21e6], [8, 3, 1, 0, 1.02e6, 0.22e6]],
    }
    return result


def single_update_dataset():
    result = empty_dataset("update")
    result["attributes"] = {
        "sym_load": ["status", "p_specified", "q_specified"],
        "source": ["status"],
    }
    result["data"] = {
        "line": [{}, {"from_status": 1, "to_status": 1}],
        "source": [[1], [0], {"status": 1, "u_ref": 1.03}],
        "sym_load": [[9, 1.01e6, 0.21e6], [10, 1.02e6, 0.22e6]],
    }
    return result


def homogeneous_batch_update_dataset():
    return {
        "version": "1.0",
        "type": "update",
        "is_batch": True,
        "attributes": {"sym_load": ["id", "p_specified", "q_specified"], "asym_load": ["id", "p_specified"]},
        "data": [
            {"sym_load": [[7, 20.0, 50.0]], "asym_load": [[9, [100.0, None, 200.0]]]},
            {"sym_load": [[7, None, None]], "asym_load": [[9, None]]},
            {
                "sym_load": [{"id": 7, "status": 0}],
                "asym_load": [{"id": 9, "q_specified": [70.0, 80.0, 90.0]}],
            },
        ],
    }


def sparse_batch_update_dataset():
    return {
        "version": "1.0",
        "type": "update",
        "is_batch": True,
        "attributes": {"sym_load": ["id", "p_specified", "q_specified"], "asym_load": ["id", "p_specified"]},
        "data": [
            {"sym_load": [[7, 20.0, 50.0]], "asym_load": [[9, [100.0, None, 200.0]]]},
            {"asym_load": [[9, None]]},
            {
                "sym_load": [[7, None, 10.0], {"id": 8, "status": 0}],
                "asym_load": [{"id": 9, "q_specified": [70.0, 80.0, 90.0]}],
            },
        ],
    }


def single_sym_output_dataset():
    result = empty_dataset("sym_output")
    result["data"] = {
        "node": [{"id": 1, "energized": 1, "u_pu": 1.01, "u_angle": 0.21, "u": 1.02e3, "p": 1.01e6, "q": 4.1e5}]
    }
    return result


def batch_sym_output_dataset():
    result = empty_dataset("sym_output")
    result["is_batch"] = True
    result["data"] = [
        {"node": [{"id": 1, "energized": 1, "u_pu": 1.01, "u_angle": 0.21, "u": 1.02e3, "p": 1.01e6, "q": 4.1e5}]},
        {"node": [{"id": 1, "energized": 0, "u_pu": 0.0, "u_angle": 0.0, "u": 0.0, "p": 0.0, "q": 0.0}]},
    ]
    return result


def single_asym_output_dataset():
    result = empty_dataset("asym_output")
    result["data"] = {
        "node": [
            {
                "id": 1,
                "energized": 1,
                "u_pu": [1.01, 1.06, 0.99],
                "u_angle": [0.21, 2.01, 4.1],
                "u": [1.02e3, 1.07e3, 9.9e2],
                "p": [1.01e6, 1.03e6, 9.8e5],
                "q": [4.1e5, 4.2e5, 4.0e5],
            }
        ]
    }
    return result


def single_sc_output_dataset():
    result = empty_dataset("sc_output")
    result["attributes"] = {"fault": ["id", "i_f"]}
    result["data"] = {
        "node": [
            {
                "id": 1,
                "energized": 1,
                "u_pu": [1.01, 1.06, 0.99],
                "u_angle": [0.21, 2.01, 4.1],
                "u": [1.02e3, 1.07e3, 9.9e2],
            }
        ],
        "fault": [{"id": 1, "i_f": [3.0e3, 2.0e3, 3.4e3]}],
    }
    return result


@pytest.fixture(
    params=[
        empty_dataset,
        simple_input_dataset,
        full_input_dataset,
        single_update_dataset,
        homogeneous_batch_update_dataset,
        sparse_batch_update_dataset,
        single_sym_output_dataset,
        batch_sym_output_dataset,
        single_asym_output_dataset,
        single_sc_output_dataset,
    ]
)
def serialized_data(request):
    if request.param is sparse_batch_update_dataset:
        request.applymarker(pytest.mark.xfail)
    return request.param()


def assert_not_a_value(value: np.ndarray):
    """Assert value contains only NaN-like values.

    Depending on type, maps to np.nan, -127, na_IntID, ...
    """
    if value.dtype.names:
        for key in value.dtype.names:
            assert_not_a_value(value[key])
    elif np.issubdtype(value.dtype, np.integer):
        na_int = np.finfo(value.dtype).min
        assert np.all(value == na_int)
    else:
        assert np.all(np.isnan(value))


def assert_almost_equal(value: np.ndarray, reference: Any):
    """Recursively walk through the values and references, assert almost equal.

    Correctly handle:
      * Both the sparse and dense data representations
      * None in the reference type maps to not a value.
    """
    if isinstance(reference, list):
        assert len(value) == len(reference)
        for v, r in zip(value, reference):
            assert_almost_equal(v, r)
    elif isinstance(reference, dict):
        assert len(value) == len(reference)
        for attribute in value.dtype.names:
            if attribute in reference:
                assert_almost_equal(value[attribute], reference[attribute])
            else:
                assert_not_a_value(reference[attribute])
    elif reference is None:
        assert_not_a_value(value)
    else:
        np.testing.assert_almost_equal(value, reference)


def assert_scenario_correct(deserialized_dataset: Dict[str, np.ndarray], serialized_dataset: Dict[str, Any]):
    assert deserialized_dataset.keys() == serialized_dataset["data"].keys()
    for component, component_result in deserialized_dataset.items():
        assert isinstance(component_result, np.ndarray)

        component_input = serialized_dataset["data"][component]
        assert len(component_result) == len(component_input)
        for result_entry, input_entry in zip(component_result, component_input):
            if isinstance(input_entry, dict):
                for attr, values in input_entry.items():
                    assert attr in result_entry.dtype.names
                    assert_almost_equal(result_entry[attr], values)
            else:
                assert isinstance(component_input, list)
                assert component in serialized_dataset["attributes"]
                for attr_idx, attr in enumerate(serialized_dataset["attributes"][component]):
                    assert attr in deserialized_dataset[component].dtype.names
                    assert_almost_equal(result_entry[attr], input_entry[attr_idx])


def assert_serialization_correct(deserialized_dataset: Dict[str, np.ndarray], serialized_dataset: Dict[str, Any]):
    """Assert the dataset correctly reprensents the input data."""
    if serialized_dataset["is_batch"]:
        assert isinstance(serialized_dataset["data"], list)

        for component_values in deserialized_dataset.values():
            assert len(component_values.shape) == 2
            assert len(component_values) == len(serialized_dataset["data"])

        for scenario_idx, scenario in enumerate(serialized_dataset["data"]):
            serialized_scenario = {k: v for k, v in serialized_dataset.items() if k not in ("data", "is_batch")}
            serialized_scenario["is_batch"] = False
            serialized_scenario["data"] = scenario

            assert_scenario_correct(
                {
                    component: component_values[scenario_idx]
                    for component, component_values in deserialized_dataset.items()
                },
                serialized_scenario,
            )
    else:
        assert_scenario_correct(deserialized_dataset, serialized_dataset)


@pytest.mark.parametrize("raw_buffer", (True, False))
def test_json_deserializer_create_destroy(serialized_data, raw_buffer: bool):
    data = to_json(serialized_data, raw_buffer=raw_buffer)

    deserializer = JsonDeserializer(data)
    assert_no_error()
    assert deserializer
    del deserializer


@pytest.mark.parametrize("raw_buffer", (True, False))
def test_json_deserializer_data(serialized_data, raw_buffer: bool):
    data = to_json(serialized_data, raw_buffer=raw_buffer)

    deserializer = JsonDeserializer(data)
    assert_no_error()

    result_type, result = deserializer.load()
    assert result_type == serialized_data["type"]
    assert isinstance(result, dict)
    assert_serialization_correct(result, serialized_data)


def test_msgpack_deserializer_create_destroy(serialized_data):
    data = to_msgpack(serialized_data)
    deserializer = MsgpackDeserializer(data)
    assert_no_error()
    assert deserializer
    del deserializer


def test_msgpack_deserializer_data(serialized_data):
    data = to_msgpack(serialized_data)

    deserializer = MsgpackDeserializer(data)
    assert_no_error()

    result_type, result = deserializer.load()
    assert result_type == serialized_data["type"]
    assert isinstance(result, dict)
    assert_serialization_correct(result, serialized_data)


@pytest.mark.parametrize("dataset_type", ("input", "update", "sym_output", "asym_output", "sc_output"))
@pytest.mark.parametrize("raw_buffer", (True, False))
def test_json_serializer_empty_dataset(dataset_type, raw_buffer: bool):
    serializer = JsonSerializer(dataset_type, {})

    if raw_buffer:
        reference = to_json(empty_dataset(dataset_type), raw_buffer=raw_buffer)
        for use_compact_list in (True, False):
            assert serializer.dump_bytes(use_compact_list=use_compact_list) == reference
    else:
        for use_compact_list, indent in itertools.product((True, False), (-1, 0, 2, 4)):
            reference = to_json(empty_dataset(dataset_type), raw_buffer=raw_buffer, indent=indent)
            assert isinstance(reference, str)

            result: Any = serializer.dump(use_compact_list=use_compact_list, indent=indent)
            assert isinstance(result, str)
            assert result == reference

            result = serializer.dump_str(use_compact_list=use_compact_list, indent=indent)
            assert isinstance(result, str)
            assert result == reference


@pytest.mark.parametrize("dataset_type", ("input", "update", "sym_output", "asym_output", "sc_output"))
def test_msgpack_serializer_empty_dataset(dataset_type):
    serializer = MsgpackSerializer(dataset_type, {})

    reference = sort_dict(empty_dataset(dataset_type))
    for use_compact_list in (True, False):
        assert from_msgpack(serializer.dump_bytes(use_compact_list=use_compact_list)) == reference


@pytest.mark.parametrize(
    ("deserializer_type", "serializer_type", "pack"),
    (
        (JsonDeserializer, JsonSerializer, to_json),
        (MsgpackDeserializer, MsgpackSerializer, to_msgpack),
    ),
)
def test_serializer_deserializer_double_round_trip(deserializer_type, serializer_type, serialized_data, pack):
    """
    Repeated deserialization and serialization must result in the same deserialized data and serialization string.
    """
    test_data = pack(serialized_data)

    deserializer_a = deserializer_type(test_data)
    dataset_type_a, deserialized_result_a = deserializer_a.load()
    serializer_a = serializer_type(dataset_type_a, deserialized_result_a)
    serialized_result_a = serializer_a.dump()

    deserializer_b = deserializer_type(serialized_result_a)
    dataset_type_b, deserialized_result_b = deserializer_b.load()
    serializer_b = serializer_type(dataset_type_b, deserialized_result_b)
    serialized_result_b = serializer_b.dump()

    assert serialized_result_a == serialized_result_b

    assert dataset_type_a == serialized_data["type"]
    assert dataset_type_b == serialized_data["type"]
    assert list(deserialized_result_b) == list(deserialized_result_a)

    for component_result_a, component_result_b in zip(deserialized_result_a.values(), deserialized_result_b.values()):
        assert component_result_a.dtype == component_result_b.dtype

        for field in component_result_a.dtype.names:
            field_result_a = component_result_a[field]
            field_result_b = component_result_b[field]

            nan_a = np.isnan(field_result_a)
            nan_b = np.isnan(field_result_b)

            np.testing.assert_array_equal(nan_a, nan_b)
            np.testing.assert_array_equal(field_result_a[~nan_a], field_result_b[~nan_b])
