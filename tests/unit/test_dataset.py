# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import numpy as np
import pytest

from power_grid_model._core.dataset_definitions import ComponentType, DatasetType
from power_grid_model._core.power_grid_dataset import CConstDataset
from power_grid_model._core.power_grid_meta import power_grid_meta_data
from power_grid_model.errors import PowerGridError


def input_dataset_types():
    return [DatasetType.input]


def update_dataset_types():
    return [DatasetType.update]


def output_dataset_types():
    return [DatasetType.sym_output, DatasetType.asym_output, DatasetType.sc_output]


def all_dataset_types():
    return input_dataset_types() + update_dataset_types() + output_dataset_types()


@pytest.fixture(params=all_dataset_types())
def dataset_type(request):
    return request.param


def test_const_dataset__empty_dataset(dataset_type):
    dataset = CConstDataset(data={}, dataset_type=dataset_type)
    info = dataset.get_info()

    assert info.name() == dataset_type
    assert info.dataset_type() == dataset_type

    assert not info.is_batch()
    assert info.batch_size() == 1
    assert info.n_components() == 0
    assert info.components() == []
    assert info.elements_per_scenario() == {}
    assert info.total_elements() == {}

    with pytest.raises(ValueError):
        CConstDataset(data={})


def test_const_dataset__conflicting_data():
    with pytest.raises(PowerGridError):
        CConstDataset(
            data={
                ComponentType.node: np.zeros(1, dtype=power_grid_meta_data["input"][ComponentType.node]),
                "sym_load": np.zeros(1, dtype=power_grid_meta_data["update"]["sym_load"]),
            }
        )


def test_const_dataset__single_data(dataset_type):
    components = {ComponentType.node: 3, ComponentType.sym_load: 2, ComponentType.asym_load: 4}
    data = {
        component: np.zeros(shape=count, dtype=power_grid_meta_data[dataset_type][component])
        for component, count in components.items()
    }

    dataset = CConstDataset(data, dataset_type)
    info = dataset.get_info()

    assert info.name() == dataset_type
    assert info.dataset_type() == dataset_type

    assert not info.is_batch()
    assert info.batch_size() == 1
    assert info.n_components() == len(components)
    assert info.components() == list(components)
    assert info.elements_per_scenario() == components
    assert info.total_elements() == components


@pytest.mark.parametrize("batch_size", (0, 1, 3))
def test_const_dataset__uniform_batch_data(dataset_type, batch_size):
    components = {ComponentType.node: 3, ComponentType.sym_load: 2, ComponentType.asym_load: 4}
    data = {
        component: np.zeros(shape=(batch_size, count), dtype=power_grid_meta_data[dataset_type][component])
        for component, count in components.items()
    }

    dataset = CConstDataset(data, dataset_type)
    info = dataset.get_info()

    assert info.name() == dataset_type
    assert info.dataset_type() == dataset_type

    assert info.is_batch()
    assert info.batch_size() == batch_size
    assert info.n_components() == len(components)
    assert info.components() == list(components)
    assert info.elements_per_scenario() == components
    assert info.total_elements() == {component: batch_size * count for component, count in components.items()}


def test_const_dataset__sparse_batch_data(dataset_type):
    batch_size = 3
    components = {ComponentType.node: 3, ComponentType.sym_load: 2, ComponentType.asym_load: 4, ComponentType.link: 4}
    data = {
        ComponentType.node: {
            "data": np.zeros(shape=3, dtype=power_grid_meta_data[dataset_type][ComponentType.node]),
            "indptr": np.array([0, 2, 3, 3]),
        },
        ComponentType.sym_load: {
            "data": np.zeros(shape=2, dtype=power_grid_meta_data[dataset_type]["sym_load"]),
            "indptr": np.array([0, 0, 1, 2]),
        },
        ComponentType.asym_load: {
            "data": np.zeros(shape=4, dtype=power_grid_meta_data[dataset_type]["asym_load"]),
            "indptr": np.array([0, 2, 3, 4]),
        },
        ComponentType.link: np.zeros(shape=(batch_size, 4), dtype=power_grid_meta_data[dataset_type]["link"]),
    }

    dataset = CConstDataset(data, dataset_type)
    info = dataset.get_info()

    assert info.name() == dataset_type
    assert info.dataset_type() == dataset_type

    assert info.is_batch()
    assert info.batch_size() == 3
    assert info.n_components() == len(components)
    assert info.components() == list(components)
    assert info.elements_per_scenario() == {
        ComponentType.node: -1,
        ComponentType.sym_load: -1,
        ComponentType.asym_load: -1,
        ComponentType.link: 4,
    }
    assert info.total_elements() == {
        ComponentType.node: 3,
        ComponentType.sym_load: 2,
        ComponentType.asym_load: 4,
        ComponentType.link: batch_size * 4,
    }


def test_const_dataset__mixed_batch_size(dataset_type):
    data = {
        ComponentType.node: np.zeros(shape=(2, 3), dtype=power_grid_meta_data[dataset_type][ComponentType.node]),
        ComponentType.line: np.zeros(shape=(3, 3), dtype=power_grid_meta_data[dataset_type][ComponentType.line]),
    }
    with pytest.raises(ValueError):
        CConstDataset(data, dataset_type)


@pytest.mark.parametrize("bad_indptr", (np.ndarray([0, 1]), np.ndarray([0, 3, 2]), np.ndarray([0, 1, 2, 3, 4])))
def test_const_dataset__bad_sparse_data(dataset_type, bad_indptr):
    data = {
        ComponentType.node: {
            "data": np.zeros(shape=2, dtype=power_grid_meta_data[dataset_type][ComponentType.node]),
            "indptr": bad_indptr,
        },
    }
    with pytest.raises(TypeError):
        CConstDataset(data, dataset_type)


@pytest.mark.parametrize(
    ("dtype", "supported"),
    [
        (power_grid_meta_data[DatasetType.input][ComponentType.node].dtype["id"], True),
        ("<i4", True),
        ("<i8", False),
        ("<i1", False),
        ("<f8", False),
    ],
)
def test_const_dataset__different_dtype(dataset_type, dtype, supported):
    data = {
        ComponentType.node: {
            "id": np.zeros(shape=3, dtype=dtype),
        }
    }
    if supported:
        result = CConstDataset(data, dataset_type)
        assert result.get_info().total_elements() == {ComponentType.node: 3}
    else:
        with pytest.raises(ValueError):
            CConstDataset(data, dataset_type)
