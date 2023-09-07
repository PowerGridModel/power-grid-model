# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Power grid model raw dataset handler
"""

from typing import Dict, List, Mapping, Tuple, Union

import numpy as np

from power_grid_model.core.error_handling import VALIDATOR_MSG, assert_no_error
from power_grid_model.core.index_integer import IdxNp
from power_grid_model.core.power_grid_core import ConstDatasetPtr, DatasetInfoPtr, IdxPtr, VoidPtr, WritableDatasetPtr
from power_grid_model.core.power_grid_core import power_grid_core as pgc
from power_grid_model.core.power_grid_meta import CBuffer, power_grid_meta_data


class CDatasetInfo:  # pylint: disable=too-few-public-methods
    """
    Raw Power Grid Model dataset info.
    """

    def __init__(self, info: DatasetInfoPtr):
        self._info: DatasetInfoPtr = info

    def name(self) -> str:
        """
        The name of the dataset type.

        Returns:
            The name of the dataset type
        """
        return pgc.dataset_info_name(self._info)

    def dataset_type(self):
        """ "
        The name of the dataset type.

        Returns:
            The name of the dataset type
        """
        return self.name()

    def is_batch(self) -> bool:
        """ "
        Whether the dataset is a batch dataset.

        Returns:
            Whether the dataset is a batch dataset
        """
        return bool(pgc.dataset_info_is_batch(self._info))

    def batch_size(self) -> int:
        """ "
        The size of the dataset.

        Returns:
            The size of the dataset
        """
        return pgc.dataset_info_batch_size(self._info)

    def n_components(self) -> int:
        """ "
        The amount of components in the dataset.

        Returns:
            The amount of components in the dataset
        """
        return pgc.dataset_info_n_components(self._info)

    def components(self) -> List[str]:
        """ "
        The components in the dataset.

        Returns:
            A list of the component names in the dataset
        """
        return [pgc.dataset_info_component_name(self._info, idx) for idx in range(self.n_components())]

    def elements_per_scenario(self) -> Dict[str, int]:
        """ "
        The number of elements per scenario in the dataset.

        Returns:
            The number of elements per senario for each component in the dataset;
            or -1 if the scenario is not uniform (different amount per scenario)
        """
        return {
            component_name: pgc.dataset_info_elements_per_scenario(self._info, idx)
            for idx, component_name in enumerate(self.components())
        }

    def total_elements(self) -> Dict[str, int]:
        """ "
        The total number of elements in the dataset.

        Returns:
            The total number of elements for each component.
            For each component, if the number of elements per scenario is uniform, its value shall be equal to
                the product of the batch size and the amount of elements per scenario for that component.
        """
        return {
            component_name: pgc.dataset_info_total_elements(self._info, idx)
            for idx, component_name in enumerate(self.components())
        }


class SubDatasetInfo:  # pylint: disable=too-few-public-methods
    """Helper class to collect info on the dataset."""

    is_sparse: bool
    is_batch: bool
    batch_size: int
    n_elements_per_scenario: int
    n_total_elements: int

    def __init__(  # pylint: disable=too-many-arguments
        self, is_sparse: bool, is_batch: bool, batch_size: int, n_elements_per_scenario: int, n_total_elements: int
    ):
        self.is_sparse = is_sparse
        self.is_batch = is_batch
        self.batch_size = batch_size
        self.n_elements_per_scenario = n_elements_per_scenario
        self.n_total_elements = n_total_elements


def get_homogeneous_sub_data_info(data: np.ndarray) -> SubDatasetInfo:
    """
    Extract the data of the homogeneous batch dataset component.

    Args:
        data (np.ndarray): the dataset component.

    Raises:
        KeyError if the dataset component is not sparse.
        ValueError if the dataset component contains conflicting or bad data.

    Returns:
        the details on the dataset component.
    """
    is_sparse = False

    if data.ndim not in (1, 2):
        raise ValueError(f"Array can only be 1D or 2D. {VALIDATOR_MSG}")

    is_batch = data.ndim == 2
    batch_size = data.shape[0] if is_batch else 1
    n_elements_per_scenario = data.shape[-1]
    n_total_elements = batch_size * n_elements_per_scenario

    return SubDatasetInfo(
        is_sparse=is_sparse,
        is_batch=is_batch,
        batch_size=batch_size,
        n_elements_per_scenario=n_elements_per_scenario,
        n_total_elements=n_total_elements,
    )


def get_sparse_sub_data_info(data: Mapping[str, np.ndarray]) -> SubDatasetInfo:
    """
    Extract the data of the sparse batch dataset component.

    Args:
        data (Mapping[str, np.ndarray]): the sparse dataset component.

    Raises:
        KeyError if the dataset component is not sparse.
        ValueError if the dataset component contains conflicting or bad data.

    Returns:
        SubDatasetInfo: the details on the dataset component.
    """
    is_sparse = True

    contents = data["data"]
    indptr = data["indptr"]

    if contents.ndim != 1:
        raise ValueError(f"Data array can only be 1D. {VALIDATOR_MSG}")
    if indptr.ndim != 1:
        raise ValueError(f"indptr can only be 1D. {VALIDATOR_MSG}")
    if indptr[0] != 0 or indptr[-1] != contents.size:
        raise ValueError(f"indptr should start from zero and end at size of data array. {VALIDATOR_MSG}")
    if np.any(np.diff(indptr) < 0):
        raise ValueError(f"indptr should be increasing. {VALIDATOR_MSG}")

    is_batch = True
    batch_size = indptr.size - 1
    n_elements_per_scenario = -1
    n_total_elements = contents.size

    return SubDatasetInfo(
        is_sparse=is_sparse,
        is_batch=is_batch,
        batch_size=batch_size,
        n_elements_per_scenario=n_elements_per_scenario,
        n_total_elements=n_total_elements,
    )


def get_sub_data_info(data: Union[np.ndarray, Mapping[str, np.ndarray]]) -> SubDatasetInfo:
    """
    Extract the data of the dataset component

    Args:
        data (Union[np.ndarray, Mapping[str, np.ndarray]]): the dataset component.

    Raises:
        ValueError if the dataset component contains conflicting or bad data.

    Returns:
        SubDatasetInfo: the details on the dataset component.
    """
    if isinstance(data, np.ndarray):
        return get_homogeneous_sub_data_info(data)

    return get_sparse_sub_data_info(data)


class CConstDataset:
    """
    A view of a user-owned dataset.

    This may be used to provide a user dataset to the Power Grid Model.

    The dataset will create read-only buffers that the Power Grid Model can use to load data.
    """

    def __init__(self, dataset_type: str, data: Dict[str, Union[np.ndarray, Mapping[str, np.ndarray]]]):
        self._dataset_type = dataset_type
        self._schema = power_grid_meta_data[self._dataset_type]

        if data:
            first_sub_info = get_sub_data_info(next(iter(data.values())))
            self._is_batch = first_sub_info.is_batch
            self._batch_size = first_sub_info.batch_size

            if self._is_batch and self._dataset_type == "input":
                raise ValueError(f"Input datasets cannot be batch dataset type. {VALIDATOR_MSG}")
        else:
            self._is_batch = False
            self._batch_size = 1

        self._const_dataset = pgc.create_dataset_const(self._dataset_type, self._is_batch, self._batch_size)
        assert_no_error()

        self.add_data(data)
        assert_no_error()

    def get_dataset_ptr(self) -> ConstDatasetPtr:
        """
        Get the raw underlying const dataset pointer.

        Returns:
            ConstDatasetPtr: the raw underlying const dataset pointer.
        """
        return self._const_dataset

    def get_info(self) -> CDatasetInfo:
        """
        Get the info for this dataset.

        Returns:
            The dataset info for this dataset.
        """
        return CDatasetInfo(pgc.dataset_const_get_info(self._const_dataset))

    def add_data(self, data: Dict[str, Union[np.ndarray, Mapping[str, np.ndarray]]]):
        """
        Add Power Grid Model data to the const dataset view.

        Args:
            data: the data.

        Raises:
            ValueError if the component is unknown and allow_unknown is False.
            ValueError if the data is inconsistent with the rest of the dataset.
            PowerGridError if there was an internal error.
        """
        for component, component_data in data.items():
            self.add_component_data(component, component_data, allow_unknown=True)

    def add_component_data(
        self, component: str, data: Union[np.ndarray, Mapping[str, np.ndarray]], allow_unknown: bool = False
    ):
        """
        Add Power Grid Model data for a single component to the const dataset view.

        Args:
            component: the name of the component
            data: the data of the component
            allow_unknown (optional): ignore any unknown components. Defaults to False.

        Raises:
            ValueError if the component is unknown and allow_unknown is False.
            ValueError if the data is inconsistent with the rest of the dataset.
            PowerGridError if there was an internal error.
        """
        if component not in self._schema:
            if not allow_unknown:
                raise ValueError(f"Unknown component {component} in schema. {VALIDATOR_MSG}")
            return

        buffer = self._get_buffer(component, data)
        self._add_buffer(component, buffer)

    def _get_buffer(self, component: str, data: Union[np.ndarray, Mapping[str, np.ndarray]]) -> CBuffer:
        if isinstance(data, np.ndarray):
            return self._get_homogeneous_buffer(component, data)

        return self._get_sparse_buffer(component, data)

    def _get_homogeneous_buffer(self, component: str, data: np.ndarray):
        info = get_homogeneous_sub_data_info(data)
        self._validate_sub_data_format(info)

        return CBuffer(
            data=self._raw_view(component, data),
            indptr=IdxPtr(),
            n_elements_per_scenario=info.n_elements_per_scenario,
            batch_size=info.batch_size,
            total_elements=info.n_total_elements,
        )

    def _get_sparse_buffer(self, component: str, data_mapping: Mapping[str, np.ndarray]) -> CBuffer:
        data = data_mapping["data"]
        indptr = data_mapping["indptr"]

        info = get_sparse_sub_data_info(data_mapping)
        self._validate_sub_data_format(info)

        return CBuffer(
            data=self._raw_view(component, data),
            indptr=self._get_indptr_view(indptr),
            n_elements_per_scenario=info.n_elements_per_scenario,
            batch_size=info.batch_size,
            total_elements=info.n_total_elements,
        )

    def _add_buffer(self, component, buffer: CBuffer):
        pgc.dataset_const_add_buffer(
            self._const_dataset,
            component,
            buffer.n_elements_per_scenario,
            buffer.total_elements,
            buffer.indptr,
            buffer.data,
        )
        assert_no_error()

    def _validate_sub_data_format(self, info: SubDatasetInfo):
        if info.is_batch != self._is_batch:
            raise ValueError(
                f"Dataset type (single or batch) must be consistent across all components. {VALIDATOR_MSG}"
            )
        if info.batch_size != self._batch_size:
            raise ValueError(f"Dataset must have a consistent batch size across all components. {VALIDATOR_MSG}")

    def _raw_view(self, component: str, data: np.ndarray):
        return np.ascontiguousarray(data, dtype=self._schema[component].dtype).ctypes.data_as(VoidPtr)

    def _get_indptr_view(self, indptr: np.ndarray):
        return np.ascontiguousarray(indptr, dtype=IdxNp).ctypes.data_as(IdxPtr)

    def __del__(self):
        pgc.destroy_dataset_const(self._const_dataset)


class CWritableDataset:
    """
    A view of a Power Grid Model-owned dataset.

    This may be used to retrieve data from the Power Grid Model.

    This class provides buffers to which the Power Grid Model can write data in an external call.
    After writing to the buffers, the data contents can be retrieved.
    """

    def __init__(self, dataset_ptr: WritableDatasetPtr):
        self._writable_dataset = dataset_ptr

        self._info = self.get_info()
        self._dataset_type = self._info.dataset_type()
        self._schema = power_grid_meta_data[self._dataset_type]

        self._component_sub_data_info = self._get_sub_data_info(self._info)
        self._data: Dict[str, Union[np.ndarray, Mapping[str, np.ndarray]]] = {}
        self._buffers: Dict[str, CBuffer] = {}

        self._add_buffers(self._component_sub_data_info)

    def get_dataset_ptr(self) -> WritableDatasetPtr:
        """
        Get the raw underlying writable dataset pointer.

        Returns:
            WritableDatasetPtr: the raw underlying writable dataset pointer.
        """
        return self._writable_dataset

    def get_info(self) -> CDatasetInfo:
        """
        Get the info for this dataset.

        Returns:
            The dataset info for this dataset.
        """
        return CDatasetInfo(pgc.dataset_writable_get_info(self._writable_dataset))

    def get_data(self) -> Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]:
        """
        Retrieve data from the Power Grid Model dataset.

        The Power Grid Model may write to these buffers at a later point in time.

        Returns:
            The full dataset.
        """
        return self._data

    def get_component_data(self, component: str) -> Union[np.ndarray, Dict[str, np.ndarray]]:
        """
        Retrieve Power Grid Model data from the dataset for a specific component.

        Args:
            component: the component.

        Returns:
            The dataset for the specified component.
        """
        return self._data[component]

    def _add_buffers(self, component_sub_data_info: Dict[str, SubDatasetInfo]):
        for component, component_info in component_sub_data_info.items():
            self._component_sub_data_info[component] = component_info
            self._add_buffer(component, component_info)

    def _add_buffer(self, component: str, component_info: SubDatasetInfo):
        data: Union[np.ndarray, Dict[str, np.ndarray]]
        buffer: CBuffer

        if component_info.is_sparse:
            data, buffer = self._create_sparse_buffer(component, component_info)
        else:
            data, buffer = self._create_homogeneous_buffer(component, component_info)

        pgc.dataset_writable_set_buffer(self._writable_dataset, component, buffer.indptr, buffer.data)
        self._data[component] = data

    def _create_homogeneous_buffer(self, component: str, info: SubDatasetInfo) -> Tuple[np.ndarray, CBuffer]:
        shape: Union[int, Tuple[int, int]] = (
            (info.batch_size, info.n_elements_per_scenario) if info.is_batch else info.n_elements_per_scenario
        )
        data = np.empty(shape=shape, dtype=self._schema[component])
        return data, CBuffer(
            data=self._raw_view(component, data),
            indptr=IdxPtr(),
            n_elements_per_scenario=info.n_elements_per_scenario,
            batch_size=info.batch_size,
            total_elements=info.n_total_elements,
        )

    def _create_sparse_buffer(self, component: str, info: SubDatasetInfo) -> Tuple[Dict[str, np.ndarray], CBuffer]:
        data = np.empty(info.n_total_elements, dtype=self._schema[component])
        indptr = np.empty([0] * info.n_total_elements + [info.batch_size])
        return {"data": data, "indptr": indptr}, CBuffer(
            data=self._raw_view(component, data),
            indptr=self._get_indptr_view(indptr),
            n_elements_per_scenario=info.n_elements_per_scenario,
            batch_size=info.batch_size,
            total_elements=info.n_total_elements,
        )

    def _raw_view(self, component: str, data: np.ndarray):
        return np.ascontiguousarray(data, dtype=self._schema[component].dtype).ctypes.data_as(VoidPtr)

    def _get_indptr_view(self, indptr: np.ndarray):
        return np.ascontiguousarray(indptr, dtype=IdxNp).ctypes.data_as(IdxPtr)

    @staticmethod
    def _get_sub_data_info(info: CDatasetInfo) -> Dict[str, SubDatasetInfo]:
        is_batch = info.is_batch()
        batch_size = info.batch_size()
        components = info.components()
        n_elements_per_scenario = info.elements_per_scenario()
        n_total_elements = info.total_elements()

        return {
            component: SubDatasetInfo(
                is_sparse=n_elements_per_scenario[component] == -1,
                is_batch=is_batch,
                batch_size=batch_size,
                n_elements_per_scenario=n_elements_per_scenario[component],
                n_total_elements=n_total_elements[component],
            )
            for component in components
        }
