# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import json
from typing import Any, Mapping

import msgpack
import numpy as np
import pytest

from power_grid_model import DatasetType
from power_grid_model._utils import get_dataset_type, is_columnar, is_sparse
from power_grid_model.data_types import BatchDataset, Dataset, SingleDataset
from power_grid_model.enum import ComponentAttributeFilterOptions
from power_grid_model.utils import json_deserialize, json_serialize, msgpack_deserialize, msgpack_serialize


def to_json(data, raw_buffer: bool = False, indent: int | None = None):
    indent = None if indent is None or indent < 0 else indent
    separators = (",", ":") if indent is None else None
    result = json.dumps(data, indent=indent, separators=separators)

    if raw_buffer:
        return result.encode("utf-8")

    return result


def to_msgpack(data):
    return msgpack.packb(data)


def from_msgpack(data):
    return msgpack.unpackb(data)


def is_non_compact_list(data_entry):
    return isinstance(data_entry, dict)


def is_serialized_data_type_deducible(serialized_input, data_filter) -> bool:
    """Find out if deserialization of serialized_input would contain at least one component with row based data"""
    serialized_input_data = serialized_input["data"]
    if not serialized_input_data or isinstance(data_filter, ComponentAttributeFilterOptions):
        return False
    if data_filter is None:
        return True

    if not serialized_input["is_batch"]:
        components = set(serialized_input_data.keys())
    else:
        components = set()
        for scenario in serialized_input_data:
            components.update(scenario.keys())

    return any(data_filter[comp] is None for comp in components.intersection(data_filter.keys()))


def is_columnar_filter(data_filter, component) -> bool:
    """A function to find if a data_filter will give out row or columnar format for a component"""
    if data_filter is None:
        return False
    if isinstance(data_filter, ComponentAttributeFilterOptions):
        return True
    return data_filter[component] is not None


def is_attribute_filtered_out(data_filter, component, attribute) -> bool:
    """Checks if attribute is being filtered out / excluded"""
    if data_filter is None or isinstance(data_filter, ComponentAttributeFilterOptions):
        return False

    if component not in data_filter:
        return True

    return not (
        data_filter[component] is None
        or isinstance(data_filter[component], ComponentAttributeFilterOptions)
        or attribute in data_filter[component]
    )


def is_component_filtered_out(data_filter, component) -> bool:
    """Checks if component is being filtered out / excluded"""
    if data_filter is None or isinstance(data_filter, ComponentAttributeFilterOptions):
        return False
    return component not in data_filter


def is_sparse_data_input(serialized_input_data, component):
    """Checks if serialized_input_data will be of sparse format after deserialization.
    If there are uneven ids for scenarios in the serialized input, that would result in sparse data"""
    if not serialized_input_data["is_batch"]:
        raise ValueError("Checking if a non batch data is sparse")

    all_scenarios_ids: list[set[int]] = list(set())
    for scenario in serialized_input_data["data"]:
        scenario_ids = set()
        if component not in scenario:
            continue
        for scenario_comp_name_comp_idx in scenario[component]:
            if is_non_compact_list(scenario_comp_name_comp_idx):
                scenario_ids.add(scenario_comp_name_comp_idx["id"])
            else:
                id_attr_idx = serialized_input_data["attributes"][component].index("id")
                scenario_ids.add(scenario_comp_name_comp_idx[id_attr_idx])
        all_scenarios_ids.append(scenario_ids)
    first_scenario_ids = next(iter(all_scenarios_ids))

    return any(first_scenario_ids != scenario_ids for scenario_ids in all_scenarios_ids)


def empty_dataset(dataset_type: DatasetType = DatasetType.input):
    return {
        "version": "1.0",
        "type": dataset_type,
        "is_batch": False,
        "attributes": {},
        "data": {},
    }


def simple_input_dataset():
    return {
        "attributes": {},
        "data": {"node": [{"id": 5}], "source": [{"id": 6}, {"id": 7}]},
        "is_batch": False,
        "type": "input",
        "version": "1.0",
    }


