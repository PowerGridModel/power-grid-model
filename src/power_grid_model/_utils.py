# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
This file contains helper functions for library-internal use only.

Disclaimer!

We do not officially support this functionality and may remove features in this library at any given time!
"""

from copy import deepcopy
from typing import Optional, Sequence, cast

import numpy as np

from power_grid_model.core.dataset_definitions import ComponentType, DatasetType
from power_grid_model.core.power_grid_meta import initialize_array, power_grid_meta_data
from power_grid_model.data_types import (
    BatchColumn,
    BatchComponentData,
    BatchDataset,
    BatchList,
    ColumnarData,
    ComponentData,
    DataArray,
    Dataset,
    DenseBatchArray,
    DenseBatchColumnarData,
    DenseBatchData,
    IndexPointer,
    PythonDataset,
    SingleArray,
    SingleColumn,
    SingleColumnarData,
    SingleComponentData,
    SingleDataset,
    SinglePythonDataset,
    SparseBatchData,
)
from power_grid_model.enum import ComponentAttributeFilterOptions
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
        component_data_checks(data, component)
        component_batches: Sequence[SingleComponentData]
        if is_sparse(data):
            component_batches = split_sparse_batch_data_in_batches(cast(SparseBatchData, data), component)
        else:
            component_batches = split_dense_batch_data_in_batches(cast(SingleComponentData, data), component)
        for i, batch in enumerate(component_batches):
            if (isinstance(batch, dict) and batch) or (isinstance(batch, np.ndarray) and batch.size > 0):
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
    Determine the number of batches and verify the data structure while we're at it. Note only batch data is supported.
    Note: SingleColumnarData would get treated as batch by this function.

    Args:
        batch_data: a batch array for power-grid-model

    Raises:
        ValueError: when the type for data_filter is incorrect

    Returns:
        The number of batches
    """
    component_data_checks(batch_data)
    if is_sparse(batch_data):
        indptr = batch_data["indptr"]
        return indptr.size - 1

    data_to_check = (
        next(iter(cast(DenseBatchColumnarData, batch_data).values()))
        if is_columnar(batch_data)
        else cast(DenseBatchArray, batch_data)
    )
    if data_to_check.ndim == 1:
        return 1
    if data_to_check.ndim == 2 and is_columnar(batch_data):
        raise ValueError("get_batch_size is not supported for columnar data")
    return data_to_check.shape[0]


def _split_numpy_array_in_batches(
    data: DenseBatchArray | SingleArray | SingleColumn | BatchColumn,
) -> list[SingleArray] | list[SingleColumn]:
    """
    Split a single dense numpy array into one or more batches

    Args:
        data: A 1D or 2D Numpy structured array. A 1D array is a single table / batch, a 2D array is a batch per table.
        component: The name of the component to which the data belongs; only used for errors.
        attribute [optional]: The name of the attribute to which the data belongs; only used for errors.

    Returns:
        A list with a single numpy structured array per batch
    """
    if data.ndim == 1:
        return [data]
    if data.ndim in [2, 3]:
        return [data[i, ...] for i in range(data.shape[0])]
    raise ValueError("Dimension of the component data is invalid.")


def split_dense_batch_data_in_batches(
    data: SingleComponentData | DenseBatchData, component: ComponentType
) -> list[SingleComponentData]:
    """
    Split a single dense numpy array into one or more batches

    Args:
        data: A 1D or 2D Numpy structured array. A 1D array is a single table / batch, a 2D array is a batch per table.
        component: The name of the component to which the data belongs, only used for errors.

    Returns:
        A list with a single component data per scenario
    """
    component_data_checks(data, component)
    if isinstance(data, np.ndarray):
        return cast(list[SingleComponentData], _split_numpy_array_in_batches(data))

    batch_size = get_batch_size(data)
    scenarios_per_attribute = {
        attribute: _split_numpy_array_in_batches(attribute_data) for attribute, attribute_data in data.items()
    }

    return [
        {attribute: scenarios_per_attribute[attribute][scenario] for attribute, attribute_data in data.items()}
        for scenario in range(batch_size)
    ]


def split_sparse_batch_data_in_batches(
    batch_data: SparseBatchData, component: ComponentType
) -> list[SingleComponentData]:
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
            return {attribute: _split_buffer(attribute_data, scenario) for attribute, attribute_data in data.items()}
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


def convert_single_dataset_to_python_single_dataset(
    data: SingleDataset,
) -> SinglePythonDataset:
    """
    Convert internal numpy arrays to native python data
    If an attribute is not available (NaN value), it will not be exported.

    Args:
        data: A single dataset for power-grid-model

    Returns:
        A python dict for single dataset
    """

    # Convert each numpy array to a list of objects, which contains only the non-NaN attributes:
    # For example: {"node": [{"id": 0, ...}, {"id": 1, ...}], "line": [{"id": 2, ...}]}
    def _convert_component(objects: SingleComponentData):
        # This should be a single data set
        if not isinstance(objects, np.ndarray) or objects.ndim != 1:
            raise ValueError("Invalid data format")

        return [
            {attribute: obj[attribute].tolist() for attribute in objects.dtype.names if not is_nan(obj[attribute])}
            for obj in objects
        ]

    return {component: _convert_component(objects) for component, objects in data.items()}


