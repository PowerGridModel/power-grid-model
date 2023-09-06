# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Power grid model raw dataset handler
"""

from typing import Dict, List, Mapping, Optional, Union

import numpy as np

from power_grid_model.core.error_handling import VALIDATOR_MSG, assert_no_error
from power_grid_model.core.index_integer import IdxNp
from power_grid_model.core.power_grid_core import DatasetInfoPtr, IdxPtr, VoidPtr
from power_grid_model.core.power_grid_core import power_grid_core as pgc
from power_grid_model.core.power_grid_meta import CBuffer, power_grid_meta_data
from power_grid_model.errors import PowerGridError


class CDatasetInfo:  # pylint: disable=too-few-public-methods
    """
    Raw Power Grid Model dataset info.
    """

    def __init__(self, info: DatasetInfoPtr):
        self._info: DatasetInfoPtr = info

    def name(self) -> str:
        """
        The name of the dataset type.

        Returns:
            The name of the dataset type
        """
        return pgc.dataset_info_name(self._info)

    def dataset_type(self):
        """ "
        The name of the dataset type.

        Returns:
            The name of the dataset type
        """
        return self.name()

    def is_batch(self) -> bool:
        """ "
        Whether the dataset is a batch dataset.

        Returns:
            Whether the dataset is a batch dataset
        """
        return bool(pgc.dataset_info_is_batch(self._info))

    def batch_size(self) -> int:
        """ "
        The size of the dataset.

        Returns:
            The size of the dataset
        """
        return pgc.dataset_info_batch_size(self._info)

    def n_components(self) -> int:
        """ "
        The amount of components in the dataset.

        Returns:
            The amount of components in the dataset
        """
        return pgc.dataset_info_n_components(self._info)

    def components(self) -> List[str]:
        """ "
        The components in the dataset.

        Returns:
            A list of the component names in the dataset
        """
        return [pgc.dataset_info_component_name(self._info, idx) for idx in range(self.n_components())]

    def elements_per_scenario(self) -> Dict[str, int]:
        """ "
        The number of elements per scenario in the dataset.

        Returns:
            The number of elements per senario for each component in the dataset;
            or -1 if the scenario is not uniform (different amount per scenario)
        """
        return {
            component_name: pgc.dataset_info_elements_per_scenario(self._info, idx)
            for idx, component_name in enumerate(self.components())
        }

    def total_elements(self) -> Dict[str, int]:
        """ "
        The total number of elements in the dataset.

        Returns:
            The total number of elements for each component.
            For each component, if the number of elements per scenario is uniform, its value shall be equal to
                the product of the batch size and the amount of elements per scenario for that component.
        """
        return {
            component_name: pgc.dataset_info_total_elements(self._info, idx)
            for idx, component_name in enumerate(self.components())
        }


class CConstDataset:
    """
    A view of a dataset.

    This may be used to provide a user dataset to the Power Grid Model.
    """

    def __init__(self, dataset_type: str):
        self._dataset_type = dataset_type
        self._schema = power_grid_meta_data[self._dataset_type]

        self._is_batch: Optional[bool] = None
        self._batch_size: Optional[int] = None

        self._const_dataset = pgc.create_dataset_const(self._dataset_type, self._is_batch, self._batch_size)
        assert_no_error()

    def get_info(self) -> CDatasetInfo:
        """
        Get the info for this dataset.

        Returns:
            The dataset info for this dataset
        """
        return CDatasetInfo(pgc.dataset_const_get_info(self._const_dataset))

    def add_data(self, data: Dict[str, Union[np.ndarray, Mapping[str, np.ndarray]]]):
        """
        Add Power Grid Model data to the const dataset view.

        Args:
            data: the data

        Raises:
            ValueError if the component is unknown and allow_unknown is False.
            ValueError if the data is inconsistent with the rest of the dataset.
            PowerGridError if there was an internal error.
        """
        for component, component_data in data.items():
            self.add_component_data(component, component_data, allow_unknown=True)

    def add_component_data(
        self, component: str, data: Union[np.ndarray, Mapping[str, np.ndarray]], allow_unknown: bool = False
    ):
        """
        Add Power Grid Model data for a single component to the const dataset view.

        Args:
            component: the name of the component
            data: the data of the component
            allow_unknown (optional): ignore any unknown components. Defaults to False.

        Raises:
            ValueError if the component is unknown and allow_unknown is False.
            ValueError if the data is inconsistent with the rest of the dataset.
            PowerGridError if there was an internal error.
        """
        if component not in self._schema:
            if not allow_unknown:
                raise ValueError(f"Unknown component {component} in schema. {VALIDATOR_MSG}")
            return

        buffer = self._get_buffer(component, data)

        pgc.dataset_const_add_buffer(
            self._const_dataset,
            component,
            -1 if not isinstance(data, np.ndarray) else data.shape[-1],
            buffer.total_elements,
            buffer.indptr,
            buffer.data,
        )
        assert_no_error()

    def _get_buffer(self, component: str, data: Union[np.ndarray, Mapping[str, np.ndarray]]) -> CBuffer:
        if isinstance(data, np.ndarray):
            return self._get_homogeneous_buffer(component, data)

        return self._get_sparse_buffer(component, data)

    def _get_homogeneous_buffer(self, component: str, data: np.ndarray):
        if data.ndim not in (1, 2):
            raise ValueError(f"Array can only be 1D or 2D. {VALIDATOR_MSG}")

        is_batch = data.ndim == 2
        batch_size = 1 if is_batch else data.shape[0]
        n_elements_per_scenario = data.shape[-1]
        n_total_elements = batch_size * n_elements_per_scenario

        self._validate_data_format(is_batch, batch_size, n_elements_per_scenario, n_total_elements)

        return CBuffer(
            data=self._raw_view(component, data),
            indptr=IdxPtr(),
            n_elements_per_scenario=n_elements_per_scenario,
            batch_size=batch_size,
            total_elements=n_total_elements,
        )

    def _get_sparse_buffer(self, component: str, data_mapping: Mapping[str, np.ndarray]) -> CBuffer:
        data = data_mapping["data"]
        indptr = data_mapping["indptr"]

        if data.ndim != 1:
            raise ValueError(f"Data array can only be 1D. {VALIDATOR_MSG}")
        if indptr.ndim != 1:
            raise ValueError(f"indptr can only be 1D. {VALIDATOR_MSG}")
        if indptr[0] != 0 or indptr[-1] != data.size:
            raise ValueError(f"indptr should start from zero and end at size of data array. {VALIDATOR_MSG}")
        if np.any(np.diff(indptr) < 0):
            raise ValueError(f"indptr should be increasing. {VALIDATOR_MSG}")

        is_batch = True
        batch_size = indptr.size - 1
        n_elements_per_scenario = -1
        n_total_elements = len(data)

        self._validate_data_format(is_batch, batch_size, n_elements_per_scenario, n_total_elements)

        return CBuffer(
            data=self._raw_view(component, data),
            indptr=self._get_indptr_view(indptr),
            n_elements_per_scenario=n_elements_per_scenario,
            batch_size=batch_size,
            total_elements=n_total_elements,
        )

    def _validate_data_format(
        self, is_batch: bool, batch_size: int, n_elements_per_scenario: int, n_total_elements: int
    ):
        if self._is_batch is None and self._batch_size is None:
            # cache
            self._is_batch = is_batch
            self._batch_size = batch_size

        # validate
        if is_batch != self._is_batch:
            raise ValueError(
                f"Dataset type (single or batch) must be consistent across all components. {VALIDATOR_MSG}"
            )
        if batch_size != self._batch_size:
            raise ValueError(f"Dataset must have a consistent batch size across all components. {VALIDATOR_MSG}")
        if self._is_batch and self._dataset_type == "input":
            raise ValueError(f"Input datasets cannot be batch dataset type. {VALIDATOR_MSG}")

        if batch_size != 1 and not is_batch:
            raise ValueError(f"Dataset must either be a batch dataset or have batch size equal to 1 {VALIDATOR_MSG}")
        if batch_size < 0:
            raise ValueError(f"Dataset batch size cannot be negative. {VALIDATOR_MSG}")
        if n_elements_per_scenario != -1 and n_total_elements != batch_size * n_elements_per_scenario:
            raise PowerGridError("Internal dataset error. Please file a bug report with the repro case.")

    def _raw_view(self, component: str, data: np.ndarray):
        return np.ascontiguousarray(data, dtype=self._schema[component].dtype).ctypes.data_as(VoidPtr)

    def _get_indptr_view(self, indptr: np.ndarray):
        return np.ascontiguousarray(indptr, dtype=IdxNp).ctypes.data_as(IdxPtr)

    def __del__(self):
        pgc.destroy_dataset_const(self._const_dataset)
