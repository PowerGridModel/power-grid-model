# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Data handling
"""


from enum import Enum
from typing import Dict, List, Mapping, Optional, Set, Union

import numpy as np

from power_grid_model.core.index_integer import IdxNp
from power_grid_model.core.power_grid_dataset import CDatasetInfo
from power_grid_model.core.power_grid_meta import CDataset, initialize_array, power_grid_meta_data, prepare_cpp_array
from power_grid_model.enum import CalculationType


class DatasetType(Enum):
    """
    The different supported dataset types:
        - input
        - update
        - sym_output
        - asym_output
        - sc_output
        ...
    """

    INPUT = "input"
    UPDATE = "update"
    SYM_OUTPUT = "sym_output"
    ASYM_OUTPUT = "asym_output"
    SC_OUTPUT = "sc_output"


class OutputType(Enum):
    """
    The different supported output types:
        - sym_output
        - asym_output
    """

    SYM_OUTPUT = "sym_output"
    ASYM_OUTPUT = "asym_output"
    SC_OUTPUT = "sc_output"


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


def is_batch_calculation(update_data: Optional[Mapping[str, Union[np.ndarray, Mapping[str, np.ndarray]]]]) -> bool:
    """
    Returns whether the provided update data represents a batch calculation or not.

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
    Create a view of the input data in a format compatible with the PGM core libary.

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
    Create a view of the update data, or an empty view if not provided, in a format compatible with the PGM core libary.

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
    create a view of the output data in a format compatible with the PGM core libary.

    Args:
        output_data:
            the output data to create the view from
        output_type:
            the output type of the output_data

    Returns:
        instance of CDataset ready to be fed into C API
    """
    return prepare_cpp_array(data_type=output_type.value, array_dict=output_data)


def create_dataset(
    *,
    component_types: Union[Set[str], List[str]],
    dataset_type: DatasetType,
    batch_size: int,
    homogeneous_component_count: Dict[str, int],
    sparse_component_count: Optional[Dict[str, int]] = None,
) -> Dict[str, np.ndarray]:
    """
    Create the dataset that the user can use. always returns batch type dataset.
        Use reduce_output_data to flatten to single scenario output if applicable.

    Args:
        component_types:
            the relevant components requested in the dataset
        dataset_type:
            the type of dataset that the user will see (as per the calculation options)
        batch_size:
            the batch size
        homogeneous_component_count:
            the amount of components in the grid, represented as a homogeneous dataset (as in the input data)
        sparse_component_count:
            the components for which to create a sparse data set and their respective total count across all batches;
            takes precedence over the homogeneous component count

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
    unknown_components = [x for x in component_types if x not in power_grid_meta_data[dataset_type.value]]
    if unknown_components:
        raise KeyError(f"You have specified some unknown component types: {unknown_components}")

    homogeneous_component_count = {k: v for k, v in homogeneous_component_count.items() if k in component_types}

    # create result dataset
    result_dict = {}

    for name, count in homogeneous_component_count.items():
        if sparse_component_count is None or name not in sparse_component_count:
            result_dict[name] = initialize_array(dataset_type.value, name, (batch_size, count), empty=True)

    if sparse_component_count is not None:
        for name, count in sparse_component_count.items():
            result_dict[name] = {
                "indptr": np.empty(shape=batch_size + 1, dtype=IdxNp),
                "data": initialize_array(dataset_type.value, name, count, empty=True),
            }

    return result_dict


def create_output_data(
    output_component_types: Union[Set[str], List[str]],
    output_type: OutputType,
    all_component_count: Dict[str, int],
    batch_size: int,
) -> Dict[str, np.ndarray]:
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
    return create_dataset(
        component_types=output_component_types,
        dataset_type=DatasetType[output_type.value.upper()],
        batch_size=batch_size,
        homogeneous_component_count=all_component_count,
    )


def reduce_dataset(dataset: Dict[str, np.ndarray], batch_calculation: bool) -> Dict[str, np.ndarray]:
    """
    Reformat the dataset into the format desired by the user.

    Args:
        dataset:
            the dataset per component in batch format
        batch_calculation:
            whether the intended output data format should be in the format returned by a batch calculation

    Returns:
        the original data, but with the data array per component flattened for normal calculation
    """
    if not batch_calculation:
        dataset = {k: v.ravel() for k, v in dataset.items()}

    return dataset


def reduce_output_data(output_data: Dict[str, np.ndarray], batch_calculation: bool) -> Dict[str, np.ndarray]:
    """
    Reformat the output data into the format desired by the user.

    Args:
        output_data:
            the output data per component in batch format
        batch_calculation:
            whether the intended output data format should be in the format returned by a batch calculation

    Returns:
        the original data, but with the data array per component flattened for normal calculation
    """
    return reduce_dataset(dataset=output_data, batch_calculation=batch_calculation)


def create_dataset_from_info(info: CDatasetInfo) -> Dict[str, np.ndarray]:
    """
    Create the dataset that the user can use.

    Args:
        info:
            the dataset info

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
    total_elements = info.total_elements().items()
    elements_per_scenario = info.elements_per_scenario()
    sparse_component_count = {
        component: count for component, count in total_elements if elements_per_scenario.get(component, -1) == -1
    }
    return reduce_dataset(
        dataset=create_dataset(  # dense data set
            component_types=set(elements_per_scenario.keys()),
            dataset_type=DatasetType[info.dataset_type().upper()],
            homogeneous_component_count=elements_per_scenario,
            batch_size=info.batch_size(),
            sparse_component_count=sparse_component_count,
        ),
        batch_calculation=info.is_batch(),
    )
