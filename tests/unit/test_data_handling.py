# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import warnings
from functools import partial

import numpy as np
import pytest

from power_grid_model._core.data_handling import create_output_data
from power_grid_model._core.dataset_definitions import ComponentType as CT, DatasetType as DT
from power_grid_model._core.power_grid_core import VoidPtr
from power_grid_model._core.power_grid_dataset import CMutableDataset
from power_grid_model._core.power_grid_meta import initialize_array, power_grid_meta_data
from power_grid_model._utils import is_columnar
from power_grid_model.enum import ComponentAttributeFilterOptions


def columnar_array(component_type, n_components, attributes=None, batch_size_tuple=()):
    component_dtype = power_grid_meta_data[DT.sym_output][component_type].dtype
    required_attributes = (
        set(component_dtype.names) & set(attributes) if attributes is not None else component_dtype.names
    )
    return {
        attr: np.empty((n_components,) + batch_size_tuple, dtype=component_dtype[attr]) for attr in required_attributes
    }


def row_array(component_type, n_components, batch_size_tuple=()):
    return initialize_array(DT.sym_output, component_type, (n_components,) + batch_size_tuple)


@pytest.fixture(params=[1, 15])
def batch_size(request):
    return request.param


@pytest.mark.parametrize(
    ("output_component_types", "expected_fns"),
    [
        (
            None,
            {
                CT.node: partial(row_array, component_type=CT.node, n_components=4),
                CT.sym_load: partial(row_array, component_type=CT.sym_load, n_components=3),
                CT.source: partial(row_array, component_type=CT.source, n_components=1),
            },
        ),
        (
            [CT.node, CT.sym_load],
            {
                CT.node: partial(row_array, component_type=CT.node, n_components=4),
                CT.sym_load: partial(row_array, component_type=CT.sym_load, n_components=3),
            },
        ),
        (
            {CT.node, CT.sym_load},
            {
                CT.node: partial(row_array, component_type=CT.node, n_components=4),
                CT.sym_load: partial(row_array, component_type=CT.sym_load, n_components=3),
            },
        ),
        pytest.param(
            ComponentAttributeFilterOptions.relevant,
            {
                CT.node: partial(columnar_array, component_type=CT.node, n_components=4, attributes=None),
                CT.sym_load: partial(columnar_array, component_type=CT.sym_load, n_components=3, attributes=None),
                CT.source: partial(columnar_array, component_type=CT.source, n_components=1, attributes=None),
            },
        ),
        pytest.param(
            {
                CT.node: ComponentAttributeFilterOptions.everything,
                CT.sym_load: ComponentAttributeFilterOptions.relevant,
            },
            {
                CT.node: partial(columnar_array, component_type=CT.node, n_components=4, attributes=None),
                CT.sym_load: partial(columnar_array, component_type=CT.sym_load, n_components=3, attributes=None),
            },
        ),
        pytest.param(
            {CT.node: ComponentAttributeFilterOptions.relevant, CT.sym_load: ["p"]},
            {
                CT.node: partial(columnar_array, component_type=CT.node, n_components=4, attributes=None),
                CT.sym_load: partial(columnar_array, component_type=CT.sym_load, n_components=3, attributes=["p"]),
            },
        ),
        pytest.param(
            {CT.node: None, CT.sym_load: ["p"]},
            {
                CT.node: partial(row_array, component_type=CT.node, n_components=4),
                CT.sym_load: partial(columnar_array, component_type=CT.sym_load, n_components=3, attributes=["p"]),
            },
        ),
        pytest.param(
            {CT.node: [], CT.sym_load: []},
            {
                CT.node: partial(columnar_array, component_type=CT.node, n_components=4, attributes=[]),
                CT.sym_load: partial(columnar_array, component_type=CT.sym_load, n_components=3, attributes=[]),
            },
        ),
        pytest.param(
            {CT.node: [], CT.sym_load: ["p"]},
            {
                CT.node: partial(columnar_array, component_type=CT.node, n_components=4, attributes=[]),
                CT.sym_load: partial(columnar_array, component_type=CT.sym_load, n_components=3, attributes=["p"]),
            },
        ),
        pytest.param(
            {CT.node: ["u"], CT.sym_load: ["p"]},
            {
                CT.node: partial(columnar_array, component_type=CT.node, n_components=4, attributes=["u"]),
                CT.sym_load: partial(columnar_array, component_type=CT.sym_load, n_components=3, attributes=["p"]),
            },
        ),
    ],
)
def test_create_output_data(output_component_types, expected_fns, batch_size):
    """Test output_data creation. Batch size is set later in the test."""
    all_component_count = {CT.node: 4, CT.sym_load: 3, CT.source: 1}
    actual = create_output_data(
        output_component_types=output_component_types,
        output_type=DT.sym_output,
        all_component_count=all_component_count,
        is_batch=False if batch_size == 1 else True,
        batch_size=batch_size,
    )

    expected = {comp: fn(batch_size_tuple=(batch_size,)) for comp, fn in expected_fns.items()}
    assert actual.keys() == expected.keys()
    for comp in expected:
        if not is_columnar(expected[comp]):
            assert actual[comp].dtype == expected[comp].dtype
        elif expected[comp] == dict():
            # Empty attributes columnar
            assert actual[comp] == expected[comp]
        else:
            assert actual[comp].keys() == expected[comp].keys()
            assert all(actual[comp][attr].dtype == expected[comp][attr].dtype for attr in expected[comp])


def test_dtype_compatibility_check_normal():
    nodes = initialize_array(DT.sym_output, CT.node, (1, 2))
    nodes_ptr = nodes.ctypes.data_as(VoidPtr)

    data = {CT.node: nodes}
    mutable_dataset = CMutableDataset(data, DT.sym_output)
    buffer_views = mutable_dataset.get_buffer_views()

    assert buffer_views[0].data.value == nodes_ptr.value


def test_dtype_compatibility_check_compatible():
    nodes = initialize_array(DT.sym_output, CT.node, 4)
    nodes = nodes[::2]
    nodes_ptr = nodes.ctypes.data_as(VoidPtr)

    data = {CT.node: nodes}
    with warnings.catch_warnings():
        warnings.simplefilter("error")
        mutable_dataset = CMutableDataset(data, DT.sym_output)
        buffer_views = mutable_dataset.get_buffer_views()

    assert buffer_views[0].data.value != nodes_ptr.value


def test_dtype_compatibility_check__error():
    nodes = initialize_array(DT.sym_output, CT.node, (1, 2))
    data = {CT.node: nodes.astype(nodes.dtype.newbyteorder("S"))}
    with pytest.raises(ValueError):
        CMutableDataset(data, DT.sym_output)