def simple_asym_input_dataset():
    return {
        "attributes": {},
        "data": {"asym_load": [{"id": 5, "p_specified": [10.0, 20.0, 30.0]}]},
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
            {
                "id": 17,
                "node": 1,
                "status": 1,
                "u_ref": 1.03,
                "sk": 1e10,
                "rx_ratio": 0.2,
            },
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


def uniform_batch_update_dataset():
    return {
        "version": "1.0",
        "type": "update",
        "is_batch": True,
        "attributes": {
            "sym_load": ["id", "p_specified", "q_specified"],
            "asym_load": ["id", "p_specified"],
        },
        "data": [
            {"sym_load": [[7, 20.0, 50.0]], "asym_load": [[9, [100.0, None, 200.0]]]},
            {"sym_load": [[7, None, None]], "asym_load": [[9, None]]},
            {
                "sym_load": [{"id": 7, "status": 0}],
                "asym_load": [{"id": 9, "q_specified": [70.0, 80.0, 90.0]}],
            },
        ],
    }


def inhomogeneous_batch_update_dataset():
    return {
        "version": "1.0",
        "type": "update",
        "is_batch": True,
        "attributes": {
            "sym_load": ["id", "p_specified", "q_specified"],
            "asym_load": ["id", "p_specified"],
        },
        "data": [
            {"sym_load": [[7, 20.0, 50.0]], "asym_load": [[9, [100.0, None, 200.0]]]},
            {"asym_load": [[9, None]]},
            {
                "sym_load": [[7, None, 10.0], {"id": 8, "status": 0}],
                "asym_load": [{"id": 9, "q_specified": [70.0, 80.0, 90.0]}],
            },
        ],
    }


def sparse_batch_update_dataset():
    return {
        "version": "1.0",
        "type": "update",
        "is_batch": True,
        "attributes": {},
        "data": [
            {"sym_load": [{"id": 7, "q_specified": 50.0}]},
            {"sym_load": [{"id": 8, "q_specified": 33.333333333333336}]},
            {
                "sym_load": [
                    {"id": 7, "q_specified": 10.0},
                    {"id": 8, "q_specified": 2.5},
                ]
            },
        ],
    }


def single_sym_output_dataset():
    result = empty_dataset("sym_output")
    result["data"] = {
        "node": [
            {
                "id": 1,
                "energized": 1,
                "u_pu": 1.01,
                "u_angle": 0.21,
                "u": 1.02e3,
                "p": 1.01e6,
                "q": 4.1e5,
            }
        ]
    }
    return result


def batch_sym_output_dataset():
    result = empty_dataset("sym_output")
    result["is_batch"] = True
    result["data"] = [
        {
            "node": [
                {
                    "id": 1,
                    "energized": 1,
                    "u_pu": 1.01,
                    "u_angle": 0.21,
                    "u": 1.02e3,
                    "p": 1.01e6,
                    "q": 4.1e5,
                }
            ]
        },
        {
            "node": [
                {
                    "id": 1,
                    "energized": 0,
                    "u_pu": 0.0,
                    "u_angle": 0.0,
                    "u": 0.0,
                    "p": 0.0,
                    "q": 0.0,
                }
            ]
        },
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
        uniform_batch_update_dataset,
        inhomogeneous_batch_update_dataset,
        sparse_batch_update_dataset,
        single_sym_output_dataset,
        batch_sym_output_dataset,
        single_asym_output_dataset,
        single_sc_output_dataset,
        simple_asym_input_dataset,
    ]
)
def serialized_data(request):
    return request.param()


@pytest.fixture(
    params=[
        pytest.param(None, id="All row filter"),
        pytest.param(ComponentAttributeFilterOptions.everything, id="All columnar filter"),
        pytest.param(ComponentAttributeFilterOptions.relevant, id="All relevant columnar filter"),
        pytest.param({"node": ["id"], "sym_load": ["id"]}, id="columnar filter"),
        pytest.param({"node": ["id"], "sym_load": None}, id="mixed columnar/row filter"),
        pytest.param({"node": ["id"], "shunt": None}, id="unused component filter"),
        pytest.param(
            {
                "node": ["id"],
                "line": ComponentAttributeFilterOptions.everything,
                "sym_load": None,
                "asym_load": ComponentAttributeFilterOptions.relevant,
            },
            id="mixed filter",
        ),
    ]
)
def data_filters(request):
    """Data filers for deserialization. Note: List / set is not supported for testing since it can be replicated using dict."""
    return request.param


