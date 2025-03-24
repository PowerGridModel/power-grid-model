# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power grid model buffer handler tests
"""

import numpy as np
import pytest

from power_grid_model._core.buffer_handling import _get_dense_buffer_properties, _get_sparse_buffer_properties
from power_grid_model._core.dataset_definitions import ComponentType, DatasetType
from power_grid_model._core.power_grid_meta import initialize_array, power_grid_meta_data


def load_data(component_type, is_batch, is_sparse, is_columnar):
    """Creates load data of different formats for testing"""
    shape = (2, 4) if is_batch else (4,)
    load = initialize_array(DatasetType.update, component_type, shape)
    columnar_names = ["p_specified", "q_specified"]

    if is_columnar:
        if is_sparse:
            return {"indptr": np.array([0, 5, 8]), "data": {k: load.reshape(-1)[k] for k in columnar_names}}
        return {k: load[k] for k in columnar_names}
    if is_sparse:
        return {"indptr": np.array([0, 5, 8]), "data": load.reshape(-1)}
    return load


@pytest.mark.parametrize(
    "component_type, is_batch, is_columnar",
    [
        pytest.param(ComponentType.sym_load, True, True, id="sym_load-batch-columnar"),
        pytest.param(ComponentType.sym_load, True, False, id="sym_load-batch-row_based"),
        pytest.param(ComponentType.sym_load, False, True, id="sym_load-single-columnar"),
        pytest.param(ComponentType.sym_load, False, False, id="sym_load-single-row_based"),
        pytest.param(ComponentType.asym_load, True, True, id="asym_load-batch-columnar"),
        pytest.param(ComponentType.asym_load, True, False, id="asym_load-batch-row_based"),
        pytest.param(ComponentType.asym_load, False, True, id="asym_load-single-columnar"),
        pytest.param(ComponentType.asym_load, False, False, id="asym_load-single-row_based"),
    ],
)
def test__get_dense_buffer_properties(component_type, is_batch, is_columnar):
    data = load_data(component_type, is_batch=is_batch, is_columnar=is_columnar, is_sparse=False)
    schema = power_grid_meta_data[DatasetType.update][component_type]
    batch_size = 2 if is_batch else None
    properties = _get_dense_buffer_properties(data, schema=schema, is_batch=is_batch, batch_size=batch_size)

    assert not properties.is_sparse
    assert properties.is_batch == is_batch
    assert properties.batch_size == (2 if is_batch else 1)
    assert properties.n_elements_per_scenario == 4
    assert properties.n_total_elements == 8 if is_batch else 4
    if is_columnar:
        assert properties.columns == list(data.keys())
    else:
        assert properties.columns is None


@pytest.mark.parametrize(
    "component_type, is_columnar",
    [
        pytest.param(ComponentType.sym_load, True, id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, False, id="sym_load-row_based"),
        pytest.param(ComponentType.asym_load, True, id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, False, id="asym_load-row_based"),
    ],
)
def test__get_sparse_buffer_properties(component_type, is_columnar):
    data = load_data(component_type, is_batch=True, is_columnar=is_columnar, is_sparse=True)

    schema = power_grid_meta_data[DatasetType.update][component_type]
    properties = _get_sparse_buffer_properties(data, schema=schema, batch_size=2)

    assert properties.is_sparse
    assert properties.is_batch
    assert properties.batch_size == 2
    assert properties.n_elements_per_scenario == -1
    assert properties.n_total_elements == 8
    if is_columnar:
        assert properties.columns == list(data["data"].keys())
    else:
        assert properties.columns is None
