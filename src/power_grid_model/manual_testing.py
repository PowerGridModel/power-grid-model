# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
This file contains all the helper functions for testing purpose
"""

import json
from pathlib import Path
from typing import Any, Dict, List, IO, Optional, Union

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
        datasets: List[Dict[str, np.ndarray]]
) -> Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]:
    """
    Convert a list of datasets to one single batch dataset

    Example data formats:
        input:  [{"node": <1d-array>, "line": <1d-array>}, {"node": <1d-array>, "line": <1d-array>}]
        output: {"node": <2d-array>, "line": <2d-array>}
         -or-:  {"indptr": <1d-array>, "data": <1d-array>}
    Args:
        datasets: list of dataset

    Returns:
        batch dataset
        For a certain component, if all the length is the same for all the batches, a 2D array is used
        Otherwise use a dict of indptr/data key
    """

    # List all *unique* types
    components = {x for dataset in datasets for x in dataset.keys()}

    batch_data = {}
    for component in components:

        # Create a 2D array if the component exists in all datasets and number of objects is the same in each dataset
        comp_exists_in_all_datasets = all(component in x for x in datasets)
        all_sizes_are_the_same = lambda: all(x[component].size == datasets[0][component].size for x in datasets)
        if comp_exists_in_all_datasets and all_sizes_are_the_same():
            batch_data[component] = np.stack([x[component] for x in datasets], axis=0)
            continue

        # otherwise use indptr/data dict
        indptr = [0]
        data = []
        for dataset in datasets:

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
    if isinstance(data, dict):
        return_dict = {}
        for component_name, component_list in data.items():
            arr: np.ndarray = initialize_array(data_type, component_name, len(component_list))
            for i, component in enumerate(component_list):
                for property_name, value in component.items():
                    if property_name == "extra":
                        continue
                    if property_name not in arr[i].dtype.names:
                        raise ValueError(f"Invalid property '{property_name}' for {component_name} {data_type} data.")
                    try:
                        arr[i][property_name] = value
                    except ValueError as ex:
                        raise ValueError(f"Invalid '{property_name}' value for {component_name} {data_type} data: {ex}")

            return_dict[component_name] = arr
        return return_dict

    if isinstance(data, list):
        list_data = [convert_python_to_numpy(json_dict, data_type=data_type) for json_dict in data]
        return convert_list_to_batch_data(list_data)

    raise TypeError("Only list or dict is allowed in JSON data!")


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
            # For indptr/data structures,
            for i, (idx0, idx1) in enumerate(zip(data["indptr"][:-1], data["indptr"][1:])):
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
    # check the dataset is single or batch
    if data:
        one_data = next(iter(data.values()))
        # it is batch dataset if it is 2D array of a dict of indptr/data
        if isinstance(one_data, dict) or one_data.ndim == 2:
            list_data = convert_batch_to_list_data(data)
            return [convert_numpy_to_python(x) for x in list_data]
    # otherwise it is single dataset
    single_dataset: Dict[str, np.ndarray] = data
    return {
        name: [{k: item[k].tolist() for k in array.dtype.names if not is_nan(item[k])} for item in array]
        for name, array in single_dataset.items()
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
        extra_info: Optional[Dict[int, Any]] = None,
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

    # Inject extra info
    if extra_info is not None:
        for component, objects in json_data.items():
            for obj in objects:
                if obj["id"] in extra_info:
                    obj["extra"] = extra_info[obj["id"]]

    with open(json_file, mode="w", encoding="utf-8") as file_pointer:
        if compact and indent:
            max_level = 4 if isinstance(json_data, list) else 3
            compact_json_dump(json_data, file_pointer, indent=indent, max_level=max_level)
        else:
            json.dump(json_data, file_pointer, indent=indent)


def compact_json_dump(data: Any, io_stream: IO[str], indent: int, max_level: int, level: int = 0):
    tab = " " * level * indent
    if level >= max_level:
        io_stream.write(tab)
        json.dump(data, io_stream, indent=None)
    elif isinstance(data, list):
        io_stream.write(tab + "[\n")
        n_obj = len(data)
        for i, obj in enumerate(data, start=1):
            compact_json_dump(obj, io_stream, indent, max_level, level + 1)
            io_stream.write(",\n" if i < n_obj else "\n")
        io_stream.write(tab + "]")
    elif isinstance(data, dict):
        io_stream.write(tab + "{\n")
        n_obj = len(data)
        for i, (key, obj) in enumerate(data.items(), start=1):
            if level == max_level - 1 or not isinstance(obj, (list, dict)):
                io_stream.write(tab + " " * indent + f'"{key}": ')
                json.dump(obj, io_stream, indent=None)
            else:
                io_stream.write(tab + " " * indent + f'"{key}":\n')
                compact_json_dump(obj, io_stream, indent, max_level, level + 2)
            io_stream.write(",\n" if i < n_obj else "\n")
        io_stream.write(tab + "}")
    else:
        io_stream.write(tab)
        json.dump(data, io_stream, indent=None)
