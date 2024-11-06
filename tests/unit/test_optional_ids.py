# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import numpy as np
import pytest

from power_grid_model import (
    ComponentType,
    DatasetType,
    LoadGenType,
    PowerGridModel,
    initialize_array,
    power_grid_meta_data,
)
from power_grid_model._utils import compatibility_convert_row_columnar_dataset
from power_grid_model.enum import CalculationType, ComponentAttributeFilterOptions
from power_grid_model.validation import assert_valid_batch_data


@pytest.fixture
def input_node_row():
    node = initialize_array(DatasetType.input, ComponentType.node, 3)
    node["id"] = np.array([1, 2, 6])
    node["u_rated"] = [10.5e3, 10.5e3, 10.5e3]
    return node


@pytest.fixture
def input_node_col():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.input][ComponentType.node].dtype
    return {
        "id": np.array([1, 2, 6], dtype=source_attribute_dtypes["id"]),
        "u_rated": np.array([10.5e3, 10.5e3, 10.5e3], dtype=source_attribute_dtypes["u_rated"]),
    }


@pytest.fixture(params=["input_node_row", "input_node_col"])
def input_node(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def input_line_row():
    line = initialize_array(DatasetType.input, ComponentType.line, 3)
    line["id"] = [3, 5, 8]
    line["from_node"] = [1, 2, 1]
    line["to_node"] = [2, 6, 6]
    line["from_status"] = [1, 1, 1]
    line["to_status"] = [1, 1, 1]
    line["r1"] = [0.25, 0.25, 0.25]
    line["x1"] = [0.2, 0.2, 0.2]
    line["c1"] = [10e-6, 10e-6, 10e-6]
    line["tan1"] = [0.0, 0.0, 0.0]
    line["i_n"] = [1000, 1000, 1000]
    return line


@pytest.fixture
def input_line_col():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.input][ComponentType.line].dtype
    return {
        "id": np.array([3, 5, 8], dtype=source_attribute_dtypes["id"]),
        "from_node": np.array([1, 2, 1], dtype=source_attribute_dtypes["from_node"]),
        "to_node": np.array([2, 6, 6], dtype=source_attribute_dtypes["to_node"]),
        "from_status": np.array([1, 1, 1], dtype=source_attribute_dtypes["from_status"]),
        "to_status": np.array([1, 1, 1], dtype=source_attribute_dtypes["to_status"]),
        "r1": np.array([0.25, 0.25, 0.25], dtype=source_attribute_dtypes["r1"]),
        "x1": np.array([0.2, 0.2, 0.2], dtype=source_attribute_dtypes["x1"]),
        "c1": np.array([10e-6, 10e-6, 10e-6], dtype=source_attribute_dtypes["c1"]),
        "tan1": np.array([0.0, 0.0, 0.0], dtype=source_attribute_dtypes["tan1"]),
        "i_n": np.array([1000, 1000, 1000], dtype=source_attribute_dtypes["i_n"]),
    }


@pytest.fixture(params=["input_line_row", "input_line_col"])
def input_line(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def input_load_row():
    sym_load = initialize_array(DatasetType.input, ComponentType.sym_load, 2)
    sym_load["id"] = [4, 7]
    sym_load["node"] = [2, 6]
    sym_load["status"] = [1, 1]
    sym_load["type"] = [LoadGenType.const_power, LoadGenType.const_power]
    sym_load["p_specified"] = [20e6, 10e6]
    sym_load["q_specified"] = [5e6, 2e6]
    return sym_load


@pytest.fixture
def input_load_col():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.input][ComponentType.sym_load].dtype
    return {
        "id": np.array([4, 7], dtype=source_attribute_dtypes["id"]),
        "node": np.array([2, 6], dtype=source_attribute_dtypes["node"]),
        "status": np.array([1, 1], dtype=source_attribute_dtypes["status"]),
        "type": np.array([LoadGenType.const_power, LoadGenType.const_power], dtype=source_attribute_dtypes["type"]),
        "p_specified": np.array([20e6, 10e6], dtype=source_attribute_dtypes["p_specified"]),
        "q_specified": np.array([5e6, 2e6], dtype=source_attribute_dtypes["q_specified"]),
    }


