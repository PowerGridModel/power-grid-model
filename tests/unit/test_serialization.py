# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import json
from typing import Any, Dict

import msgpack
import numpy as np
import pytest

from power_grid_model import JsonDeserializer, MsgpackDeserializer
from power_grid_model.core.error_handling import assert_no_error


def to_json(data, raw_buffer: bool):
    data = json.dumps(data)

    if raw_buffer:
        data = bytes(data, "utf-8")

    return data


def to_msgpack(data):
    return msgpack.packb(data)


def empty_dataset(dataset_type: str = "input"):
    return {"version": "0.0", "type": dataset_type, "is_batch": False, "data": {}, "attributes": {}}


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
                assert_almost_equal(v, reference[attribute])
            else:
                assert_not_a_value(reference[attribute])
    elif reference is None:
        assert_not_a_value(value)
    else:
        np.testing.assert_almost_equal(value, reference)


def assert_scenario_correct(result: Dict[str, np.ndarray], serialized_data: Dict[str, Any]):
    assert result.keys() == serialized_data["data"].keys()
    for component, component_result in result.items():
        assert isinstance(component_result, np.ndarray)

        component_input = serialized_data["data"][component]
        assert len(component_result) == len(component_input)
        for result_entry, input_entry in zip(component_result, component_input):
            if isinstance(input_entry, dict):
                for attr, values in input_entry.items():
                    assert attr in result_entry.dtype.names
                    assert_almost_equal(result_entry[attr], values)
            else:
                assert isinstance(component_input, list)
                assert component in serialized_data["attributes"]
                for attr_idx, attr in enumerate(serialized_data["attributes"][component]):
                    assert attr in result[component].dtype.names
                    assert_almost_equal(result_entry[attr], input_entry[attr_idx])


def assert_serialization_correct(result: Dict[str, np.ndarray], serialized_data: Dict[str, Any]):
    """Assert the dataset correctly reprensents the input data."""
    if serialized_data["is_batch"]:
        assert isinstance(serialized_data["data"], list)

        for component_values in result.values():
            assert len(component_values.shape) == 2
            assert len(component_values) == len(serialized_data["data"])

        for scenario_idx, scenario in enumerate(serialized_data["data"]):
            serialized_scenario = {k: v for k, v in serialized_data.items() if k not in ("data", "is_batch")}
            serialized_scenario["is_batch"] = False
            serialized_scenario["data"] = scenario

            assert_scenario_correct(
                {component: component_values[scenario_idx] for component, component_values in result.items()},
                serialized_scenario,
            )
    else:
        assert_scenario_correct(result, serialized_data)


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

    result = deserializer.load()
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

    result = deserializer.load()
    assert isinstance(result, dict)
    assert_serialization_correct(result, serialized_data)
