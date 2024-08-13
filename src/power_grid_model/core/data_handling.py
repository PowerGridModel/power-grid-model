# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Data handling
"""


from enum import Enum
from typing import Mapping

import numpy as np

from power_grid_model.core.dataset_definitions import ComponentType, DatasetType
from power_grid_model.core.power_grid_dataset import CConstDataset, CMutableDataset
from power_grid_model.core.power_grid_meta import initialize_array, power_grid_meta_data
from power_grid_model.data_types import Dataset
from power_grid_model.enum import CalculationType
from power_grid_model.typing import ComponentAttributeMapping, _ComponentAttributeMappingDict


class OutputType(Enum):
    """
    The different supported output types:
        - DatasetType.sym_output
        - DatasetType.asym_output
        - DatasetType.sc_output
    """

    SYM_OUTPUT = DatasetType.sym_output
    ASYM_OUTPUT = DatasetType.asym_output
    SC_OUTPUT = DatasetType.sc_output


def get_output_type(*, calculation_type: CalculationType, symmetric: bool) -> OutputType:
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
        return OutputType.SYM_OUTPUT if symmetric else OutputType.ASYM_OUTPUT

    if calculation_type == CalculationType.short_circuit:
        return OutputType.SC_OUTPUT

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


def prepare_output_view(output_data: Mapping[ComponentType, np.ndarray], output_type: OutputType) -> CMutableDataset:
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
    return CMutableDataset(output_data, dataset_type=output_type.value)


def create_output_data(
    output_component_types: ComponentAttributeMapping,
    output_type: OutputType,
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
    processed_output_types = process_output_component_types(
        output_type, output_component_types, list(all_component_count.keys())
    )

    all_component_count = {k: v for k, v in all_component_count.items() if k in processed_output_types}

    # create result dataset
    result_dict = {}

    for name, count in all_component_count.items():
        # shape
        if is_batch:
            shape: tuple[int] | tuple[int, int] = (batch_size, count)
        else:
            shape = (count,)
        result_dict[name] = initialize_array(output_type.value, name, shape=shape, empty=True)
    return result_dict


def process_output_component_types(
    output_type: OutputType,
    output_component_types: ComponentAttributeMapping,
    available_components: list[ComponentType],
) -> _ComponentAttributeMappingDict:
    """Checks valid type for output_component_types. Also checks for any invalid component names and attribute names

    Args:
        output_type (OutputType): the type of output that the user will see (as per the calculation options)
        output_component_types (ComponentAttributeMapping): output_component_types provided by user
        available_components (list[ComponentType]): all components available in model instance

    Raises:
        ValueError: when the type for output_comoponent_types is incorrect
        KeyError: with "unknown component" for any unknown components
        KeyError: with "unknown attributes" for any unknown attributes for a known component

    Returns:
        _OutputComponentTypeDict: processed output_component_types in a dictionary
    """
    # limit all component count to user specified component types in output and convert to a dict
    if output_component_types is None:
        output_component_types = {k: None for k in available_components}
    elif isinstance(output_component_types, (list, set)):
        output_component_types = {k: None for k in output_component_types}
    elif not isinstance(output_component_types, dict) or not all(
        attrs is None or isinstance(attrs, (set, list)) for attrs in output_component_types.values()
    ):
        raise ValueError(f"Invalid output_component_types provided: {output_component_types}")

    # raise error if some specified components are unknown
    output_meta = power_grid_meta_data[output_type.value]
    unknown_components = [x for x in output_component_types if x not in output_meta]
    if unknown_components:
        raise KeyError(f"You have specified some unknown component types: {unknown_components}")

    unknown_attributes = {}
    for comp_name, attrs in output_component_types.items():
        if attrs is None:
            continue
        attr_names = output_meta[comp_name].dtype.names
        diff = set(attrs).difference(attr_names) if attr_names is not None else set(attrs)
        if diff != set():
            unknown_attributes[comp_name] = diff

    if unknown_attributes:
        raise KeyError(f"You have specified some unknown attributes: {unknown_attributes}")

    return output_component_types
