# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Power grid model raw dataset handler
"""

from typing import Dict, List, Mapping, Optional, Union

import numpy as np

from power_grid_model.core.buffer_handling import (
    BufferProperties,
    CBuffer,
    create_buffer,
    get_buffer_properties,
    get_buffer_view,
)
from power_grid_model.core.error_handling import VALIDATOR_MSG, assert_no_error
from power_grid_model.core.power_grid_core import ConstDatasetPtr, DatasetInfoPtr, WritableDatasetPtr
from power_grid_model.core.power_grid_core import power_grid_core as pgc
from power_grid_model.core.power_grid_meta import DatasetMetaData, power_grid_meta_data
from power_grid_model.errors import PowerGridError


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
        """
        The name of the dataset type.

        Returns:
            The name of the dataset type
        """
        return self.name()

    def is_batch(self) -> bool:
        """
        Whether the dataset is a batch dataset.

        Returns:
            Whether the dataset is a batch dataset
        """
        return bool(pgc.dataset_info_is_batch(self._info))

    def batch_size(self) -> int:
        """
        The size of the dataset.

        Returns:
            The size of the dataset
        """
        return pgc.dataset_info_batch_size(self._info)

    def n_components(self) -> int:
        """
        The amount of components in the dataset.

        Returns:
            The amount of components in the dataset
        """
        return pgc.dataset_info_n_components(self._info)

    def components(self) -> List[str]:
        """
        The components in the dataset.

        Returns:
            A list of the component names in the dataset
        """
        return [pgc.dataset_info_component_name(self._info, idx) for idx in range(self.n_components())]

    def elements_per_scenario(self) -> Mapping[str, int]:
        """
        The number of elements per scenario in the dataset.

        Returns:
            The number of elements per senario for each component in the dataset;
            or -1 if the scenario is not uniform (different amount per scenario)
        """
        return {
            component_name: pgc.dataset_info_elements_per_scenario(self._info, idx)
            for idx, component_name in enumerate(self.components())
        }

    def total_elements(self) -> Mapping[str, int]:
        """
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


def get_dataset_type(data: Mapping[str, Union[np.ndarray, Mapping[str, np.ndarray]]]) -> str:
    """
    Deduce the dataset type from the provided dataset.

    Args:
        data: the dataset

    Raises:
        ValueError
            if the dataset type cannot be deduced because multiple dataset types match the format
            (probably because the data contained no supported components, e.g. was empty)
        PowerGridError
            if no dataset type matches the format of the data
            (probably because the data contained conflicting data formats)

    Returns:
        The dataset type.
    """
    candidates = set(power_grid_meta_data.keys())

    for dataset_type, dataset_metadatas in power_grid_meta_data.items():
        for component, dataset_metadata in dataset_metadatas.items():
            if component not in data:
                continue

            component_data = data[component]
            if isinstance(component_data, np.ndarray):
                component_dtype = component_data.dtype
            else:
                component_dtype = component_data["data"].dtype

            if component_dtype is not dataset_metadata.dtype:
                candidates.discard(dataset_type)
                break

    if not candidates:
        raise PowerGridError(
            "The dataset type could not be deduced because no type matches the data. "
            "This usually means inconsistent data was provided."
        )
    if len(candidates) > 1:
        raise ValueError("The dataset type could not be deduced because multiple dataset types match the data.")

    return next(iter(candidates))