def compatibility_convert_row_columnar_dataset(
    data: Dataset,
    data_filter: ComponentAttributeMapping,
    dataset_type: DatasetType,
    available_components: list[ComponentType] | None = None,
) -> Dataset:
    """Temporary function to transform row, column or mixed based datasets to a full row or column based dataset as per
    the data_filter. The purpose of this function is to mimic columnar data and transform back to row data without any
    memory footprint benefits.
    Note: Copies are made on a per-component basis; if a component is row based in both the input and the requested
    output, that componened is returned without a copy.

    Args:
        data (Dataset): dataset to convert
        data_filter (ComponentAttributeMapping): desired component and attribute mapping
        dataset_type (DatasetType): type of dataset (e.g., input, update or [sym | asym | sc]_output)
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
                data=data[comp_name]["data"],
                comp_name=comp_name,
                dataset_type=dataset_type,
                attrs=attrs,
            )
            result_data[comp_name]["indptr"] = data[comp_name]["indptr"]
        else:
            result_data[comp_name] = _convert_data_to_row_or_columnar(
                data=data[comp_name],
                comp_name=comp_name,
                dataset_type=dataset_type,
                attrs=attrs,
            )
    return result_data


def _convert_data_to_row_or_columnar(
    data: SingleComponentData,
    comp_name: ComponentType,
    dataset_type: DatasetType,
    attrs: set[str] | list[str] | None | ComponentAttributeFilterOptions,
) -> SingleComponentData:
    """Converts row or columnar component data to row or columnar component data as requested in `attrs`."""
    if attrs is None:
        if not is_columnar(data):
            return data
        data = cast(SingleColumnarData, data)
        output_array = initialize_array(dataset_type, comp_name, next(iter(data.values())).shape)
        for k in data:
            output_array[k] = data[k]
        return output_array

    if isinstance(attrs, (list, set)) and len(attrs) == 0:
        return {}
    if isinstance(attrs, ComponentAttributeFilterOptions):
        names = cast(SingleArray, data).dtype.names if not is_columnar(data) else cast(SingleColumnarData, data).keys()
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
    elif isinstance(data_filter, ComponentAttributeFilterOptions):
        processed_data_filter = {ComponentType[k]: data_filter for k in available_components}
    elif isinstance(data_filter, (list, set)):
        processed_data_filter = {ComponentType[k]: None for k in data_filter}
    elif isinstance(data_filter, dict) and all(
        attrs is None or isinstance(attrs, (set, list, ComponentAttributeFilterOptions))
        for attrs in data_filter.values()
    ):
        processed_data_filter = data_filter
    else:
        raise ValueError(f"Invalid filter provided: {data_filter}")

    validate_data_filter(processed_data_filter, dataset_type, available_components)
    return processed_data_filter


def validate_data_filter(
    data_filter: _ComponentAttributeMappingDict,
    dataset_type: DatasetType,
    available_components: list[ComponentType],
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

    for source, components in {
        "data_filter": data_filter.keys(),
        "data": available_components,
    }.items():
        unknown_components = [x for x in components if x not in dataset_meta]
        if unknown_components:
            raise KeyError(f"The following specified component types are unknown:{unknown_components} in {source}")

    unknown_attributes = {}
    for comp_name, attrs in data_filter.items():
        if attrs is None or isinstance(attrs, ComponentAttributeFilterOptions):
            continue

        attr_names = dataset_meta[comp_name].dtype.names
        diff = set(cast(set[str] | list[str], attrs))
        if attr_names is not None:
            diff = diff.difference(attr_names)
        if diff != set():
            unknown_attributes[comp_name] = diff

    if unknown_attributes:
        raise KeyError(f"The following specified attributes are unknown: {unknown_attributes} in data_filter")


def is_sparse(component_data: ComponentData) -> bool:
    """Check if component_data is sparse or dense. Only batch data can be sparse."""
    return isinstance(component_data, dict) and set(component_data.keys()) == {
        "indptr",
        "data",
    }


def is_columnar(component_data: ComponentData) -> bool:
    """Check if component_data is columnar or row based"""
    if is_sparse(component_data):
        return not isinstance(component_data["data"], np.ndarray)
    return not isinstance(component_data, np.ndarray)


def is_nan_or_equivalent(array):
    """
    Check if the array contains only nan values or equivalent nan values for specific data types.

    Args:
        array: The array to check.

    Returns:
        bool: True if the array contains only nan or equivalent nan values, False otherwise.
    """
    return isinstance(array, np.ndarray) and (
        (array.dtype == np.float64 and np.isnan(array).all())
        or (array.dtype in (np.int32, np.int8) and np.all(array == np.iinfo(array.dtype).min))
    )


def _check_sparse_dense(component_data: ComponentData, err_msg_suffixed: str) -> ComponentData:
    if is_sparse(component_data):
        indptr = component_data["indptr"]
        if not isinstance(indptr, np.ndarray):
            raise TypeError(err_msg_suffixed.format(f"Invalid indptr type {type(indptr).__name__}. "))
        sub_data = component_data["data"]
    elif isinstance(component_data, dict) and ("indptr" in component_data or "data" in component_data):
        missing_element = "indptr" if "indptr" not in component_data else "data"
        raise KeyError(err_msg_suffixed.format(f"Missing '{missing_element}' in sparse batch data. "))
    else:
        sub_data = component_data
    return sub_data


def _check_columnar_row(sub_data: ComponentData, err_msg_suffixed: str) -> None:
    if is_columnar(sub_data):
        if not isinstance(sub_data, dict):
            raise TypeError(err_msg_suffixed.format(""))
        for attribute, attribute_array in sub_data.items():
            if not isinstance(attribute_array, np.ndarray):
                raise TypeError(err_msg_suffixed.format(f"'{attribute}' attribute. "))
            if attribute_array.ndim not in [1, 2, 3]:
                raise TypeError(err_msg_suffixed.format(f"Invalid dimension: {attribute_array.ndim}"))
    elif not isinstance(sub_data, np.ndarray):
        raise TypeError(err_msg_suffixed.format(f"Invalid data type {type(sub_data).__name__} "))
    elif isinstance(sub_data, np.ndarray) and sub_data.ndim not in [1, 2]:
        raise TypeError(err_msg_suffixed.format(f"Invalid dimension: {sub_data.ndim}. "))


def component_data_checks(component_data: ComponentData, component=None) -> None:
    """Checks if component_data is of ComponentData and raises ValueError if its not"""
    component_name = f"'{component}'" if component is not None else ""
    err_msg = f"Invalid data for {component_name} component. " "{0}"
    err_msg_suffixed = err_msg + "Expecting a 1D/2D Numpy structured array or a dictionary of such."

    sub_data = _check_sparse_dense(component_data, err_msg_suffixed)
    _check_columnar_row(sub_data, err_msg_suffixed)


def _extract_indptr(data: ComponentData) -> IndexPointer:  # pragma: no cover
    """returns indptr and checks if its valid

    Args:
        data (ComponentData): The component data

    Raises:
        TypeError: if indptr is invalid or is not available

    Returns:
        IndexPointer: indptr if present
    """
    if not is_sparse(data):
        raise TypeError("Not sparse data")
    indptr = data["indptr"]
    if not isinstance(indptr, np.ndarray):
        raise TypeError("indptr is not a 1D numpy array")
    if indptr.ndim != 1:
        raise TypeError("indptr is not a 1D numpy array")
    return indptr


def _extract_columnar_data(data: ComponentData, is_batch: bool | None = None) -> ColumnarData:  # pragma: no cover
    """returns the contents of the columnar data.

    Args:
        data (ComponentData): component data
        is_batch (bool | None, optional): If given data is batch. Skips batch check if provided None.

    Raises:
        TypeError: if data is not columnar or invalid data

    Returns:
        ColumnarData: the contents of columnar data
    """
    not_columnar_data_message = "Expected columnar data"

    if is_batch is not None:
        allowed_dims = [2, 3] if is_batch else [1, 2]
    else:
        allowed_dims = [1, 2, 3]

    sub_data = data["data"] if is_sparse(data) else data

    if not isinstance(sub_data, dict):
        raise TypeError(not_columnar_data_message)
    for attribute, attribute_array in sub_data.items():
        if not isinstance(attribute_array, np.ndarray) or not isinstance(attribute, str):
            raise TypeError(not_columnar_data_message)
        if attribute_array.ndim not in allowed_dims:
            raise TypeError(not_columnar_data_message)
    return cast(ColumnarData, sub_data)


def _extract_row_based_data(data: ComponentData, is_batch: bool | None = None) -> DataArray:  # pragma: no cover
    """returns the contents of the row based data

    Args:
        data (ComponentData): component data
        is_batch (bool | None, optional): If given data is batch. Skips batch check if provided None.

    Raises:
        TypeError: if data is not row based or invalid data

    Returns:
        DataArray: the contents of row based data
    """
    if is_batch is not None:
        allowed_dims = [2] if is_batch else [1]
    else:
        allowed_dims = [1, 2]

    sub_data = data["data"] if is_sparse(data) else data

    if not isinstance(sub_data, np.ndarray):
        raise TypeError("Expected row based data")
    if sub_data.ndim not in allowed_dims:
        raise TypeError("Expected row based data")
    return sub_data


def _extract_data_from_component_data(data: ComponentData, is_batch: bool | None = None):
    return _extract_columnar_data(data, is_batch) if is_columnar(data) else _extract_row_based_data(data, is_batch)
