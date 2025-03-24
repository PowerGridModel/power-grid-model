# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Data handling
"""

import numpy as np

from power_grid_model._core.dataset_definitions import ComponentType, DatasetType
from power_grid_model._core.power_grid_dataset import CConstDataset, CMutableDataset
from power_grid_model._core.power_grid_meta import initialize_array, power_grid_meta_data
from power_grid_model._utils import process_data_filter
from power_grid_model.data_types import Dataset, SingleDataset
from power_grid_model.enum import CalculationType, ComponentAttributeFilterOptions
from power_grid_model.errors import PowerGridUnreachableHitError
from power_grid_model.typing import ComponentAttributeMapping


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


def prepare_input_view(input_data: SingleDataset) -> CConstDataset:
    """
    Create a view of the input data in a format compatible with the PGM core libary.

    Args:
        input_data:
            the input data to create the view from

    Returns:
        instance of CConstDataset ready to be fed into C API
    """
    return CConstDataset(input_data, dataset_type=DatasetType.input)


def prepare_update_view(update_data: Dataset) -> CConstDataset:
    """
    Create a view of the update data, or an empty view if not provided, in a format compatible with the PGM core libary.

    Args:
        update_data:
            the update data to create the view from. Defaults to None

    Returns:
        instance of CConstDataset ready to be fed into C API
    """
    return CConstDataset(update_data, dataset_type=DatasetType.update)


def prepare_output_view(output_data: Dataset, output_type: DatasetType) -> CMutableDataset:
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
    result_dict: Dataset = {}

    for name, count in all_component_count.items():
        # shape
        if is_batch:
            shape: tuple[int] | tuple[int, int] = (batch_size, count)
        else:
            shape = (count,)

        requested_component = processed_output_types[name]
        dtype = power_grid_meta_data[output_type][name].dtype
        if dtype.names is None:
            raise PowerGridUnreachableHitError
        if requested_component is None:
            result_dict[name] = initialize_array(output_type, name, shape=shape, empty=True)
        elif requested_component in [
            ComponentAttributeFilterOptions.everything,
            ComponentAttributeFilterOptions.relevant,
        ]:
            result_dict[name] = {attr: np.empty(shape, dtype=dtype[attr]) for attr in dtype.names}
        elif isinstance(requested_component, list | set):
            result_dict[name] = {attr: np.empty(shape, dtype=dtype[attr]) for attr in requested_component}

    return result_dict