def split_deserialized_dataset_into_individual_scenario(scenario_idx, deserialized_dataset):
    deserialized_scenario = {}
    for component, component_values in deserialized_dataset.items():
        if not is_sparse(component_values):
            if not is_columnar(component_values):
                deserialized_scenario[component] = component_values[scenario_idx]
            else:
                deserialized_scenario[component] = {}
                for attr, attr_values in component_values.items():
                    deserialized_scenario[component][attr] = attr_values[scenario_idx]
        else:
            scenario_slice = slice(
                component_values["indptr"][scenario_idx],
                component_values["indptr"][scenario_idx + 1],
            )
            if not is_columnar(component_values):
                sparse_component_data = component_values["data"][scenario_slice]
                component_in_scenario = len(sparse_component_data) > 0
            else:
                sparse_component_data = {
                    attr: attr_value[scenario_slice] for attr, attr_value in component_values["data"].items()
                }
                component_in_scenario = len(next(iter(sparse_component_data.values()))) > 0

            if component_in_scenario:
                deserialized_scenario[component] = sparse_component_data
    return deserialized_scenario


def assert_not_a_value(value: np.ndarray):
    """Assert value contains only NaN-like values.

    Depending on type, maps to np.nan, -127, na_IntID, etc.
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


def assert_single_dataset_entries(
    deserialized_dataset: SingleDataset,
    serialized_dataset: Mapping[str, Any],
    data_filter,
):
    """Check a SingleDataset's individual components for correctness"""
    for component, serialized_input in serialized_dataset["data"].items():
        if is_component_filtered_out(data_filter, component):
            assert component not in deserialized_dataset
            continue

        deserialized_output = deserialized_dataset[component]

        # Complete component data checks
        if is_columnar_filter(data_filter, component):
            assert isinstance(deserialized_output, dict)
            # Assert number of batches are equal for all attributes
            assert all(len(v) == len(serialized_input) for v in deserialized_output.values())
            # Assert number of attributes are not more than input for all batches
            # Discrepencies in equal / less than input captured in individual check
            # TODO Test for deserialize only relevant attributes
            # assert all(len(v) >= len(deserialized_output) for v in serialized_input)
        else:
            assert isinstance(deserialized_output, np.ndarray)
            assert len(deserialized_output) == len(serialized_input)

        assert_individual_data_entry(
            serialized_dataset,
            data_filter,
            component,
            serialized_input,
            deserialized_output,
        )


def assert_individual_data_entry(serialized_dataset, data_filter, component, serialized_input, deserialized_output):
    """Checks each data entry of the component for correctness"""
    for comp_idx, input_entry in enumerate(serialized_input):
        if is_non_compact_list(input_entry):
            for attr in input_entry:
                if is_columnar_filter(data_filter, component):
                    if is_attribute_filtered_out(data_filter, component, attr):
                        assert attr not in deserialized_output
                        continue
                    assert attr in deserialized_output.keys()
                    assert_almost_equal(
                        deserialized_output[attr][comp_idx],
                        serialized_input[comp_idx][attr],
                    )
                else:
                    assert attr in deserialized_output[comp_idx].dtype.names
                    assert_almost_equal(
                        deserialized_output[comp_idx][attr],
                        serialized_input[comp_idx][attr],
                    )
        else:
            assert component in serialized_dataset["attributes"]
            for attr_idx, attr in enumerate(serialized_dataset["attributes"][component]):
                if is_columnar_filter(data_filter, component):
                    if is_attribute_filtered_out(data_filter, component, attr):
                        assert attr not in deserialized_output
                        continue
                    assert attr in deserialized_output.keys()
                    assert_almost_equal(
                        deserialized_output[attr][comp_idx],
                        serialized_input[comp_idx][attr_idx],
                    )
                else:
                    assert attr in deserialized_output[comp_idx].dtype.names
                    assert_almost_equal(
                        deserialized_output[comp_idx][attr],
                        serialized_input[comp_idx][attr_idx],
                    )


def assert_single_dataset_structure(deserialized_dataset, data_filter):
    for component, component_result in deserialized_dataset.items():
        if is_columnar_filter(data_filter, component):
            for attr_name, arr in component_result.items():
                assert isinstance(arr, np.ndarray)
                assert isinstance(attr_name, str)
                assert len(arr.shape) in [1, 2]
        else:
            assert isinstance(component_result, np.ndarray)
            assert component_result.ndim == 1


