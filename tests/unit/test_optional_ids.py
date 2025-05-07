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
from power_grid_model._core.utils import compatibility_convert_row_columnar_dataset
from power_grid_model.enum import CalculationType, ComponentAttributeFilterOptions
from power_grid_model.validation import assert_valid_batch_data


@pytest.fixture
def input_data_r():
    node = initialize_array(DatasetType.input, ComponentType.node, 3)
    node["id"] = np.array([1, 2, 6])
    node["u_rated"] = [10.5e3, 10.5e3, 10.5e3]

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

    sym_load = initialize_array(DatasetType.input, ComponentType.sym_load, 2)
    sym_load["id"] = [4, 7]
    sym_load["node"] = [2, 6]
    sym_load["status"] = [1, 1]
    sym_load["type"] = [LoadGenType.const_power, LoadGenType.const_power]
    sym_load["p_specified"] = [20e6, 10e6]
    sym_load["q_specified"] = [5e6, 2e6]

    source = initialize_array(DatasetType.input, ComponentType.source, 1)
    source["id"] = [10]
    source["node"] = [1]
    source["status"] = [1]
    source["u_ref"] = [1.0]

    return {
        ComponentType.node: node,
        ComponentType.line: line,
        ComponentType.sym_load: sym_load,
        ComponentType.source: source,
    }


@pytest.fixture
def input_data_c(input_data_r):
    return compatibility_convert_row_columnar_dataset(
        input_data_r, ComponentAttributeFilterOptions.relevant, DatasetType.input
    )


@pytest.fixture(params=["input_data_r", "input_data_c"])
def input_data(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def update_sym_load_r():
    sym_load = initialize_array(DatasetType.update, ComponentType.sym_load, (2, 2))
    sym_load["id"] = [[4], [7]]
    sym_load["p_specified"] = [[30e6], [15e6]]
    return sym_load


@pytest.fixture
def update_sym_load_no_id_r():
    sym_load = initialize_array(DatasetType.update, ComponentType.sym_load, (2, 2))
    sym_load["p_specified"] = [[30e6, 10e6], [30e6, 15e6]]
    return sym_load


@pytest.fixture
def update_sym_load_c():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.update][ComponentType.sym_load].dtype
    return {
        "id": np.array([[4], [7]], dtype=source_attribute_dtypes["id"]),
        "p_specified": np.array([[30e6], [15e6]], dtype=source_attribute_dtypes["p_specified"]),
    }


@pytest.fixture
def update_sym_load_no_id_c():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.update][ComponentType.sym_load].dtype
    return {
        "p_specified": np.array([[30e6, 10e6], [30e6, 15e6]], dtype=source_attribute_dtypes["p_specified"]),
    }


@pytest.fixture(
    params=[
        "update_sym_load_r",
        "update_sym_load_no_id_r",
        "update_sym_load_c",
        "update_sym_load_no_id_c",
    ]
)
def update_sym_load(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def update_line_r():
    line = initialize_array(DatasetType.update, ComponentType.line, (2, 1))
    line["id"] = [[3], [5]]
    line["from_status"] = [[0], [0]]
    return line


@pytest.fixture
def update_line_no_id_r():
    line = initialize_array(DatasetType.update, ComponentType.line, (2, 3))
    line["from_status"] = [[0, 1, 1], [0, 0, 1]]
    return line


@pytest.fixture
def update_line_c():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.update][ComponentType.line].dtype
    return {
        "id": np.array([[3], [5]], dtype=source_attribute_dtypes["id"]),
        "from_status": np.array([[0], [0]], dtype=source_attribute_dtypes["from_status"]),
    }


@pytest.fixture
def update_line_no_id_c():
    source_attribute_dtypes = power_grid_meta_data[DatasetType.update][ComponentType.line].dtype
    return {
        "from_status": np.array([[0, 1, 1], [0, 0, 1]], dtype=source_attribute_dtypes["from_status"]),
    }


@pytest.fixture(
    params=[
        "update_line_r",
        "update_line_no_id_r",
        "update_line_c",
        "update_line_no_id_c",
    ]
)
def update_line(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def update_data(update_sym_load, update_line):
    return {
        ComponentType.sym_load: update_sym_load,
        ComponentType.line: update_line,
    }


def test_power_flow(input_data, update_data):
    model = PowerGridModel(input_data)
    assert_valid_batch_data(input_data=input_data, update_data=update_data, calculation_type=CalculationType.power_flow)
    model.calculate_power_flow(update_data=update_data)
