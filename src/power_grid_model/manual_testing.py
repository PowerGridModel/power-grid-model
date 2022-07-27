# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
This file contains all the helper functions for testing purpose
"""

import json
from pathlib import Path
from typing import IO, Any, Dict, List, Optional, Union

import numpy as np

from . import initialize_array


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
    return nan_func[data.dtype](data)


def convert_list_to_batch_data(
    list_data: List[Dict[str, np.ndarray]]
) -> Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]:
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

    batch_data = {}
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
            batch_data[component] = {"indptr": np.array(indptr, dtype=np.int32), "data": np.concatenate(data, axis=0)}

    return batch_data


def convert_python_to_numpy(
    data: Union[Dict, List], data_type: str
) -> Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]:
    """
    Convert native python data to internal numpy
    Args:
        data: data in dict or list
        data_type: type of data: input, update, sym_output, or asym_output

    Returns:
        A single or batch dataset for power-grid-model

    """

    # If the inpute data is a list, we are dealing with batch data. Each element in the list is a batch. We'll
    # first convert each batch seperately, by recusively calling this function for each batch. Then the numpy
    # data for all batches in converted into a proper and compact numpy structure.
    if isinstance(data, list):
        list_data = [convert_python_to_numpy(json_dict, data_type=data_type) for json_dict in data]
        return convert_list_to_batch_data(list_data)

    # This should be a normal (non-batch) structure, with a list of objects (dictionaries) per component.
    if not isinstance(data, dict):
        raise TypeError("Only list or dict is allowed in JSON data!")

    dataset: Dict[str, np.ndarray] = {}
    for component, objects in data.items():

        # We'll initialize an 1d-array with NaN values for all the objects of this component type
        dataset[component] = initialize_array(data_type, component, len(objects))

        for i, obj in enumerate(objects):
            # As each object is a separate dictionary, and the properties may differ per object, we need to check
            # all properties. Non-existing properties
            for prop, value in obj.items():
                if prop == "extra":
                    # The "extra" property is a special one. It can store any type of information associated with
                    # an object, but it will not be used in the calculations. Therefore it is not included in the
                    # numpy array, so we can skip this property
                    continue

                if prop not in dataset[component].dtype.names:
                    # If a property doen't exist, the user made a mistake. Let's be merciless in that case,
                    # for their own good.
                    raise ValueError(f"Invalid property '{prop}' for {component} {data_type} data.")

                # Now just assign the value and raise an error if the value cannot be stored in the specific
                # numpy array data format for this property.
                try:
                    dataset[component][i][prop] = value
                except ValueError as ex:
                    raise ValueError(f"Invalid '{prop}' value for {component} {data_type} data: {ex}") from ex
    return dataset


def convert_batch_to_list_data(
    batch_data: Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]
) -> List[Dict[str, np.ndarray]]:
    """
    Convert list of dataset to one single batch dataset
    Args:
        batch_data: a batch dataset for power-grid-model

    Returns:
        list of single dataset
    """

    # If the batch data is empty, return an empty list
    if not batch_data:
        return []

    # Get the data for an arbitrary component; assuming that the number of batches of each component is the same.
    # The structure may differ per component
    example_batch_data = next(iter(batch_data.values()))

    if isinstance(example_batch_data, np.ndarray):
        # We expect the batch data to be a 2d numpy array of n_batches x n_objects
        if len(example_batch_data.shape) != 2:
            raise ValueError("Invalid batch data format")
        n_batches = example_batch_data.shape[0]
    elif isinstance(example_batch_data, dict):
        # If the batch data is a dictionary, we assume that it is an indptr/data structure (otherwise it is an
        # invalid dictionary). There is always one indptr more than there are batches.
        if "indptr" not in example_batch_data:
            raise ValueError("Invalid batch data format")
        n_batches = example_batch_data["indptr"].size - 1
    else:
        # If the batch data is not a numpy array and not a dictionary, it is invalid
        raise ValueError("Invalid batch data format")

    # Initialize an empty list with dictionaries
    # Note that [{}] * n_batches would result in n copies of the same dict.
    list_data = [{} for _ in range(n_batches)]

    # While the number of batches must be the same for each component, the structure (2d numpy array or indptr/data)
    # doesn't have to be. Therefore, we'll check the structure for each component and copy the data accordingly.
    for component, data in batch_data.items():
        if isinstance(data, np.ndarray):
            # For 2d numpy arrays, copy each batch into an element of the list
            for i, batch in enumerate(data):
                list_data[i][component] = batch
        else:
            # For indptr/data structures, use the indptr to select the items for each batch.
            indptr = data["indptr"]
            for i, (idx0, idx1) in enumerate(zip(indptr[:-1], indptr[1:])):
                list_data[i][component] = data["data"][idx0:idx1]
    return list_data


def convert_numpy_to_python(
    data: Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]
) -> Union[Dict[str, List[Dict[str, Union[int, float]]]], List[Dict[str, List[Dict[str, Union[int, float]]]]]]:
    """
    Convert internal numpy arrays to native python data
    If an attribute is not available (NaN value), it will not be exported.
    Args:
        data: A single or batch dataset for power-grid-model
    Returns:
        A json dict for single dataset
        A json list for batch dataset

    """
    # Check if the dataset is a single dataset or batch dataset
    # It is batch dataset if it is 2D array or a indptr/data structure
    example_data = next(iter(data.values()))
    is_dense_batch = isinstance(example_data, np.ndarray) and example_data.ndim == 2
    is_sparse_batch = isinstance(example_data, dict) and "indptr" in example_data and "data" in example_data

    # If it is a batch, convert the batch data to a list of batches, then convert each batch individually.
    if is_dense_batch or is_sparse_batch:
        list_data = convert_batch_to_list_data(data)
        return [convert_numpy_to_python(x) for x in list_data]

    # Otherwise it should be a single data set
    if not isinstance(example_data, np.ndarray) or example_data.ndim != 1:
        raise ValueError("Invalid data format")

    # Convert each numpy array to a list of objects, which contains only the non-NaN properties:
    # For example: {"node": [{"id": 0, ...}, {"id": 1, ...}], "line": [{"id": 2, ...}]}
    return {
        component: [
            {property: obj[property].tolist() for property in objects.dtype.names if not is_nan(obj[property])}
            for obj in objects
        ]
        for component, objects in data.items()
    }


def import_json_data(json_file: Path, data_type: str) -> Union[Dict[str, np.ndarray], List[Dict[str, np.ndarray]]]:
    """
    import json data
    for a list, import individual entry as dictionary of arrays
    Args:
        json_file: path to the json file
        data_type: type of data: input, update, sym_output, or asym_output

    Returns:
         A single or batch dataset for power-grid-model
    """
    with open(json_file, mode="r", encoding="utf-8") as file_pointer:
        json_data = json.load(file_pointer)
    return convert_python_to_numpy(json_data, data_type)


def export_json_data(
    json_file: Path,
    data: Union[Dict[str, np.ndarray], List[Dict[str, np.ndarray]]],
    indent: Optional[int] = 2,
    compact: bool = False,
    extra_info: Optional[Union[Dict[int, Any], List[Dict[int, Any]]]] = None,
):
    """
    export json data
    Args:
        json_file: path to json file
        data: a single or batch dataset for power-grid-model
        indent: indent of the file, default 2
        compact: write components on a single line
        extra_info: extra information (in any json-serializable format), indexed on the object ids
                    e.g. a string representing the original id, or a dictionary storing even more information.

    Returns:
        Save to file
    """
    json_data = convert_numpy_to_python(data)
    if extra_info is not None:
        _inject_extra_info(data=json_data, extra_info=extra_info)

    with open(json_file, mode="w", encoding="utf-8") as file_pointer:
        if compact and indent:
            is_batch_data = isinstance(json_data, list)
            max_level = 4 if is_batch_data else 3
            _compact_json_dump(json_data, file_pointer, indent=indent, max_level=max_level)
        else:
            json.dump(json_data, file_pointer, indent=indent)


def _inject_extra_info(
    data: Union[Dict[str, List[Dict[str, Union[float, int]]]], List[Dict[str, List[Dict[str, Union[float, int]]]]]],
    extra_info: Union[Dict[int, Any], List[Dict[int, Any]]],
):
    """
    Injects extra info to the objects by ID

    Args:
        data: Power Grid Model Python data, as written to pgm json files.
        extra_info: A dictionary indexed by object id. The value may be anything.

    """
    if isinstance(data, list):
        if isinstance(extra_info, list):
            # If both data and extra_info are lists, expect one extra info set per batch
            for batch, info in zip(data, extra_info):
                _inject_extra_info(batch, info)
        else:
            # If only data is a list, copy extra_info for each batch
            for batch in data:
                _inject_extra_info(batch, extra_info)
    elif isinstance(data, dict):
        if not isinstance(extra_info, dict):
            raise TypeError("Invalid extra info data type")
        for _, objects in data.items():
            for obj in objects:
                if obj["id"] in extra_info:
                    obj["extra"] = extra_info[obj["id"]]
    else:
        raise TypeError("Invalid data type")


def _compact_json_dump(data: Any, io_stream: IO[str], indent: int, max_level: int, level: int = 0):
    """Custom compact JSON writer that is intended to put data belonging to a single object on a single line.

    For example:
    {
        "node": [
            {"id": 0, "u_rated": 10500.0, "extra": {"original_id": 123}},
            {"id": 1, "u_rated": 10500.0, "extra": {"original_id": 456}},
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
            _compact_json_dump(obj, io_stream, indent, max_level, level + 1)
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
            _compact_json_dump(obj, io_stream, indent, max_level, level + 2)
        io_stream.write(",\n" if i < n_obj else "\n")
    io_stream.write(tab + "}")
