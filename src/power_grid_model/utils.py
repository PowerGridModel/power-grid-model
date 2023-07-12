# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
This file contains all the helper functions for testing purpose
"""

import json
from pathlib import Path
from typing import IO, Any, List, Optional, cast

import numpy as np

from power_grid_model import initialize_array
from power_grid_model.data_types import (
    BatchArray,
    BatchDataset,
    BatchList,
    ComponentList,
    Dataset,
    PythonDataset,
    SingleDataset,
    SinglePythonDataset,
    SparseBatchArray,
)


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


def convert_list_to_batch_data(list_data: BatchList) -> BatchDataset:
    """
    Convert a list of datasets to one single batch dataset

    Example data formats:
        input:  [{"node": <1d-array>, "line": <1d-array>}, {"node": <1d-array>, "line": <1d-array>}]
        output: {"node": <2d-array>, "line": <2d-array>}
         -or-:  {"indptr": <1d-array>, "data": <1d-array>}
    Args:
        list_data: list of dataset

    Returns:
        batch dataset
        For a certain component, if all the length is the same for all the batches, a 2D array is used
        Otherwise use a dict of indptr/data key
    """

    # List all *unique* types
    components = {x for dataset in list_data for x in dataset.keys()}

    batch_data: BatchDataset = {}
    for component in components:
        # Create a 2D array if the component exists in all datasets and number of objects is the same in each dataset
        comp_exists_in_all_datasets = all(component in x for x in list_data)
        if comp_exists_in_all_datasets:
            all_sizes_are_the_same = all(x[component].size == list_data[0][component].size for x in list_data)
            if all_sizes_are_the_same:
                batch_data[component] = np.stack([x[component] for x in list_data], axis=0)
                continue

        # otherwise use indptr/data dict
        indptr = [0]
        data = []
        for dataset in list_data:
            if component in dataset:
                # If the current dataset contains the component, increase the indptr for this batch and append the data
                objects = dataset[component]
                indptr.append(indptr[-1] + len(objects))
                data.append(objects)

            else:
                # If the current dataset does not contain the component, add the last indptr again.
                indptr.append(indptr[-1])

            # Convert the index pointers to a numpy array and combine the list of object numpy arrays into a singe
            # numpy array. All objects of all batches are now stores in one large array, the index pointers define
            # which elemets of the array (rows) belong to which batch.
            batch_data[component] = {"indptr": np.array(indptr, dtype=np.int64), "data": np.concatenate(data, axis=0)}

    return batch_data


def convert_python_to_numpy(data: PythonDataset, data_type: str, ignore_extra: bool = False) -> Dataset:
    """
    Convert native python data to internal numpy

    Args:
        data: data in dict or list
        data_type: type of data: input, update, sym_output, or asym_output
        ignore_extra: Allow (and ignore) extra attributes in the data

    Returns:
        A single or batch dataset for power-grid-model

    """

    # If the input data is a list, we are dealing with batch data. Each element in the list is a batch. We'll
    # first convert each batch separately, by recursively calling this function for each batch. Then the numpy
    # data for all batches in converted into a proper and compact numpy structure.
    if isinstance(data, list):
        list_data = [
            convert_python_single_dataset_to_single_dataset(json_dict, data_type=data_type, ignore_extra=ignore_extra)
            for json_dict in data
        ]
        return convert_list_to_batch_data(list_data)

    # Otherwise this should be a normal (non-batch) structure, with a list of objects (dictionaries) per component.
    if not isinstance(data, dict):
        raise TypeError("Data should be either a list or a dictionary!")

    return convert_python_single_dataset_to_single_dataset(data=data, data_type=data_type, ignore_extra=ignore_extra)


def convert_python_single_dataset_to_single_dataset(
    data: SinglePythonDataset, data_type: str, ignore_extra: bool = False
) -> SingleDataset:
    """
    Convert native python data to internal numpy

    Args:
        data: data in dict
        data_type: type of data: input, update, sym_output, or asym_output
        ignore_extra: Allow (and ignore) extra attributes in the data

    Returns:
        A single dataset for power-grid-model

    """

    dataset: SingleDataset = {}
    for component, objects in data.items():
        dataset[component] = convert_component_list_to_numpy(
            objects=objects, component=component, data_type=data_type, ignore_extra=ignore_extra
        )

    return dataset


def convert_component_list_to_numpy(
    objects: ComponentList, component: str, data_type: str, ignore_extra: bool = False
) -> np.ndarray:
    """
    Convert native python data to internal numpy

    Args:
        objects: data in dict
        component: the name of the component
        data_type: type of data: input, update, sym_output, or asym_output
        ignore_extra: Allow (and ignore) extra attributes in the data

    Returns:
        A single numpy array

    """

    # We'll initialize an 1d-array with NaN values for all the objects of this component type
    array = initialize_array(data_type, component, len(objects))

    for i, obj in enumerate(objects):
        # As each object is a separate dictionary, and the attributes may differ per object, we need to check
        # all attributes. Non-existing attributes
        for attribute, value in obj.items():
            # If an attribute doesn't exist, the user should explicitly state that she/he is ok with extra
            # information in the data. This is to protect the user from overlooking errors.
            if attribute not in array.dtype.names:
                if ignore_extra:
                    continue
                raise ValueError(
                    f"Invalid attribute '{attribute}' for {component} {data_type} data. "
                    "(Use ignore_extra=True to ignore the extra data and suppress this exception)"
                )

            # Assign the value and raise an error if the value cannot be stored in the specific numpy array data format
            # for this attribute.
            try:
                array[i][attribute] = value
            except ValueError as ex:
                raise ValueError(f"Invalid '{attribute}' value for {component} {data_type} data: {ex}") from ex
    return array


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
    checked_components: List[str] = []
    for component, data in batch_data.items():
        n_component_batch_size = get_batch_size(data)
        if checked_components and n_component_batch_size != n_batch_size:
            if len(checked_components) == 1:
                checked_components_str = f"'{checked_components.pop()}'"
            else:
                checked_components_str = "/".join(sorted(checked_components))
            raise ValueError(
                f"Inconsistent number of batches in batch data. "
                f"Component '{component}' contains {n_component_batch_size} batches, "
                f"while {checked_components_str} contained {n_batch_size} batches."
            )
        n_batch_size = n_component_batch_size
        checked_components.append(component)
    return n_batch_size


def get_batch_size(batch_data: BatchArray) -> int:
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
        n_batches = batch_data.shape[0]
    elif isinstance(batch_data, dict):
        # If the batch data is a dictionary, we assume that it is an indptr/data structure (otherwise it is an
        # invalid dictionary). There is always one indptr more than there are batches.
        if "indptr" not in batch_data:
            raise ValueError("Invalid batch data format, expected 'indptr' and 'data' entries")
        n_batches = batch_data["indptr"].size - 1
    else:
        # If the batch data is not a numpy array and not a dictionary, it is invalid
        raise ValueError(
            "Invalid batch data format, expected a 2-d numpy array or a dictionary with an 'indptr' and 'data' entry"
        )
    return n_batches


def split_numpy_array_in_batches(data: np.ndarray, component: str) -> List[np.ndarray]:
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


def split_sparse_batches_in_batches(batch_data: SparseBatchArray, component: str) -> List[np.ndarray]:
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

    if not isinstance(data, np.ndarray) or data.ndim != 1:
        raise TypeError(
            f"Invalid data type {type(data).__name__} in sparse batch data for '{component}' "
            "(should be a 1D Numpy structured array (i.e. a single 'table'))."
        )

    if not isinstance(indptr, np.ndarray) or indptr.ndim != 1 or not np.issubdtype(indptr.dtype, np.integer):
        raise TypeError(
            f"Invalid indptr data type {type(indptr).__name__} in batch data for '{component}' "
            "(should be a 1D Numpy array (i.e. a single 'list'), "
            "containing indices (i.e. integers))."
        )

    if indptr[0] != 0 or indptr[-1] != len(data) or any(indptr[i] > indptr[i + 1] for i in range(len(indptr) - 1)):
        raise TypeError(
            f"Invalid indptr in batch data for '{component}' "
            f"(should start with 0, end with the number of objects ({len(data)}) "
            "and be monotonic increasing)."
        )

    return [data[indptr[i] : indptr[i + 1]] for i in range(len(indptr) - 1)]


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


def import_json_data(json_file: Path, data_type: str, ignore_extra: bool = False) -> Dataset:
    """
    import json data

    Args:
        json_file: path to the json file
        data_type: type of data: input, update, sym_output, or asym_output
        ignore_extra: Allow (and ignore) extra attributes in the json file

    Returns:
        A single or batch dataset for power-grid-model
    """
    with open(json_file, mode="r", encoding="utf-8") as file_pointer:
        data = json.load(file_pointer)
    return convert_python_to_numpy(data=data, data_type=data_type, ignore_extra=ignore_extra)


def import_input_data(json_file: Path) -> SingleDataset:
    """
    import input json data

    Args:
        json_file: path to the json file

    Returns:
         A single dataset for power-grid-model
    """
    data = import_json_data(json_file=json_file, data_type="input")
    assert isinstance(data, dict)
    assert all(isinstance(component, np.ndarray) and component.ndim == 1 for component in data.values())
    return cast(SingleDataset, data)


def import_update_data(json_file: Path) -> BatchDataset:
    """
    import update json data

    Args:
        json_file: path to the json file

    Returns:
         A batch dataset for power-grid-model
    """
    return cast(BatchDataset, import_json_data(json_file=json_file, data_type="update"))


def export_json_data(json_file: Path, data: Dataset, indent: Optional[int] = 2, compact: bool = False):
    """
    export json data

    Args:
        json_file: path to json file
        data: a single or batch dataset for power-grid-model
        indent: indent of the file, default 2
        compact: write components on a single line

    Returns:
        Save to file
    """
    json_data = convert_dataset_to_python_dataset(data)

    with open(json_file, mode="w", encoding="utf-8") as file_pointer:
        if compact and indent:
            is_batch_data = isinstance(json_data, list)
            max_level = 4 if is_batch_data else 3
            compact_json_dump(json_data, file_pointer, indent=indent, max_level=max_level)
        else:
            json.dump(json_data, file_pointer, indent=indent)


def compact_json_dump(data: Any, io_stream: IO[str], indent: int, max_level: int, level: int = 0):
    """Custom compact JSON writer that is intended to put data belonging to a single object on a single line.

    For example::
    {
        "node": [
            {"id": 0, "u_rated": 10500.0},
            {"id": 1, "u_rated": 10500.0},
        ],
        "line": [
            {"id": 2, "node_from": 0, "node_to": 1, ...}
        ]
    }

    The function is being called recursively, starting at level 0 and recursing until max_level is reached. It is
    basically a full json writer, but for efficiency reasons, on the last levels the native json.dump method is used.
    """

    # Let's define a 'tab' indent, depending on the level
    tab = " " * level * indent

    # If we are at the max_level, or the data simply doesn't contain any more levels, write the indent and serialize
    # the data on a single line.
    if level >= max_level or not isinstance(data, (list, dict)):
        io_stream.write(tab)
        json.dump(data, io_stream, indent=None)
        return

    # We'll need the number of objects later on
    n_obj = len(data)

    # If the data is a list:
    # 1. start with an opening bracket
    # 2. dump each element in the list
    # 3. add a comma and a new line after each element, except for the last element, there we don't need a comma.
    # 4. finish with a closing bracket
    if isinstance(data, list):
        io_stream.write(tab + "[\n")
        for i, obj in enumerate(data, start=1):
            compact_json_dump(obj, io_stream, indent, max_level, level + 1)
            io_stream.write(",\n" if i < n_obj else "\n")
        io_stream.write(tab + "]")
        return

    # If the data is a dictionary:
    # 1. start with an opening curly bracket
    # 2. for each element: write it's key, plus a colon ':'
    # 3. if the next level would be the max_level, add a space and dump the element on a single,
    #    else add a new line before dumping the element recursively.
    # 4. add a comma and a new line after each element, except for the last element, there we don't need a comma.
    # 5. finish with a closing curly bracket
    io_stream.write(tab + "{\n")
    for i, (key, obj) in enumerate(data.items(), start=1):
        io_stream.write(tab + " " * indent + f'"{key}":')
        if level == max_level - 1 or not isinstance(obj, (list, dict)):
            io_stream.write(" ")
            json.dump(obj, io_stream, indent=None)
        else:
            io_stream.write("\n")
            compact_json_dump(obj, io_stream, indent, max_level, level + 2)
        io_stream.write(",\n" if i < n_obj else "\n")
    io_stream.write(tab + "}\n")
