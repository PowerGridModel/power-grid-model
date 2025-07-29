# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power grid model buffer handler tests
"""

import numpy as np
import pytest

from power_grid_model._core.buffer_handling import (
    _get_dense_buffer_properties,
    _get_raw_attribute_data_view,
    _get_sparse_buffer_properties,
    get_buffer_view,
)
from power_grid_model._core.dataset_definitions import ComponentType, DatasetType
from power_grid_model._core.power_grid_meta import initialize_array, power_grid_meta_data

SINGLE_DATASET_NDIM = 1
BATCH_DATASET_NDIM = 2
SCENARIO_TOTAL_ELEMENTS = 4
BATCH_TOTAL_ELEMENTS = 8


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
    batch_size = BATCH_DATASET_NDIM if is_batch else None
    properties = _get_dense_buffer_properties(data, schema=schema, is_batch=is_batch, batch_size=batch_size)

    assert not properties.is_sparse
    assert properties.is_batch == is_batch
    assert properties.batch_size == (BATCH_DATASET_NDIM if is_batch else SINGLE_DATASET_NDIM)
    assert properties.n_elements_per_scenario == SCENARIO_TOTAL_ELEMENTS
    assert properties.n_total_elements == BATCH_TOTAL_ELEMENTS if is_batch else 4
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
    assert properties.batch_size == BATCH_DATASET_NDIM
    assert properties.n_elements_per_scenario == -1
    assert properties.n_total_elements == BATCH_TOTAL_ELEMENTS
    if is_columnar:
        assert properties.columns == list(data["data"].keys())
    else:
        assert properties.columns is None


@pytest.mark.parametrize(
    "component, is_batch, is_columnar, is_sparse",
    [
        pytest.param(ComponentType.sym_load, True, True, False, id="sym_load-columnar"),
        pytest.param(ComponentType.asym_load, True, True, False, id="asym_load-columnar"),
    ],
)
def test__get_raw_attribute_data_view(component, is_batch, is_columnar, is_sparse):
    schema = power_grid_meta_data[DatasetType.update][component]

    data = load_data(
        component_type=component,
        is_batch=is_batch,
        is_columnar=is_columnar,
        is_sparse=is_sparse,
    )
    asym_dense_batch_last_dim = 3
    if component == ComponentType.asym_load:
        assert data["p_specified"].shape[-1] == asym_dense_batch_last_dim

    buffer_view = get_buffer_view(
        data,
        schema=schema,
        is_batch=is_batch,
    )
    assert buffer_view is not None
    assert buffer_view.batch_size == BATCH_DATASET_NDIM
    assert buffer_view.n_elements_per_scenario == SCENARIO_TOTAL_ELEMENTS
    assert buffer_view.total_elements == BATCH_TOTAL_ELEMENTS


@pytest.mark.parametrize(
    "component, attribute",
    [
        pytest.param(ComponentType.asym_load, "p_specified", id="asym_load-shape_missmatch"),
    ],
)
def test__get_raw_attribute_data_view_fail(component, attribute):
    schema = power_grid_meta_data[DatasetType.update][component]

    data = load_data(
        component_type=component,
        is_batch=True,
        is_columnar=True,
        is_sparse=False,
    )

    old_shape = data[attribute].shape
    new_shape = list(old_shape)
    # because in _get_raw_attribute_data_view we check if the dimension of the last element is not 3 to raise an error
    new_shape[-1] = 2
    data[attribute] = np.zeros(new_shape, dtype=data[attribute].dtype)

    asym_dense_batch_last_dim = 3
    unsupported_asym_dense_batch_last_dim = 2
    updated_shape = data[attribute].shape
    assert old_shape != updated_shape
    assert old_shape[-1] == asym_dense_batch_last_dim
    assert updated_shape[-1] == unsupported_asym_dense_batch_last_dim

    with pytest.raises(ValueError, match="Given data has a different schema than supported."):
        get_buffer_view(data, schema=schema, is_batch=True)


@pytest.mark.parametrize(
    "component, is_batch, is_columnar, is_sparse",
    [
        pytest.param(ComponentType.sym_load, True, True, False, id="sym_load"),
        pytest.param(ComponentType.asym_load, True, True, False, id="asym_load-columnar"),
    ],
)
def test__get_raw_attribute_data_view_direct(component, is_batch, is_columnar, is_sparse):
    attribute = "p_specified"
    schema = power_grid_meta_data[DatasetType.update][component]

    data = load_data(
        component_type=component,
        is_batch=is_batch,
        is_columnar=is_columnar,
        is_sparse=is_sparse,
    )

    attribute_data = data[attribute]

    _get_raw_attribute_data_view(attribute_data, schema, "p_specified")


@pytest.mark.parametrize(
    "component, attr_data_shape, attribute",
    [
        pytest.param(ComponentType.asym_load, (2, 4, 3), "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (1, 4, 3), "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 5, 3), "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 6, 3), "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, 3, "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (3, 3), "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 4, 3), "q_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (1, 4, 3), "q_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 5, 3), "q_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 6, 3), "q_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, 3, "q_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (3, 3), "q_specified", id="asym_load-columnar"),
        # sym component
        pytest.param(ComponentType.sym_load, 3, "p_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (3, 3), "p_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (4, 3), "p_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (4, 0), "p_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, 0, "p_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, 3, "q_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (3, 3), "q_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (4, 3), "q_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (4, 0), "q_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (0, 4), "q_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, 0, "q_specified", id="sym_load-columnar"),
    ],
)
def test__get_raw_attribute_data_view_directly(component, attr_data_shape, attribute):
    arr = np.zeros(attr_data_shape)
    schema = power_grid_meta_data[DatasetType.update][component]

    _get_raw_attribute_data_view(arr, schema, attribute)


@pytest.mark.parametrize(
    "component, attr_data_shape, attribute",
    [
        pytest.param(ComponentType.asym_load, (2, 4, 4), "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 6, 4), "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, 0, "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 0, 0), "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 4, 0), "p_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 4, 4), "q_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 6, 4), "q_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, 0, "q_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 0, 0), "q_specified", id="asym_load-columnar"),
        pytest.param(ComponentType.asym_load, (2, 4, 0), "q_specified", id="asym_load-columnar"),
        # sym component
        pytest.param(ComponentType.sym_load, (2, 4, 4), "p_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (2, 4, 3), "p_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (0, 0, 1), "p_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (2, 4, 4), "q_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (2, 4, 3), "q_specified", id="sym_load-columnar"),
        pytest.param(ComponentType.sym_load, (0, 0, 1), "q_specified", id="sym_load-columnar"),
    ],
)
def test__get_raw_attribute_data_view_directly_fail(component, attr_data_shape, attribute):
    arr = np.zeros(attr_data_shape)
    schema = power_grid_meta_data[DatasetType.update][component]

    with pytest.raises(ValueError, match="Given data has a different schema than supported."):
        _get_raw_attribute_data_view(arr, schema, attribute)
