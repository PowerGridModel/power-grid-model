# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Data handling
"""


from typing import Mapping

import numpy as np

from power_grid_model.core.dataset_definitions import ComponentType, DatasetType
from power_grid_model.core.power_grid_dataset import CConstDataset, CMutableDataset
from power_grid_model.core.power_grid_meta import initialize_array, power_grid_meta_data
from power_grid_model.data_types import Dataset
from power_grid_model.enum import CalculationType
from power_grid_model.typing import ComponentAttributeMapping, _ComponentAttributeMappingDict


def get_output_type(*, calculation_type: CalculationType, symmetric: bool) -> DatasetType:
    """
    Get the output type based on the provided arguments.

    Args:
        calculation_type:
            request the output type for a specific calculation type (power_flow, state_estimation, ...)
        symmetric:
            True: three-phase symmetric calculation, even for asymmetric loads/generations
            False: three-phase asymmetric calculation

    Returns:
        the output type that fits the format requested by the output type
    """
    if calculation_type in (CalculationType.power_flow, CalculationType.state_estimation):
        return DatasetType.sym_output if symmetric else DatasetType.asym_output

    if calculation_type == CalculationType.short_circuit:
        return DatasetType.sc_output

    raise NotImplementedError()


def prepare_input_view(input_data: Mapping[ComponentType, np.ndarray]) -> CConstDataset:
    """
    Create a view of the input data in a format compatible with the PGM core libary.

    Args:
        input_data:
            the input data to create the view from

    Returns:
        instance of CConstDataset ready to be fed into C API
    """
    return CConstDataset(input_data, dataset_type=DatasetType.input)


def prepare_update_view(update_data: Mapping[ComponentType, np.ndarray | Mapping[str, np.ndarray]]) -> CConstDataset:
    """
    Create a view of the update data, or an empty view if not provided, in a format compatible with the PGM core libary.

    Args:
        update_data:
            the update data to create the view from. Defaults to None

    Returns:
        instance of CConstDataset ready to be fed into C API
    """
    return CConstDataset(update_data, dataset_type=DatasetType.update)


def prepare_output_view(output_data: Mapping[ComponentType, np.ndarray], output_type: DatasetType) -> CMutableDataset:
    """
    create a view of the output data in a format compatible with the PGM core libary.

    Args:
        output_data:
            the output data to create the view from
        output_type:
            the output type of the output_data

    Returns:
        instance of CMutableDataset ready to be fed into C API
    """
    return CMutableDataset(output_data, dataset_type=output_type)


def create_output_data(
    output_component_types: ComponentAttributeMapping,
    output_type: DatasetType,
    all_component_count: dict[ComponentType, int],
    is_batch: bool,
    batch_size: int,
) -> Dataset:
    """
    Create the output dataset based on component and batch size from the model; and output attributes requested by user.

    Args:
        output_component_types:
            the output components the user seeks to extract
        output_type:
            the type of output that the user will see (as per the calculation options)
        all_component_count:
            the amount of components in the grid (as per the input data)
        is_batch:
            if the dataset is batch
        batch_size:
            the batch size

    Returns:
        Dataset: output dataset
    """
    processed_output_types = process_data_filter(output_type, output_component_types, list(all_component_count.keys()))

    all_component_count = {k: v for k, v in all_component_count.items() if k in processed_output_types}

    # create result dataset
    result_dict = {}

    for name, count in all_component_count.items():
        # shape
        if is_batch:
            shape: tuple[int] | tuple[int, int] = (batch_size, count)
        else:
            shape = (count,)
        result_dict[name] = initialize_array(output_type, name, shape=shape, empty=True)
    return result_dict


def process_data_filter(
    dataset_type: DatasetType,
    data_filter: ComponentAttributeMapping,
    available_components: list[ComponentType],
) -> _ComponentAttributeMappingDict:
    """Checks valid type for a mapping of componenet to attributes.
    The default for `None` is row_based format while Ellipsis (...) is columnar format.
    Also checks for any invalid component names and attribute names

    Args:
        dataset_type (DatasetType): the type of output that the user will see (as per the calculation options)
        data_filter (ComponentAttributeMapping):  component to attribute mapping or list provided by user
        available_components (list[ComponentType]):  all components available in model instance

    Returns:
        _ComponentAttributeMappingDict: processed component to attribute mapping
    """
    # limit all component count to user specified component types in output and convert to a dict
    if data_filter is None:
        data_filter = {k: None for k in available_components}
    elif data_filter is Ellipsis:
        data_filter = {k: Ellipsis for k in available_components}
    elif isinstance(data_filter, (list, set)):
        data_filter = {k: None for k in data_filter}
    elif not isinstance(data_filter, dict) or not all(
        attrs is None or attrs is Ellipsis or isinstance(attrs, (set, list)) for attrs in data_filter.values()
    ):
        raise ValueError(f"Invalid filter provided: {data_filter}")

    validate_data_filter(data_filter, dataset_type)

    return data_filter


def validate_data_filter(data_filter: _ComponentAttributeMappingDict, dataset_type: DatasetType) -> None:
    """Raise error if some specified components or attributes are unknown

    Args:
        data_filter (_ComponentAttributeMappingDict): Component to attribtue mapping
        dataset_type (DatasetType):  Type of dataset

    Raises:
        ValueError: when the type for output_comoponent_types is incorrect
        KeyError: with "unknown component" for any unknown components
        KeyError: with "unknown attributes" for any unknown attributes for a known component
    """
    dataset_meta = power_grid_meta_data[dataset_type]
    unknown_components = [x for x in data_filter if x not in dataset_meta]
    if unknown_components:
        raise KeyError(f"You have specified some unknown component types: {unknown_components}")

    unknown_attributes = {}
    for comp_name, attrs in data_filter.items():
        if attrs is None or attrs is Ellipsis:
            continue
        diff = set(attrs).difference(dataset_meta[comp_name].dtype.names)
        if diff != set():
            unknown_attributes[comp_name] = diff

    if unknown_attributes:
        raise KeyError(f"You have specified some unknown attributes: {unknown_attributes}")
