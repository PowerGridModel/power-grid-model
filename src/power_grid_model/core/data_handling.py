# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Data handling
"""


from enum import Enum
from typing import Dict, List, Mapping, Optional, Set, Union

import numpy as np

from power_grid_model.core.power_grid_meta import CDataset, initialize_array, power_grid_meta_data, prepare_cpp_array
from power_grid_model.enum import CalculationType


class OutputType(Enum):
    """
    the different supported output types:
        - sym_output
        - asym_output
    """

    SYM_OUTPUT = "sym_output"
    ASYM_OUTPUT = "asym_output"
    SC_OUTPUT = "sc_output"


def get_output_type(*, calculation_type: CalculationType, symmetric: bool) -> OutputType:
    """
    get the output type based on the provided arguments

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


def is_batch_calculation(update_data: Optional[Mapping[str, Union[np.ndarray, Mapping[str, np.ndarray]]]]) -> bool:
    """
    returns whether the provided update data represents a batch calculation or not

    Args:
        update_data:
            The update data.

    Returns:
        True if the update_data is provided, else False.
    """
    # update data exists for batch calculation
    return update_data is not None


def prepare_input_view(input_data: Mapping[str, np.ndarray]) -> CDataset:
    """
    create a view of the input data in a format compatible with the PGM core libary

    Args:
        input_data:
            the input data to create the view from

    Returns:
        instance of CDataset ready to be fed into C API
    """
    return prepare_cpp_array(data_type="input", array_dict=input_data)


def prepare_update_view(
    update_data: Optional[Mapping[str, Union[np.ndarray, Mapping[str, np.ndarray]]]] = None
) -> CDataset:
    """
    create a view of the update data, or an empty view if not provided, in a format compatible with the PGM core libary

    Args:
        update_data:
            the update data to create the view from. Defaults to None

    Returns:
        instance of CDataset ready to be fed into C API
    """
    if update_data is None:
        # no update dataset, create one batch with empty set
        update_data = {}
    return prepare_cpp_array(data_type="update", array_dict=update_data)


def prepare_output_view(output_data: Mapping[str, np.ndarray], output_type: OutputType) -> CDataset:
    """
    create a view of the output data in a format compatible with the PGM core libary

    Args:
        output_data:
            the output data to create the view from
        output_type:
            the output type of the output_data

    Returns:
        instance of CDataset ready to be fed into C API
    """
    return prepare_cpp_array(data_type=output_type.value, array_dict=output_data)


def create_output_data(
    output_component_types: Union[Set[str], List[str]],
    output_type: OutputType,
    all_component_count: Dict[str, int],
    batch_size: int,
) -> Dict[str, np.ndarray]:
    """
    create the output data that the user can use. always returns batch type output data.
        Use reduce_output_data to flatten to single scenario output if applicable.

    Args:
        output_component_types:
            the output components the user seeks to extract
        output_type:
            the type of output that the user will see (as per the calculation options)
        all_component_count:
            the amount of components in the grid (as per the input data)
        batch_size:
            the batch size

    Returns:
        dictionary of results of all components
            key: component type name to be updated in batch
            value:
                for single calculation: 1D numpy structured array for the results of this component type
                for batch calculation: 2D numpy structured array for the results of this component type
                    Dimension 0: each batch
                    Dimension 1: the result of each element for this component type
        Error handling:
            in case if some specified components are unknown, a KeyError will be raised.
    """
    # raise error if some specified components are unknown
    unknown_components = [x for x in output_component_types if x not in power_grid_meta_data[output_type.value]]
    if unknown_components:
        raise KeyError(f"You have specified some unknown component types: {unknown_components}")

    all_component_count = {k: v for k, v in all_component_count.items() if k in output_component_types}

    # create result dataset
    result_dict = {}
    for name, count in all_component_count.items():
        # intialize array
        arr = initialize_array(output_type.value, name, (batch_size, count), empty=True)
        result_dict[name] = arr

    return result_dict


def reduce_output_data(output_data: Dict[str, np.ndarray], batch_calculation: bool) -> Dict[str, np.ndarray]:
    """
    reformat the output data into the format desired by the user

    Args:
        output_data:
            the output data per component in batch format
        batch_calculation:
            whether the intended output data format should be in the format returned by a batch calculation

    Returns:
        the original data, but with the data array per component flattened for normal calculation
    """
    if not batch_calculation:
        output_data = {k: v.ravel() for k, v in output_data.items()}

    return output_data