def assert_batch_dataset_structure(
    deserialized_dataset: BatchDataset,
    serialized_dataset: Mapping[str, Any],
    data_filter,
):
    """Checks if the structure of the batch dataset is correct.
    Then splits into individual scenario's dataset and checks if all of them are correct."""

    # Check structure of the whole BatchDataset
    assert isinstance(serialized_dataset["data"], list)
    for component, component_values in deserialized_dataset.items():
        if is_sparse_data_input(serialized_dataset, component):
            component_indptr = component_values["indptr"]
            component_data = component_values["data"]
            assert isinstance(component_indptr, np.ndarray)
            assert len(component_indptr) == len(serialized_dataset["data"]) + 1
            if is_columnar_filter(data_filter, component):
                assert isinstance(component_data, dict)
                for attr, attr_value in component_data.items():
                    assert isinstance(attr, str)
                    assert isinstance(attr_value, np.ndarray)
                    assert attr_value.ndim == 1
            else:
                assert isinstance(component_data, np.ndarray)
                assert component_data.ndim == 1
                assert len(component_data) == component_indptr[-1]
        else:
            if is_columnar_filter(data_filter, component):
                for attr, attr_value in component_values.items():
                    assert isinstance(attr, str)
                    assert isinstance(attr_value, np.ndarray)
                    assert len(attr_value.shape) in [2, 3]
                    assert len(attr_value) == len(serialized_dataset["data"])
            else:
                assert isinstance(component_values, np.ndarray)
                assert len(component_values.shape) == 2
                assert len(component_values) == len(serialized_dataset["data"])


def assert_serialization_correct(deserialized_dataset: Dataset, serialized_dataset: Mapping[str, Any], data_filter):
    """Assert the dataset correctly reprensents the input data."""
    if serialized_dataset["is_batch"]:
        assert_batch_dataset_structure(deserialized_dataset, serialized_dataset, data_filter=data_filter)

        # Split into individual SingleDataset and check if they all are correct
        for scenario_idx, scenario in enumerate(serialized_dataset["data"]):
            serialized_scenario = {k: v for k, v in serialized_dataset.items() if k not in ("data", "is_batch")}
            serialized_scenario["is_batch"] = False
            serialized_scenario["data"] = scenario

            deserialized_scenario = split_deserialized_dataset_into_individual_scenario(
                scenario_idx, deserialized_dataset
            )
            assert_single_dataset_entries(deserialized_scenario, serialized_scenario, data_filter=data_filter)
    else:
        assert_single_dataset_structure(deserialized_dataset, data_filter)

        assert_single_dataset_entries(
            deserialized_dataset,  # type: ignore[arg-type]
            serialized_dataset,
            data_filter=data_filter,
        )


def _check_only_relevant_attributes_present(component_values) -> bool:
    for array in component_values.values():
        if not isinstance(array, np.ndarray):
            continue
        if (array.dtype == np.float64 and np.isnan(array).all()) or (
            array.dtype in (np.int32, np.int8) and np.all(array == np.iinfo(array.dtype).min)
        ):
            return False
    return True


def assert_deserialization_filtering_correct(deserialized_dataset: Dataset, data_filter) -> bool:
    if data_filter is ComponentAttributeFilterOptions.everything:
        return True
    if data_filter is ComponentAttributeFilterOptions.relevant:
        for component_values in deserialized_dataset.values():
            if not _check_only_relevant_attributes_present(component_values):
                return False
    return True


@pytest.mark.parametrize("raw_buffer", (True, False))
def test_json_deserialize_data(serialized_data, data_filters, raw_buffer: bool):
    data = to_json(serialized_data, raw_buffer=raw_buffer)
    result = json_deserialize(data, data_filter=data_filters)

    assert isinstance(result, dict)
    assert_serialization_correct(result, serialized_data, data_filters)

    if is_serialized_data_type_deducible(serialized_data, data_filter=data_filters):
        result_type = get_dataset_type(result)
        assert result_type == serialized_data["type"]


def test_msgpack_deserialize_data(serialized_data, data_filters):
    data = to_msgpack(serialized_data)
    result = msgpack_deserialize(data, data_filters)

    assert isinstance(result, dict)
    assert_serialization_correct(result, serialized_data, data_filter=data_filters)

    if is_serialized_data_type_deducible(serialized_data, data_filter=data_filters):
        result_type = get_dataset_type(result)
        assert result_type == serialized_data["type"]


