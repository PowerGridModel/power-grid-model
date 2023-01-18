# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Main power grid model class
"""
from typing import Dict, Optional

import numpy as np
from errors import assert_error
from index_integer import Idx_np

from power_grid_model.power_grid_core import ModelPtr
from power_grid_model.power_grid_core import power_grid_core as pgc
from power_grid_model.power_grid_meta import CDataset, initialize_array, prepare_cpp_array


class PowerGridModel:
    _model_ptr: ModelPtr
    _all_component_count: Optional[Dict[str, int]]

    @property
    def _model(self):
        if not self._model_ptr:
            raise TypeError("You have an empty instance of PowerGridModel!")
        return self._model_ptr

    @property
    def all_component_count(self) -> Dict[str, int]:
        """
        Get count of number of elements per component type.
        If the count for a component type is zero, it will not be in the returned dictionary.
        Returns:
            a dictionary with
                key: component type name
                value: integer count of elements of this type
        """
        return self._all_component_count

    def copy(self) -> "PowerGridModel":
        """

        Copy the current model

        Returns:
            a copy of PowerGridModel
        """
        new_model = PowerGridModel.__new__(PowerGridModel)
        new_model._model_ptr = pgc.copy_model(self._model)
        assert_error()
        new_model._all_component_count = self._all_component_count
        return new_model

    def __copy__(self):
        return self.copy()

    def __new__(cls, *args, **kwargs):
        instance = super().__new__(cls, *args, **kwargs)
        instance._model_ptr = ModelPtr()
        instance._all_component_count = None
        return instance

    def __init__(self, input_data: Dict[str, np.ndarray], system_frequency: float = 50.0):
        """
        Initialize the model from an input data set.

        Args:
            input_data: input data dictionary
                key: component type name
                value: 1D numpy structured array for this component input
            system_frequency: frequency of the power system, default 50 Hz
        """
        prepared_input: CDataset = prepare_cpp_array("input", input_data)
        self._model_ptr = pgc.create_model(
            system_frequency,
            prepared_input.n_types,
            prepared_input.type_names,
            prepared_input.items_per_type_per_batch,
            prepared_input.data_ptrs_per_type,
        )
        assert_error()
        self._all_component_count = {k: v.size for k, v in input_data.items()}

    def update(self, *, update_data: Dict[str, np.ndarray]):
        """
        Update the model with changes.
        Args:
            update_data: update data dictionary
                key: component type name
                value: 1D numpy structured array for this component update
        Returns:
            None
        """
        prepared_update: CDataset = prepare_cpp_array("update", update_data)
        pgc.update_model(
            self._model,
            prepared_update.n_types,
            prepared_update.type_names,
            prepared_update.items_per_type_per_batch,
            prepared_update.data_ptrs_per_type,
        )
