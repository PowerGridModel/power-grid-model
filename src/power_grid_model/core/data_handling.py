# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Data handling
"""


from enum import Enum
from typing import Dict, List, Mapping, Set, Tuple, Union

import numpy as np

from power_grid_model.core.dataset_definitions import ComponentType, DatasetType
from power_grid_model.core.power_grid_dataset import CConstDataset, CMutableDataset
from power_grid_model.core.power_grid_meta import initialize_array, power_grid_meta_data
from power_grid_model.enum import CalculationType


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


def prepare_update_view(
    update_data: Mapping[ComponentType, Union[np.ndarray, Mapping[str, np.ndarray]]]
) -> CConstDataset:
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
    output_component_types: Union[Set[ComponentType], List[ComponentType]],
    output_type: OutputType,
    all_component_count: Dict[ComponentType, int],
    is_batch: bool,
    batch_size: int,
) -> Dict[ComponentType, np.ndarray]:
    """
    Create the output data that the user can use. always returns batch type output data.
        Use reduce_output_data to flatten to single scenario output if applicable.

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

    Raises:
        KeyError: if some specified components are unknown.

    Returns:
        dictionary of results of all components
            key: component type name to be updated in batch
            value:
                for single calculation: 1D numpy structured array for the results of this component type
                for batch calculation: 2D numpy structured array for the results of this component type
                    Dimension 0: each batch
                    Dimension 1: the result of each element for this component type
    """
    # raise error if some specified components are unknown
    unknown_components = [x for x in output_component_types if x not in power_grid_meta_data[output_type.value]]
    if unknown_components:
        raise KeyError(f"You have specified some unknown component types: {unknown_components}")

    all_component_count = {k: v for k, v in all_component_count.items() if k in output_component_types}

    # create result dataset
    result_dict = {}

    for name, count in all_component_count.items():
        # shape
        if is_batch:
            shape: Union[Tuple[int], Tuple[int, int]] = (batch_size, count)
        else:
            shape = (count,)
        result_dict[name] = initialize_array(output_type.value, name, shape=shape, empty=True)

    return result_dict
