# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
This file contains helper functions for library-internal use only.

Disclaimer!

We do not officially support this functionality and may remove features in this library at any given time!
"""

from copy import deepcopy
from types import EllipsisType
from typing import Optional, cast

import numpy as np

from power_grid_model.core.dataset_definitions import ComponentType, DatasetType
from power_grid_model.core.power_grid_meta import initialize_array, power_grid_meta_data
from power_grid_model.data_types import (
    BatchComponentData,
    BatchDataset,
    BatchList,
    ComponentData,
    Dataset,
    DenseBatchArray,
    PythonDataset,
    SingleArray,
    SingleComponentData,
    SingleDataset,
    SinglePythonDataset,
    SparseBatchData,
)
from power_grid_model.typing import ComponentAttributeMapping, _ComponentAttributeMappingDict


def is_nan(data) -> bool:
    """
    Determine if the data point is valid
    Args:
        data: a single scaler or numpy array

    Returns:
        True if all the data points are invalid
        False otherwise
    """
    nan_func = {
        np.dtype("f8"): lambda x: np.all(np.isnan(x)),
        np.dtype("i4"): lambda x: np.all(x == np.iinfo("i4").min),
        np.dtype("i1"): lambda x: np.all(x == np.iinfo("i1").min),
    }
    return bool(nan_func[data.dtype](data))


def convert_batch_dataset_to_batch_list(batch_data: BatchDataset) -> BatchList:
    """
    Convert batch datasets to a list of individual batches

    Args:
        batch_data: a batch dataset for power-grid-model

    Returns:
        A list of individual batches
    """

    # If the batch data is empty, return an empty list
    if len(batch_data) == 0:
        return []

    n_batches = get_and_verify_batch_sizes(batch_data=batch_data)

    # Initialize an empty list with dictionaries
    # Note that [{}] * n_batches would result in n copies of the same dict.
    list_data: BatchList = [{} for _ in range(n_batches)]

    # While the number of batches must be the same for each component, the structure (2d numpy array or indptr/data)
    # doesn't have to be. Therefore, we'll check the structure for each component and copy the data accordingly.
    for component, data in batch_data.items():
        if isinstance(data, np.ndarray):
            component_batches = split_numpy_array_in_batches(data, component)
        elif isinstance(data, dict):
            component_batches = split_sparse_batches_in_batches(data, component)
        else:
            raise TypeError(
                f"Invalid data type {type(data).__name__} in batch data for '{component}' "
                "(should be a Numpy structured array or a python dictionary)."
            )
        for i, batch in enumerate(component_batches):
            if batch.size > 0:
                list_data[i][component] = batch
    return list_data


def get_and_verify_batch_sizes(batch_data: BatchDataset) -> int:
    """
    Determine the number of batches for each component and verify that each component has the same number of batches

    Args:
        batch_data: a batch dataset for power-grid-model

    Returns:
        The number of batches
    """

    n_batch_size = 0
    checked_components: list[ComponentType] = []
    for component, data in batch_data.items():
        n_component_batch_size = get_batch_size(data)
        if checked_components and n_component_batch_size != n_batch_size:
            if len(checked_components) == 1:
                checked_components_str = f"'{checked_components.pop()}'"
            else:
                str_checked_components = [str(component) for component in checked_components]
                checked_components_str = "/".join(sorted(str_checked_components))
            raise ValueError(
                f"Inconsistent number of batches in batch data. "
                f"Component '{component}' contains {n_component_batch_size} batches, "
                f"while {checked_components_str} contained {n_batch_size} batches."
            )
        n_batch_size = n_component_batch_size
        checked_components.append(component)
    return n_batch_size


def get_batch_size(batch_data: BatchComponentData) -> int:
    """
    Determine the number of batches and verify the data structure while we're at it.

    Args:
        batch_data: a batch array for power-grid-model

    Returns:
        The number of batches
    """
    if isinstance(batch_data, np.ndarray):
        # We expect the batch data to be a 2d numpy array of n_batches x n_objects. If it is a 1d numpy array instead,
        # we assume that it is a single batch.
        if batch_data.ndim == 1:
            return 1
        return batch_data.shape[0]

    if isinstance(batch_data, dict):
        # If the batch data is a dictionary, we assume that it is an indptr/data structure (otherwise it is an
        # invalid dictionary). There is always one indptr more than there are batches.
        if "indptr" not in batch_data:
            raise ValueError("Invalid batch data format, expected 'indptr' and 'data' entries")
        indptr = batch_data["indptr"]
        if isinstance(indptr, np.ndarray):
            return indptr.size - 1

    # If the batch data is not a numpy array and not a dictionary, it is invalid
    raise ValueError(
        "Invalid batch data format, expected a 2-d numpy array or a dictionary with an 'indptr' and 'data' entry"
    )


def split_numpy_array_in_batches(data: DenseBatchArray | SingleArray, component: ComponentType) -> list[np.ndarray]:
    """
    Split a single dense numpy array into one or more batches

    Args:
        data: A 1D or 2D Numpy structured array. A 1D array is a single table / batch, a 2D array is a batch per table.
        component: The name of the component to which the data belongs, only used for errors.

    Returns:
        A list with a single numpy structured array per batch

    """
    if not isinstance(data, np.ndarray):
        raise TypeError(
            f"Invalid data type {type(data).__name__} in batch data for '{component}' "
            "(should be a 1D/2D Numpy structured array)."
        )
    if data.ndim == 1:
        return [data]
    if data.ndim == 2:
        return [data[i, :] for i in range(data.shape[0])]
    raise TypeError(
        f"Invalid data dimension {data.ndim} in batch data for '{component}' "
        "(should be a 1D/2D Numpy structured array)."
    )


def split_sparse_batches_in_batches(batch_data: SparseBatchData, component: ComponentType) -> list[SingleComponentData]:
    """
    Split a single numpy array representing, a compressed sparse structure, into one or more batches

    Args:
        batch_data: Sparse batch data
        component: The name of the component to which the data belongs, only used for errors.

    Returns:
        A list with a single numpy structured array per batch
    """

    for key in ["indptr", "data"]:
        if key not in batch_data:
            raise KeyError(
                f"Missing '{key}' in sparse batch data for '{component}' "
                "(expected a python dictionary containing two keys: 'indptr' and 'data')."
            )

    data = batch_data["data"]
    indptr = batch_data["indptr"]

    def _split_buffer(buffer: np.ndarray, scenario: int) -> SingleArray:
        if not isinstance(buffer, np.ndarray) or buffer.ndim != 1:
            raise TypeError(
                f"Invalid data type {type(buffer).__name__} in sparse batch data for '{component}' "
                "(should be a 1D Numpy structured array (i.e. a single 'table'))."
            )

        if not isinstance(indptr, np.ndarray) or indptr.ndim != 1 or not np.issubdtype(indptr.dtype, np.integer):
            raise TypeError(
                f"Invalid indptr data type {type(indptr).__name__} in batch data for '{component}' "
                "(should be a 1D Numpy array (i.e. a single 'list'), "
                "containing indices (i.e. integers))."
            )

        if indptr[0] != 0 or indptr[-1] != len(buffer) or indptr[scenario] > indptr[scenario + 1]:
            raise TypeError(
                f"Invalid indptr in batch data for '{component}' "
                f"(should start with 0, end with the number of objects ({len(buffer)}) "
                "and be monotonic increasing)."
            )

        return buffer[indptr[scenario] : indptr[scenario + 1]]

    def _get_scenario(scenario: int) -> SingleComponentData:
        if isinstance(data, dict):
            # return {attribute: _split_buffer(attribute_data, scenario) for attribute, attribute_data in data.items()}
            raise NotImplementedError()  # TODO(mgovers): uncomment when columnar data support is added
        return _split_buffer(data, scenario)

    return [_get_scenario(i) for i in range(len(indptr) - 1)]


def convert_dataset_to_python_dataset(data: Dataset) -> PythonDataset:
    """
    Convert internal numpy arrays to native python data
      If an attribute is not available (NaN value), it will not be exported.

    Args:
        data: A single or batch dataset for power-grid-model
    Returns:
        A python dict for single dataset
        A python list for batch dataset
    """

    # Check if the dataset is a single dataset or batch dataset
    # It is batch dataset if it is 2D array or a indptr/data structure
    is_batch: Optional[bool] = None
    for component, array in data.items():
        is_dense_batch = isinstance(array, np.ndarray) and array.ndim == 2
        is_sparse_batch = isinstance(array, dict) and "indptr" in array and "data" in array
        if is_batch is not None and is_batch != (is_dense_batch or is_sparse_batch):
            raise ValueError(
                f"Mixed {'' if is_batch else 'non-'}batch data "
                f"with {'non-' if is_batch else ''}batch data ({component})."
            )
        is_batch = is_dense_batch or is_sparse_batch

    # If it is a batch, convert the batch data to a list of batches, then convert each batch individually.
    if is_batch:
        # We have established that this is batch data, so let's tell the type checker that this is a BatchDataset
        data = cast(BatchDataset, data)
        list_data = convert_batch_dataset_to_batch_list(data)
        return [convert_single_dataset_to_python_single_dataset(data=x) for x in list_data]

    # We have established that this is not batch data, so let's tell the type checker that this is a BatchDataset
    data = cast(SingleDataset, data)
    return convert_single_dataset_to_python_single_dataset(data=data)


def convert_single_dataset_to_python_single_dataset(data: SingleDataset) -> SinglePythonDataset:
    """
    Convert internal numpy arrays to native python data
    If an attribute is not available (NaN value), it will not be exported.

    Args:
        data: A single dataset for power-grid-model

    Returns:
        A python dict for single dataset
    """

    # This should be a single data set
    for component, array in data.items():
        if not isinstance(array, np.ndarray) or array.ndim != 1:
            raise ValueError("Invalid data format")

    # Convert each numpy array to a list of objects, which contains only the non-NaN attributes:
    # For example: {"node": [{"id": 0, ...}, {"id": 1, ...}], "line": [{"id": 2, ...}]}
    return {
        component: [
            {attribute: obj[attribute].tolist() for attribute in objects.dtype.names if not is_nan(obj[attribute])}
            for obj in objects
        ]
        for component, objects in data.items()
    }


def compatibility_convert_row_columnar_dataset(
    data: Dataset,
    data_filter: ComponentAttributeMapping,
    dataset_type: DatasetType,
    available_components: list[ComponentType] | None = None,
) -> Dataset:
    """Temporary function to copy row based dataset to a column based dataset as per the data_filter.
    The purpose of this function is to mimic columnar data without any memory footprint benefits.
    Note: If both the input and requested output are row based, the same dataset is returned without a copy.

    Args:
        data (Dataset):
        component_types (_ComponentAttributeMappingDict):

    Returns:
        Dataset: converted dataset
    Args:
        data (Dataset): dataset to convert
        data_filter (ComponentAttributeMapping): desired component and attribute mapping
        dataset_type (DatasetType): type of dataset
        available_components (list[ComponentType] | None): available components in model

    Returns:
        Dataset: converted dataset
    """
    if available_components is None:
        available_components = list(data.keys())

    processed_data_filter = process_data_filter(dataset_type, data_filter, available_components)

    result_data: Dataset = {}
    for comp_name, attrs in processed_data_filter.items():
        if comp_name not in data:
            continue
        if is_sparse(data[comp_name]):
            result_data[comp_name] = {}
            result_data[comp_name]["data"] = _convert_data_to_row_or_columnar(
                data=data[comp_name]["data"], comp_name=comp_name, dataset_type=dataset_type, attrs=attrs
            )
            result_data[comp_name]["indptr"] = data[comp_name]["indptr"]
        else:
            result_data[comp_name] = _convert_data_to_row_or_columnar(
                data=data[comp_name], comp_name=comp_name, dataset_type=dataset_type, attrs=attrs
            )
    return result_data


def _convert_data_to_row_or_columnar(
    data: SingleComponentData,
    comp_name: ComponentType,
    dataset_type: DatasetType,
    attrs: set[str] | list[str] | None | EllipsisType,
) -> SingleComponentData:
    """Converts row or columnar component data to row or columnar component data as requested in `attrs`."""
    if attrs is None:
        if not is_columnar(data):
            return data
        output_array = initialize_array(dataset_type, comp_name, next(iter(data.values())).shape)
        for k in data:
            output_array[k] = data[k]
        return output_array
    if isinstance(attrs, (list, set)) and len(attrs) == 0:
        return {}
    if isinstance(attrs, EllipsisType):
        names = data.dtype.names if not is_columnar(data) else data.keys()
        return {attr: deepcopy(data[attr]) for attr in names}
    return {attr: deepcopy(data[attr]) for attr in attrs}


def process_data_filter(
    dataset_type: DatasetType,
    data_filter: ComponentAttributeMapping,
    available_components: list[ComponentType],
) -> _ComponentAttributeMappingDict:
    """Checks valid type for data_filter. Also checks for any invalid component names and attribute names.

    Args:
        dataset_type (DatasetType): the type of output that the user will see (as per the calculation options)
        data_filter (ComponentAttributeMapping):  data_filter provided by user
        available_components (list[ComponentType]):  all components available in model instance or data

    Returns:
        _ComponentAttributeMappingDict: processed data_filter in a dictionary
    """
    if data_filter is None:
        processed_data_filter: _ComponentAttributeMappingDict = {ComponentType[k]: None for k in available_components}
    elif data_filter is Ellipsis:
        processed_data_filter = {ComponentType[k]: ... for k in available_components}
    elif isinstance(data_filter, (list, set)):
        processed_data_filter = {ComponentType[k]: None for k in data_filter}
    elif isinstance(data_filter, dict) and all(
        attrs is None or attrs is Ellipsis or isinstance(attrs, (set, list)) for attrs in data_filter.values()
    ):
        processed_data_filter = data_filter
    else:
        raise ValueError(f"Invalid filter provided: {data_filter}")

    validate_data_filter(processed_data_filter, dataset_type, available_components)
    return processed_data_filter


def validate_data_filter(
    data_filter: _ComponentAttributeMappingDict, dataset_type: DatasetType, available_components: list[ComponentType]
) -> None:
    """Raise error if some specified components or attributes are unknown.

    Args:
        data_filter (_ComponentAttributeMappingDict): Processed component to attribtue dictionary
        dataset_type (DatasetType):  Type of dataset
        available_components (list[ComponentType]):  all components available in model instance or data

    Raises:
        ValueError: when the type for data_filter is incorrect
        KeyError: with "unknown component types" for any unknown components
        KeyError: with "unknown attributes" for unknown attribute(s) for a known component
    """
    dataset_meta = power_grid_meta_data[dataset_type]

    for source, components in {"data_filter": data_filter.keys(), "data": available_components}.items():
        unknown_components = [x for x in components if x not in dataset_meta]
        if unknown_components:
            raise KeyError(f"The following specified component types are unknown:{unknown_components} in {source}")

    unknown_attributes = {}
    for comp_name, attrs in data_filter.items():
        if attrs is None or attrs is Ellipsis:
            continue
        attr_names = dataset_meta[comp_name].dtype.names
        diff = set(attrs).difference(attr_names) if attr_names is not None else set(attrs)
        if diff != set():
            unknown_attributes[comp_name] = diff

    if unknown_attributes:
        raise KeyError(f"The following specified attributes are unknown: {unknown_attributes} in data_filter")


def is_sparse(component_data: ComponentData) -> bool:
    """Check if component_data is sparse or dense. Only batch data can be sparse."""
    return isinstance(component_data, dict) and set(component_data.keys()) == {"indptr", "data"}


def is_columnar(component_data: ComponentData) -> bool:
    """Check if component_data is columnar or row based"""
    if is_sparse(component_data):
        return not isinstance(component_data["data"], np.ndarray)
    return not isinstance(component_data, np.ndarray)