class CConstDataset:
    """
    A view of a user-owned dataset.

    This may be used to provide a user dataset to the Power Grid Model.

    The dataset will create read-only buffers that the Power Grid Model can use to load data.
    """

    _dataset_type: str
    _schema: DatasetMetaData
    _is_batch: bool
    _batch_size: int
    _const_dataset: ConstDatasetPtr

    def __new__(
        cls,
        data: Union[Mapping[str, np.ndarray], Mapping[str, Union[np.ndarray, Mapping[str, np.ndarray]]]],
        dataset_type: Optional[str] = None,
    ):
        instance = super().__new__(cls)

        instance._dataset_type = dataset_type if isinstance(dataset_type, str) else get_dataset_type(data)
        instance._schema = power_grid_meta_data[instance._dataset_type]

        if data:
            first_sub_info = get_buffer_properties(next(iter(data.values())))
            instance._is_batch = first_sub_info.is_batch
            instance._batch_size = first_sub_info.batch_size

            if instance._is_batch and instance._dataset_type == "input":
                raise ValueError(f"Input datasets cannot be batch dataset type. {VALIDATOR_MSG}")
        else:
            instance._is_batch = False
            instance._batch_size = 1

        instance._const_dataset = pgc.create_dataset_const(
            instance._dataset_type, instance._is_batch, instance._batch_size
        )
        assert_no_error()

        instance.add_data(data)
        assert_no_error()

        return instance

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

    def add_data(
        self, data: Union[Mapping[str, np.ndarray], Mapping[str, Union[np.ndarray, Mapping[str, np.ndarray]]]]
    ):
        """
        Add Power Grid Model data to the const dataset view.

        Args:
            data: the data.

        Raises:
            ValueError: if the component is unknown and allow_unknown is False.
            ValueError: if the data is inconsistent with the rest of the dataset.
            PowerGridError: if there was an internal error.
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
            ValueError: if the component is unknown and allow_unknown is False.
            ValueError: if the data is inconsistent with the rest of the dataset.
            PowerGridError: if there was an internal error.
        """
        if component not in self._schema:
            if not allow_unknown:
                raise ValueError(f"Unknown component {component} in schema. {VALIDATOR_MSG}")
            return

        self._validate_properties(data)
        self._register_buffer(component, get_buffer_view(data, self._schema[component]))

    def _register_buffer(self, component, buffer: CBuffer):
        pgc.dataset_const_add_buffer(
            dataset=self._const_dataset,
            component=component,
            elements_per_scenario=buffer.n_elements_per_scenario,
            total_elements=buffer.total_elements,
            indptr=buffer.indptr,
            data=buffer.data,
        )
        assert_no_error()

    def _validate_properties(self, data: Union[np.ndarray, Mapping[str, np.ndarray]]):
        properties = get_buffer_properties(data)
        if properties.is_batch != self._is_batch:
            raise ValueError(
                f"Dataset type (single or batch) must be consistent across all components. {VALIDATOR_MSG}"
            )
        if properties.batch_size != self._batch_size:
            raise ValueError(f"Dataset must have a consistent batch size across all components. {VALIDATOR_MSG}")

    def __del__(self):
        if hasattr(self, "_const_dataset"):
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

        info = self.get_info()
        self._dataset_type = info.dataset_type()
        self._schema = power_grid_meta_data[self._dataset_type]

        self._component_buffer_properties = self._get_buffer_properties(info)
        self._data: Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]] = {}
        self._buffers: Mapping[str, CBuffer] = {}

        self._add_buffers()
        assert_no_error()

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

    def get_component_data(self, component: str) -> Union[np.ndarray, Mapping[str, np.ndarray]]:
        """
        Retrieve Power Grid Model data from the dataset for a specific component.

        Args:
            component: the component.

        Returns:
            The dataset for the specified component.
        """
        return self._data[component]

    def _add_buffers(self):
        for component, buffer_properties in self._component_buffer_properties.items():
            self._add_buffer(component, buffer_properties)

    def _add_buffer(self, component: str, buffer_properties: BufferProperties):
        schema = self._schema[component]

        self._data[component] = create_buffer(buffer_properties, schema)
        self._register_buffer(component, get_buffer_view(self._data[component], schema))

    def _register_buffer(self, component: str, buffer: CBuffer):
        pgc.dataset_writable_set_buffer(
            dataset=self._writable_dataset, component=component, indptr=buffer.indptr, data=buffer.data
        )
        assert_no_error()

    @staticmethod
    def _get_buffer_properties(info: CDatasetInfo) -> Mapping[str, BufferProperties]:
        is_batch = info.is_batch()
        batch_size = info.batch_size()
        components = info.components()
        n_elements_per_scenario = info.elements_per_scenario()
        n_total_elements = info.total_elements()

        return {
            component: BufferProperties(
                is_sparse=n_elements_per_scenario[component] == -1,
                is_batch=is_batch,
                batch_size=batch_size,
                n_elements_per_scenario=n_elements_per_scenario[component],
                n_total_elements=n_total_elements[component],
            )
            for component in components
        }
