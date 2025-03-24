# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
This module contains functions that may be useful when working with the power-grid-model library.
"""

import json
import math
import tempfile
import warnings
from pathlib import Path
from typing import cast as cast_type

import numpy as np

from power_grid_model import CalculationMethod, PowerGridModel
from power_grid_model._core.dataset_definitions import DatasetType, _map_to_component_types
from power_grid_model._core.serialization import (  # pylint: disable=unused-import
    json_deserialize,
    json_serialize,
    msgpack_deserialize,
    msgpack_serialize,
)
from power_grid_model._utils import (
    _extract_data_from_component_data,
    _extract_indptr,
    get_and_verify_batch_sizes as _get_and_verify_batch_sizes,
    get_batch_size as _get_batch_size,
    get_dataset_type,
    is_columnar,
    is_sparse,
)
from power_grid_model.data_types import (
    BatchComponentData,
    BatchDataset,
    Dataset,
    DenseBatchArray,
    IndexPointer,
    SingleComponentData,
    SingleDataset,
)
from power_grid_model.errors import PowerGridError, PowerGridSerializationError
from power_grid_model.typing import ComponentAttributeMapping

_DEPRECATED_FUNCTION_MSG = "This function is deprecated."
_DEPRECATED_JSON_DESERIALIZATION_MSG = f"{_DEPRECATED_FUNCTION_MSG} Please use json_deserialize_to_file instead."
_DEPRECATED_JSON_SERIALIZATION_MSG = f"{_DEPRECATED_FUNCTION_MSG} Please use json_serialize_from_file instead."


def get_dataset_scenario(dataset: BatchDataset, scenario: int) -> SingleDataset:
    """
    Obtain the single dataset at a given scenario, independently of the internal batch data structure.

    Args:
        dataset: the batch dataset
        scenario: the scenario index

    Raises:
        IndexError: if the scenario is out of range for any of the components.

    Returns:
        The dataset for a specific scenario
    """

    def _get_dense_scenario(arr: np.ndarray) -> np.ndarray:
        return arr[scenario]

    def _get_sparse_scenario(arr: np.ndarray, indptr: IndexPointer) -> np.ndarray:
        return arr[indptr[scenario] : indptr[scenario + 1]]

    def _get_component_scenario(component_scenarios: BatchComponentData) -> SingleComponentData:
        data = _extract_data_from_component_data(component_scenarios)

        if is_sparse(component_scenarios):
            indptr = _extract_indptr(component_scenarios)
            if is_columnar(component_scenarios):
                return {
                    attribute: _get_sparse_scenario(attribute_data, indptr)
                    for attribute, attribute_data in data.items()
                }
            return _get_sparse_scenario(data, indptr)

        if is_columnar(component_scenarios):
            return {attribute: _get_dense_scenario(attribute_data) for attribute, attribute_data in data.items()}
        return _get_dense_scenario(cast_type(DenseBatchArray, component_scenarios))

    return {component: _get_component_scenario(component_data) for component, component_data in dataset.items()}


def get_dataset_batch_size(dataset: BatchDataset) -> int:
    """
    Get the number of scenarios in the batch dataset.

    Args:
        dataset: the batch dataset

    Raises:
        ValueError: if the batch dataset is inconsistent.

    Returns:
        The size of the batch dataset. Making use of existing _utils function.
    """
    return _get_and_verify_batch_sizes(dataset)


def get_component_batch_size(data_array: BatchComponentData) -> int:
    """
    Determine the number of batch scenarios and verify the data structure

    Args:
        data_array: batch data for power-grid-model

    Returns:
        The number of batch scenarios in data_array
    """
    return _get_batch_size(data_array)


def json_deserialize_from_file(
    file_path: Path,
    data_filter: ComponentAttributeMapping = None,
) -> Dataset:
    """
    Load and deserialize a JSON file to a new dataset.

    Args:
        file_path: the path to the file to load and deserialize.

    Raises:
        ValueError: if the data is inconsistent with the rest of the dataset or a component is unknown.
        PowerGridError: if there was an internal error.

    Returns:
        The deserialized dataset in Power grid model input format.
    """
    with open(file_path, encoding="utf-8") as file_pointer:
        return json_deserialize(file_pointer.read(), data_filter=data_filter)


def json_serialize_to_file(
    file_path: Path,
    data: Dataset,
    dataset_type: DatasetType | None = None,
    use_compact_list: bool = False,
    indent: int | None = 2,
):
    """
    Export JSON data in most recent format.

    Args:
        file_path: the path to the file to load and deserialize.
        data: a single or batch dataset for power-grid-model.
        use_compact_list: write components on a single line.
        indent: indent of the file. Defaults to 2.

    Returns:
        Save to file.
    """
    data = _map_to_component_types(data)
    result = json_serialize(
        data=data, dataset_type=dataset_type, use_compact_list=use_compact_list, indent=-1 if indent is None else indent
    )

    with open(file_path, mode="w", encoding="utf-8") as file_pointer:
        file_pointer.write(result)


def msgpack_deserialize_from_file(
    file_path: Path,
    data_filter: ComponentAttributeMapping = None,
) -> Dataset:
    """
    Load and deserialize a msgpack file to a new dataset.

    Args:
        file_path: the path to the file to load and deserialize.

    Raises:
        ValueError: if the data is inconsistent with the rest of the dataset or a component is unknown.
        PowerGridError: if there was an internal error.

    Returns:
        The deserialized dataset in Power grid model input format.
    """
    with open(file_path, mode="rb") as file_pointer:
        return msgpack_deserialize(file_pointer.read(), data_filter=data_filter)


def msgpack_serialize_to_file(
    file_path: Path, data: Dataset, dataset_type: DatasetType | None = None, use_compact_list: bool = False
):
    """
    Export msgpack data in most recent format.

    Args:
        file_path: the path to the file to load and deserialize.
        data: a single or batch dataset for power-grid-model.
        use_compact_list: write components on a single line.
        indent: indent of the file, default 2.

    Returns:
        Save to file.
    """
    data = _map_to_component_types(data)
    result = msgpack_serialize(data=data, dataset_type=dataset_type, use_compact_list=use_compact_list)

    with open(file_path, mode="wb") as file_pointer:
        file_pointer.write(result)


def import_json_data(json_file: Path, data_type: str, *args, **kwargs) -> Dataset:
    """
    [deprecated] Import json data.

    **WARNING:** This function is deprecated. Please use json_deserialize_from_file instead.

    Args:
        json_file: path to the json file.
        data_type: type of data: input, update, sym_output, or asym_output.
        [deprecated]: All extra positional and keyword arguments are ignored.

    Returns:
        A single or batch dataset for power-grid-model.
    """
    warnings.warn(_DEPRECATED_JSON_DESERIALIZATION_MSG, DeprecationWarning)
    if args:
        warnings.warn("Provided positional arguments at index 2 and following are deprecated.", DeprecationWarning)
    if kwargs:
        warnings.warn(f"Provided keyword arguments {list(kwargs.keys())} are deprecated.", DeprecationWarning)

    return _compatibility_deprecated_import_json_data(json_file=json_file, data_type=data_type)  # type: ignore


def export_json_data(
    json_file: Path, data: Dataset, indent: int | None = 2, compact: bool = False, use_deprecated_format: bool = True
):
    """
    [deprecated] Export json data in a deprecated serialization format.

    **WARNING:** This function is deprecated. Please use json_serialize_to_file instead.

    For backwards compatibility, this function outputs the deprecated serialization format by default.
    This feature may be removed in the future.

    Args:
        json_file: path to json file.
        data: a single or batch dataset for power-grid-model.
        indent: indent of the file, default 2.
        compact: write components on a single line.
        use_deprecated_format: use the old style format. Defaults to True for backwards compatibility.

    Returns:
        Save to file.
    """
    warnings.warn(_DEPRECATED_JSON_SERIALIZATION_MSG, DeprecationWarning)
    if use_deprecated_format:
        warnings.warn(
            "Argument use_deprecated_format is a temporary backwards-compatibility measure. "
            "Please upgrade to use_deprecated_format=False or json_serialize_to_file as soon as possible.",
            DeprecationWarning,
            stacklevel=2,
        )
        _compatibility_deprecated_export_json_data(json_file=json_file, data=data)
    else:
        json_serialize_to_file(file_path=json_file, data=data, use_compact_list=compact, indent=indent)


def _compatibility_deprecated_export_json_data(
    json_file: Path, data: Dataset, indent: int | None = 2, compact: bool = False
):
    serialized_data = json_serialize(data=data, use_compact_list=compact, indent=-1 if indent is None else indent)
    old_format_serialized_data = json.dumps(json.loads(serialized_data)["data"])
    with open(json_file, mode="w", encoding="utf-8") as file_pointer:
        file_pointer.write(old_format_serialized_data)


def import_input_data(json_file: Path) -> SingleDataset:
    """
    [deprecated] Import input json data.

    **WARNING:** This function is deprecated. Please use json_deserialize_from_file instead.

    For backwards and forward compatibility, this function supportes both the latest and the old serialization format.

    Args:
        json_file: path to the json file.

    Returns:
        A single dataset for power-grid-model.
    """
    warnings.warn(_DEPRECATED_JSON_DESERIALIZATION_MSG, DeprecationWarning)

    data = _compatibility_deprecated_import_json_data(json_file=json_file, data_type=DatasetType.input)
    assert isinstance(data, dict)
    assert all(isinstance(component, np.ndarray) and component.ndim == 1 for component in data.values())
    return cast_type(SingleDataset, data)


def import_update_data(json_file: Path) -> BatchDataset:
    """
    [deprecated] Import update json data.

    **WARNING:** This function is deprecated. Please use json_deserialize_from_file instead.

    For backwards and forward compatibility, this function supportes both the latest and the old serialization format.

    Args:
        json_file: path to the json file.

    Returns:
        A batch dataset for power-grid-model.
    """
    warnings.warn(_DEPRECATED_JSON_DESERIALIZATION_MSG, DeprecationWarning)

    return cast_type(
        BatchDataset,
        _compatibility_deprecated_import_json_data(json_file=json_file, data_type=DatasetType.update),
    )


def _compatibility_deprecated_import_json_data(json_file: Path, data_type: DatasetType):
    with open(json_file, mode="r", encoding="utf-8") as file_pointer:
        data = json.load(file_pointer)

    if "version" not in data:  # convert old format to version 1.0
        data = {
            "attributes": {},
            "data": data,
            "is_batch": isinstance(data, list),
            "type": data_type,
            "version": "1.0",
        }

    result = json_deserialize(json.dumps(data))
    if get_dataset_type(result) != data_type:
        raise PowerGridSerializationError("An internal error occured during deserialization")

    return result


def self_test():
    """
    Perform a self-test of the Power Grid Model functionality.

    Tests whether the installation was successful and there are no build errors,
    segmentation violations, undefined symbols, etc.

    This function is designed to validate the basic functionality of data serialization,
    model instantiation, power flow calculation, and serialization of results using the
    Power Grid Model library.

    Raises:
        PowerGridError: if there was an internal error.
    """
    with tempfile.TemporaryDirectory() as temp_dir:
        # Create a simple JSON input data file in the temporary directory
        input_data = {
            "version": "1.0",
            "type": "input",
            "is_batch": False,
            "attributes": {},
            "data": {
                "node": [{"id": 1, "u_rated": 10000}],
                "source": [{"id": 2, "node": 1, "u_ref": 1, "sk": 1e20}],
                "sym_load": [{"id": 3, "node": 1, "status": 1, "type": 0, "p_specified": 0, "q_specified": 0}],
            },
        }

        input_file_path = Path(temp_dir) / "input_data.json"
        input_file_path.write_text(json.dumps(input_data))

        try:
            # Load the created JSON input data file (deserialize)
            deserialized_data = json_deserialize_from_file(input_file_path)

            # Create a PowerGridModel instance from the loaded input data
            model = PowerGridModel(deserialized_data)

            # Run a simple power flow calculation on the created model (linear calculation)
            output_data = model.calculate_power_flow(calculation_method=CalculationMethod.linear)

            # Write the calculation result to a file in the temporary directory
            output_file_path = Path(temp_dir) / "output_data.json"

            json_serialize_to_file(output_file_path, output_data)

            # Verify that the written output is correct
            with open(output_file_path, "r", encoding="utf-8") as output_file:
                output_data = json.load(output_file)

            assert output_data is not None
            assert math.isclose(
                output_data["data"]["node"][0]["u"], input_data["data"]["node"][0]["u_rated"], abs_tol=1e-9
            )

            print("Self test finished.")
        except Exception as e:
            raise PowerGridError from e
