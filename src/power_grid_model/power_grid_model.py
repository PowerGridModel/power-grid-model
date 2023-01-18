# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Main power grid model class
"""
from typing import Dict

import numpy as np

from power_grid_model.power_grid_meta import initialize_array, prepare_cpp_array, CDataset
from index_integer import Idx_np
from errors import assert_error
from power_grid_model.power_grid_core import ModelPtr
from power_grid_model.power_grid_core import power_grid_core as pgc


class PowerGridModel:
    _model_ptr: ModelPtr

    @property
    def _model(self):
        if not self._model_ptr:
            raise TypeError("You have an empty instance of PowerGridModel!")
        return self._model_ptr

    def copy(self) -> "PowerGridModel":
        """

        Copy the current model

        Returns:
            a copy of PowerGridModel
        """
        new_model = PowerGridModel.__new__(PowerGridModel)
        new_model._model_ptr = pgc.copy_model(self._model)
        return new_model

    def __copy__(self):
        return self.copy()

    def __new__(cls, *args, **kwargs):
        instance = super().__new__(cls, *args, **kwargs)
        instance._model_ptr = ModelPtr()
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
