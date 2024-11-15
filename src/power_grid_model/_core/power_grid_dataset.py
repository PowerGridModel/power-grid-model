# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power grid model raw dataset handler
"""

from typing import Any, Mapping

from power_grid_model._core.buffer_handling import (
    BufferProperties,
    CAttributeBuffer,
    CBuffer,
    create_buffer,
    get_buffer_properties,
    get_buffer_view,
)
from power_grid_model._core.dataset_definitions import ComponentType, DatasetType, _str_to_component_type
from power_grid_model._core.error_handling import VALIDATOR_MSG, assert_no_error
from power_grid_model._core.power_grid_core import (
    ConstDatasetPtr,
    DatasetInfoPtr,
    MutableDatasetPtr,
    WritableDatasetPtr,
    power_grid_core as pgc,
)
from power_grid_model._core.power_grid_meta import ComponentMetaData, DatasetMetaData, power_grid_meta_data
from power_grid_model._utils import get_dataset_type, is_columnar, is_nan_or_equivalent, process_data_filter
from power_grid_model.data_types import AttributeType, ComponentData, Dataset
from power_grid_model.enum import ComponentAttributeFilterOptions
from power_grid_model.typing import ComponentAttributeMapping, _ComponentAttributeMappingDict


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

    def components(self) -> list[ComponentType]:
        """
        The components in the dataset.

        Returns:
            A list of the component names in the dataset
        """
        return [
            _str_to_component_type(pgc.dataset_info_component_name(self._info, idx))
            for idx in range(self.n_components())
        ]

    def elements_per_scenario(self) -> Mapping[ComponentType, int]:
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

    def total_elements(self) -> Mapping[ComponentType, int]:
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


class CMutableDataset:
    """
    A view of a user-owned dataset.

    This may be used to provide a user dataset to the Power Grid Model.

    The dataset will create mutable buffers that the Power Grid Model can use to load data.
    """

    _dataset_type: DatasetType
    _schema: DatasetMetaData
    _is_batch: bool
    _batch_size: int
    _mutable_dataset: MutableDatasetPtr
    _buffer_views: list[CBuffer]

    def __new__(cls, data: Dataset, dataset_type: Any = None):
        instance = super().__new__(cls)
        instance._mutable_dataset = MutableDatasetPtr()
        instance._buffer_views = []

        instance._dataset_type = dataset_type if dataset_type in DatasetType else get_dataset_type(data)
        instance._schema = power_grid_meta_data[instance._dataset_type]

        if data:
            first_component, first_component_data = next(iter(data.items()))
            first_sub_info = get_buffer_properties(data=first_component_data, schema=instance._schema[first_component])
            instance._is_batch = first_sub_info.is_batch
            instance._batch_size = first_sub_info.batch_size
        else:
            instance._is_batch = False
            instance._batch_size = 1

        instance._mutable_dataset = pgc.create_dataset_mutable(
            instance._dataset_type.value, instance._is_batch, instance._batch_size
        )
        assert_no_error()

        instance._add_data(data)
        assert_no_error()

        return instance

    def get_dataset_ptr(self) -> MutableDatasetPtr:
        """
        Get the raw underlying mutable dataset pointer.

        Returns:
            MutableDatasetPtr: the raw underlying mutable dataset pointer.
        """
        return self._mutable_dataset

    def get_info(self) -> CDatasetInfo:
        """
        Get the info for this dataset.

        Returns:
            The dataset info for this dataset.
        """
        return CDatasetInfo(pgc.dataset_mutable_get_info(self._mutable_dataset))

    def get_buffer_views(self) -> list[CBuffer]:
        """
        Get list of buffer views

        Returns:
            list of buffer view
        """
        return self._buffer_views

    def _add_data(self, data: Dataset):
        """
        Add Power Grid Model data to the mutable dataset view.

        Args:
            data: the data.

        Raises:
            ValueError: if the component is unknown and allow_unknown is False.
            ValueError: if the data is inconsistent with the rest of the dataset.
            PowerGridError: if there was an internal error.
        """
        for component, component_data in data.items():
            self._add_component_data(component, component_data, allow_unknown=False)

    def _add_component_data(self, component: ComponentType, data: ComponentData, allow_unknown: bool = False):
        """
        Add Power Grid Model data for a single component to the mutable dataset view.

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

        self._validate_properties(data, self._schema[component])
        c_buffer = get_buffer_view(data, self._schema[component], self._is_batch, self._batch_size)
        self._buffer_views.append(c_buffer)
        self._register_buffer(component, c_buffer)

    def _register_buffer(self, component: ComponentType, buffer: CBuffer):
        pgc.dataset_mutable_add_buffer(
            dataset=self._mutable_dataset,
            component=component.value,
            elements_per_scenario=buffer.n_elements_per_scenario,
            total_elements=buffer.total_elements,
            indptr=buffer.indptr,
            data=buffer.data,
        )
        assert_no_error()
        for attr, attr_data in buffer.attribute_data.items():
            self._register_attribute_buffer(component, attr, attr_data)

    def _register_attribute_buffer(self, component, attr, attr_data):
        pgc.dataset_mutable_add_attribute_buffer(
            dataset=self._mutable_dataset,
            component=component.value,
            attribute=attr,
            data=attr_data.data,
        )
        assert_no_error()

    def _validate_properties(self, data: ComponentData, schema: ComponentMetaData):
        properties = get_buffer_properties(data, schema=schema, is_batch=self._is_batch, batch_size=self._batch_size)
        if properties.is_batch != self._is_batch:
            raise ValueError(
                f"Dataset type (single or batch) must be consistent across all components. {VALIDATOR_MSG}"
            )
        if properties.batch_size != self._batch_size:
            raise ValueError(f"Dataset must have a consistent batch size across all components. {VALIDATOR_MSG}")

    def __del__(self):
        pgc.destroy_dataset_mutable(self._mutable_dataset)