@pytest.fixture(params=["input_load_row", "input_load_col"])
def input_load(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def input_source_row():
    source = initialize_array(DatasetType.input, ComponentType.source, 1)
    source["id"] = [10]
    source["node"] = [1]
    source["status"] = [1]
    source["u_ref"] = [1.0]
    return source


@pytest.fixture
def input_source_col():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.input][ComponentType.source].dtype
    return {
        "id": np.array([10], dtype=source_attribute_dtypes["id"]),
        "node": np.array([1], dtype=source_attribute_dtypes["node"]),
        "status": np.array([1], dtype=source_attribute_dtypes["status"]),
        "u_ref": np.array([1.0], dtype=source_attribute_dtypes["u_ref"]),
    }


@pytest.fixture(params=["input_source_row", "input_source_col"])
def input_source(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def input_dataset(input_node, input_line, input_load, input_source):
    return {
        ComponentType.node: input_node,
        ComponentType.line: input_line,
        ComponentType.sym_load: input_load,
        ComponentType.source: input_source,
    }


@pytest.fixture
def update_sym_load_row():
    sym_load = initialize_array(DatasetType.update, ComponentType.sym_load, 2)
    sym_load["id"] = [4, 7]
    sym_load["p_specified"] = [30e6, 15e6]
    return sym_load


@pytest.fixture
def update_sym_load_no_id_row():
    sym_load = initialize_array(DatasetType.update, ComponentType.sym_load, 2)
    sym_load["p_specified"] = [30e6, 15e6]
    return sym_load


@pytest.fixture
def update_sym_load_col():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.update][ComponentType.sym_load].dtype
    return {
        "id": np.array([4, 7], dtype=source_attribute_dtypes["id"]),
        "p_specified": np.array([30e6, 15e6], dtype=source_attribute_dtypes["p_specified"]),
    }


@pytest.fixture
def update_sym_load_no_id_col():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.update][ComponentType.sym_load].dtype
    return {
        "p_specified": np.array([30e6, 15e6], dtype=source_attribute_dtypes["p_specified"]),
    }


@pytest.fixture(
    params=["update_sym_load_row", "update_sym_load_no_id_row", "update_sym_load_col", "update_sym_load_no_id_col"]
)
def update_sym_load(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def update_line_row():
    line = initialize_array(DatasetType.update, ComponentType.line, 1)
    line["id"] = [3]
    line["from_status"] = [0]
    return line


@pytest.fixture
def update_line_no_id_row():
    line = initialize_array(DatasetType.update, ComponentType.line, 3)
    line["from_status"] = [0, 1, 1]
    return line


@pytest.fixture
def update_line_col():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.update][ComponentType.line].dtype
    return {
        "id": np.array([3], dtype=source_attribute_dtypes["id"]),
        "from_status": np.array([0], dtype=source_attribute_dtypes["from_status"]),
    }


@pytest.fixture
def update_line_no_id_col():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.update][ComponentType.line].dtype
    return {
        "from_status": np.array([0, 1, 1], dtype=source_attribute_dtypes["from_status"]),
    }


@pytest.fixture(params=["update_line_row", "update_line_no_id_row", "update_line_col", "update_line_no_id_col"])
def update_line(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def update_dataset(update_sym_load, update_line):
    return {
        ComponentType.sym_load: update_sym_load,
        ComponentType.line: update_line,
    }


def test_assert_valid_batch_data(input_dataset, update_dataset):
    assert_valid_batch_data(
        input_data=input_dataset, update_data=update_dataset, calculation_type=CalculationType.power_flow
    )


def test_permanent_update(input_dataset, update_dataset):
    model = PowerGridModel(input_dataset)
    model.update(update_data=update_dataset)
