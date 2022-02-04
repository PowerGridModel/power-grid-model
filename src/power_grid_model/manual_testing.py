# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
This file contains all the helper functions for testing purpose
"""

import json
from pathlib import Path
from typing import Dict, List, Union

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
    Convert list of dataset to one single batch dataset
    Args:
        list_data: list of dataset

    Returns:
        batch dataset
        For a certain component, if all the length is the same for all the batches, a 2D array is used
        Otherwise use a dict of indptr/data key
    """

    # List all *unique* types
    all_types = list({x for single_batch in list_data for x in single_batch.keys()})

    batch_data = {}
    for comp_type in all_types:
        # use 2D array if the type exists in all single dataset and the size is the same
        if np.all([comp_type in x for x in list_data]) and np.unique([x[comp_type].size for x in list_data]).size == 1:
            batch_data[comp_type] = np.stack([x[comp_type] for x in list_data], axis=0)
            continue
        # otherwise use indptr/data dict
        indptr = [0]
        data = []
        for single_batch in list_data:
            if comp_type not in single_batch:
                indptr.append(indptr[-1])
            else:
                single_data = single_batch[comp_type]
                indptr.append(indptr[-1] + single_data.shape[0])
                data.append(single_data)
        batch_data[comp_type] = {"indptr": np.array(indptr, dtype=np.int32), "data": np.concatenate(data, axis=0)}
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
    list_data = []
    # return empty list
    if not batch_data:
        return list_data
    # get n_batch
    one_data = next(iter(batch_data.values()))
    if isinstance(one_data, dict):
        n_batch = one_data["indptr"].size - 1
    else:
        n_batch = one_data.shape[0]
    # convert
    for i in range(n_batch):
        single_dataset = {}
        for key, batch in batch_data.items():
            if isinstance(batch, dict):
                single_dataset[key] = batch["data"][batch["indptr"][i] : batch["indptr"][i + 1]]
            else:
                single_dataset[key] = batch[i, ...]
        list_data.append(single_dataset)
    return list_data


def convert_numpy_to_python(data: Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]) -> Union[Dict, List]:
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


def export_json_data(json_file: Path, data: Union[Dict[str, np.ndarray], List[Dict[str, np.ndarray]]], indent=2):
    """
    export json data
    Args:
        json_file: path to json file
        data: A single or batch dataset for power-grid-model
        indent:
            indent of the file, default 2

    Returns:
        Save to file
    """
    json_data = convert_numpy_to_python(data)
    with open(json_file, mode="w", encoding="utf-8") as file_pointer:
        json.dump(json_data, file_pointer, indent=indent)