@pytest.mark.parametrize(
    "dataset_type",
    (
        DatasetType.input,
        DatasetType.update,
        DatasetType.sym_output,
        DatasetType.asym_output,
        DatasetType.sc_output,
    ),
)
@pytest.mark.parametrize(
    "use_compact_list",
    (pytest.param(True, id="compact_list"), pytest.param(False, id="non-compact_list")),
)
def test_json_serialize_empty_dataset(dataset_type, use_compact_list: bool):
    for indent in (-1, 0, 2, 4):
        reference = to_json(empty_dataset(dataset_type), indent=indent)
        assert isinstance(reference, str)

        result = json_serialize({}, dataset_type, use_compact_list=use_compact_list, indent=indent)
        assert isinstance(result, str)
        assert result == reference

        with pytest.raises(ValueError):
            json_serialize({}, use_compact_list=use_compact_list, indent=indent)


@pytest.mark.parametrize(
    "dataset_type",
    (
        DatasetType.input,
        DatasetType.update,
        DatasetType.sym_output,
        DatasetType.asym_output,
        DatasetType.sc_output,
    ),
)
@pytest.mark.parametrize(
    "use_compact_list",
    (pytest.param(True, id="compact_list"), pytest.param(False, id="non-compact_list")),
)
def test_msgpack_serialize_empty_dataset(dataset_type, use_compact_list):
    reference = empty_dataset(dataset_type)
    assert from_msgpack(msgpack_serialize({}, dataset_type, use_compact_list=use_compact_list)) == reference

    with pytest.raises(ValueError):
        json_serialize({}, use_compact_list=use_compact_list)


@pytest.mark.parametrize(
    ("deserialize", "serialize", "pack"),
    (
        pytest.param(json_deserialize, json_serialize, to_json, id="json"),
        pytest.param(msgpack_deserialize, msgpack_serialize, to_msgpack, id="msgpack"),
    ),
)
def test_serialize_deserialize_type_deduction(deserialize, serialize, serialized_data, data_filters, pack):
    deserialized_data = deserialize(pack(serialized_data), data_filter=data_filters)
    full_result = serialize(deserialized_data, serialized_data["type"])

    if is_serialized_data_type_deducible(serialized_data, data_filter=data_filters):
        assert serialize(deserialized_data) == full_result
    else:
        with pytest.raises(ValueError):
            serialize(deserialized_data)


@pytest.mark.parametrize(
    ("deserialize", "serialize", "pack"),
    (
        pytest.param(json_deserialize, json_serialize, to_json, id="json"),
        pytest.param(msgpack_deserialize, msgpack_serialize, to_msgpack, id="msgpack"),
    ),
)
def test_serialize_deserialize_double_round_trip(deserialize, serialize, serialized_data, data_filters, pack):
    """
    Repeated deserialization and serialization must result in the same deserialized data and serialization string.
    """
    test_data = pack(serialized_data)

    deserialized_result_a = deserialize(test_data, data_filters)
    serialized_result_a = serialize(deserialized_result_a, dataset_type=serialized_data["type"])

    deserialized_result_b = deserialize(serialized_result_a, data_filters)
    serialized_result_b = serialize(deserialized_result_b, dataset_type=serialized_data["type"])

    assert serialized_result_a == serialized_result_b
    assert list(deserialized_result_b) == list(deserialized_result_a)
    assert assert_deserialization_filtering_correct(deserialized_result_a, data_filters)
    assert assert_deserialization_filtering_correct(deserialized_result_b, data_filters)

    for (component_a, component_result_a), component_result_b in zip(
        deserialized_result_a.items(), deserialized_result_b.values()
    ):
        is_non_uniform = is_sparse(component_result_a)
        assert is_non_uniform == is_sparse(component_result_b)

        if not is_non_uniform:
            component_data_a = component_result_a
            component_data_b = component_result_b
        else:
            np.testing.assert_array_equal(component_result_a["indptr"], component_result_b["indptr"])
            component_data_a = component_result_a["data"]
            component_data_b = component_result_b["data"]

        if is_columnar_filter(data_filters, component_a):
            assert component_data_a.keys() == component_data_b.keys()
            assert all(v_a.dtype == v_b.dtype for v_a, v_b in zip(component_data_a.values(), component_data_b.values()))
            fields_or_keys = component_data_a.keys()
        else:
            assert component_data_a.dtype == component_data_b.dtype
            fields_or_keys = component_data_a.dtype.names

        for field in fields_or_keys:
            field_result_a = component_data_a[field]
            field_result_b = component_data_b[field]

            nan_a = np.isnan(field_result_a)
            nan_b = np.isnan(field_result_b)

            np.testing.assert_array_equal(nan_a, nan_b)
            np.testing.assert_allclose(field_result_a[~nan_a], field_result_b[~nan_b], rtol=1e-15)