class CConstDataset:
    """
    A view of a user-owned dataset.

    This may be used to provide a user dataset to the Power Grid Model.

    The dataset will create const buffers that the Power Grid Model can use to load data.

    It is created from mutable dataset.
    """

    _const_dataset: ConstDatasetPtr
    _buffer_views: list[CBuffer]

    def __new__(cls, data: Dataset, dataset_type: DatasetType | None = None):
        instance = super().__new__(cls)
        instance._const_dataset = ConstDatasetPtr()

        # create from mutable dataset
        mutable_dataset = CMutableDataset(data=data, dataset_type=dataset_type)
        instance._const_dataset = pgc.create_dataset_const_from_mutable(mutable_dataset.get_dataset_ptr())
        assert_no_error()
        instance._buffer_views = mutable_dataset.get_buffer_views()

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

    def __del__(self):
        pgc.destroy_dataset_const(self._const_dataset)


class CWritableDataset:
    """
    A view of a Power Grid Model-owned dataset.

    This may be used to retrieve data from the Power Grid Model.

    This class provides buffers to which the Power Grid Model can write data in an external call.
    After writing to the buffers, the data contents can be retrieved.
    """

    def __init__(self, dataset_ptr: WritableDatasetPtr, data_filter: ComponentAttributeMapping):
        self._writable_dataset = dataset_ptr

        info = self.get_info()
        self._dataset_type = info.dataset_type()
        self._schema = power_grid_meta_data[self._dataset_type]

        self._data_filter = process_data_filter(
            dataset_type=info.dataset_type(),
            data_filter=data_filter,
            available_components=info.components(),
        )

        self._data: Dataset = {}
        self._buffers: Mapping[str, CBuffer] = {}
        self._component_buffer_properties = self._get_buffer_properties(info)

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

    def get_data(self) -> Dataset:
        """
        Retrieve data from the Power Grid Model dataset.

        The Power Grid Model may write to these buffers at a later point in time.

        Returns:
            The full dataset with filters applied.
        """
        self._post_filtering()
        return self._data

    def get_component_data(self, component: ComponentType) -> ComponentData:
        """
        Retrieve Power Grid Model data from the dataset for a specific component.

        Args:
            component: the component.

        Returns:
            The dataset for the specified component.
        """
        return self._data[component]

    def get_data_filter(self) -> _ComponentAttributeMappingDict:
        """Gets the data filter requested

        Returns:
            _ComponentAttributeMappingDict: data filter
        """
        return self._data_filter

    def _add_buffers(self):
        for component, buffer_properties in self._component_buffer_properties.items():
            self._add_buffer(component, buffer_properties)

    def _add_buffer(self, component: ComponentType, buffer_properties: BufferProperties):
        schema = self._schema[component]

        self._data[component] = create_buffer(buffer_properties, schema)
        self._register_buffer(component, get_buffer_view(self._data[component], schema))

    def _register_buffer(self, component: ComponentType, buffer: CBuffer):
        pgc.dataset_writable_set_buffer(
            dataset=self._writable_dataset,
            component=component,
            indptr=buffer.indptr,
            data=buffer.data,
        )
        assert_no_error()
        for attribute, attribute_data in buffer.attribute_data.items():
            self._register_attribute_buffer(component, attribute, attribute_data)

    def _register_attribute_buffer(
        self,
        component: ComponentType,
        attribute: AttributeType,
        buffer: CAttributeBuffer,
    ):
        pgc.dataset_writable_set_attribute_buffer(
            dataset=self._writable_dataset,
            component=component,
            attribute=attribute,
            data=buffer.data,
        )
        assert_no_error()

    def _get_buffer_properties(self, info: CDatasetInfo) -> Mapping[ComponentType, BufferProperties]:
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
                columns=_get_filtered_attributes(
                    schema=self._schema[component],
                    component_data_filter=self._data_filter[component],
                ),
            )
            for component in components
            if component in self._data_filter
        }

    def _filter_attributes(self, attributes):
        keys_to_remove = []
        for attr, array in attributes.items():
            if is_columnar(array):
                continue
            if is_nan_or_equivalent(array):
                keys_to_remove.append(attr)
        for key in keys_to_remove:
            del attributes[key]

    def _filter_with_mapping(self):
        for component_type, attributes in self._data.items():
            if component_type in self._data_filter:
                filter_option = self._data_filter[component_type]
                if filter_option is ComponentAttributeFilterOptions.relevant:
                    self._filter_attributes(attributes)

    def _post_filtering(self):
        if isinstance(self._data_filter, dict):
            self._filter_with_mapping()


def _get_filtered_attributes(
    schema: ComponentMetaData,
    component_data_filter: set[str] | list[str] | None | ComponentAttributeFilterOptions,
) -> list[str] | None:
    if component_data_filter is None:
        return None

    if isinstance(component_data_filter, ComponentAttributeFilterOptions):
        return [] if schema.dtype.names is None else list(schema.dtype.names)

    return list(component_data_filter)
