# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import itertools

import numpy as np
import pytest

from power_grid_model.core.power_grid_dataset import CConstDataset, get_dataset_type
from power_grid_model.core.power_grid_meta import power_grid_meta_data
from power_grid_model.errors import PowerGridError


def input_dataset_types():
    return ["input"]


def update_dataset_types():
    return ["update"]


def output_dataset_types():
    return ["sym_output", "asym_output", "sc_output"]


def all_dataset_types():
    return input_dataset_types() + update_dataset_types() + output_dataset_types()


@pytest.fixture(params=all_dataset_types())
def dataset_type(request):
    return request.param


def test_get_dataset_type(dataset_type):
    assert (
        get_dataset_type(
            data={
                "node": np.zeros(1, dtype=power_grid_meta_data[dataset_type]["node"]),
                "sym_load": np.zeros(1, dtype=power_grid_meta_data[dataset_type]["sym_load"]),
            }
        )
        == dataset_type
    )


def test_get_dataset_type__empty_data():
    with pytest.raises(ValueError):
        get_dataset_type(data={})


def test_get_dataset_type__conflicting_data():
    for first, second in itertools.product(all_dataset_types(), all_dataset_types()):
        data = {
            "node": np.zeros(1, dtype=power_grid_meta_data[first]["node"]),
            "sym_load": np.zeros(1, dtype=power_grid_meta_data[second]["sym_load"]),
        }
        if first == second:
            assert get_dataset_type(data=data) == first
        else:
            with pytest.raises(PowerGridError):
                get_dataset_type(data=data)


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
                "node": np.zeros(1, dtype=power_grid_meta_data["input"]["node"]),
                "sym_load": np.zeros(1, dtype=power_grid_meta_data["update"]["sym_load"]),
            }
        )


def test_const_dataset__single_data(dataset_type):
    components = {"node": 3, "sym_load": 2, "asym_load": 4}
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
    components = {"node": 3, "sym_load": 2, "asym_load": 4}
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
    components = {"node": 3, "sym_load": 2, "asym_load": 4, "link": 4}
    data = {
        "node": {
            "data": np.zeros(shape=3, dtype=power_grid_meta_data[dataset_type]["node"]),
            "indptr": np.array([0, 2, 3, 3]),
        },
        "sym_load": {
            "data": np.zeros(shape=2, dtype=power_grid_meta_data[dataset_type]["sym_load"]),
            "indptr": np.array([0, 0, 1, 2]),
        },
        "asym_load": {
            "data": np.zeros(shape=4, dtype=power_grid_meta_data[dataset_type]["asym_load"]),
            "indptr": np.array([0, 2, 3, 4]),
        },
        "link": np.zeros(shape=(batch_size, 4), dtype=power_grid_meta_data[dataset_type]["link"]),
    }

    dataset = CConstDataset(data, dataset_type)
    info = dataset.get_info()

    assert info.name() == dataset_type
    assert info.dataset_type() == dataset_type

    assert info.is_batch()
    assert info.batch_size() == 3
    assert info.n_components() == len(components)
    assert info.components() == list(components)
    assert info.elements_per_scenario() == {"node": -1, "sym_load": -1, "asym_load": -1, "link": 4}
    assert info.total_elements() == {"node": 3, "sym_load": 2, "asym_load": 4, "link": batch_size * 4}


def test_const_dataset__mixed_batch_size(dataset_type):
    data = {
        "node": np.zeros(shape=(2, 3), dtype=power_grid_meta_data[dataset_type]["node"]),
        "line": np.zeros(shape=(3, 3), dtype=power_grid_meta_data[dataset_type]["line"]),
    }
    with pytest.raises(ValueError):
        CConstDataset(data, dataset_type)


@pytest.mark.parametrize("bad_indptr", (np.ndarray([0, 1]), np.ndarray([0, 3, 2]), np.ndarray([0, 1, 2, 3, 4])))
def test_const_dataset__bad_sparse_data(dataset_type, bad_indptr):
    data = {
        "node": {
            "data": np.zeros(shape=2, dtype=power_grid_meta_data[dataset_type]["node"]),
            "indptr": bad_indptr,
        },
    }
    with pytest.raises(ValueError):
        CConstDataset(data, dataset_type)
