# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Main power grid model class
"""
from typing import Dict, List, Optional, Set, Union

import numpy as np

from power_grid_model.enum import CalculationMethod, CalculationType
from power_grid_model.errors import PowerGridBatchError, PowerGridError, assert_error, find_error
from power_grid_model.index_integer import ID_c, ID_np, Idx_c, Idx_np
from power_grid_model.options import Options
from power_grid_model.power_grid_core import ModelPtr
from power_grid_model.power_grid_core import power_grid_core as pgc
from power_grid_model.power_grid_meta import CDataset, initialize_array, power_grid_meta_data, prepare_cpp_array


class PowerGridModel:
    _model_ptr: ModelPtr
    _all_component_count: Optional[Dict[str, int]]
    _independent: bool  # all update datasets consists of exactly the same components
    _cache_topology: bool  # there are no changes in topology (branch, source) in the update dataset

    @property
    def independent(self) -> bool:
        return self._independent

    @property
    def cache_topology(self) -> bool:
        return self._cache_topology

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
        instance = super().__new__(cls)
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
            prepared_input.n_components,
            prepared_input.components,
            prepared_input.n_component_elements_per_scenario,
            prepared_input.data_ptrs_per_component,
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
            prepared_update.n_components,
            prepared_update.components,
            prepared_update.n_component_elements_per_scenario,
            prepared_update.data_ptrs_per_component,
        )

    def get_indexer(self, component_type: str, ids: np.ndarray):
        """
        Get array of indexers given array of ids for component type

        Args:
            component_type: type of component
            ids: array of ids

        Returns:
            array of inderxers, same shape as input array ids

        """
        ids_c = np.ascontiguousarray(ids, dtype=ID_np).ctypes.data_as(ID_c)
        indexer = np.empty_like(ids_c, dtype=Idx_np, order="C")
        indexer_c = indexer.ctypes.data_as(Idx_c)
        size = ids.size
        # call c function
        pgc.get_indexer(self._model, component_type, size, ids_c, indexer_c)
        return indexer

    def calculate(
        self,
        calculation_type: CalculationType,
        symmetric: bool,
        error_tolerance: float,
        max_iterations: int,
        calculation_method: Union[CalculationMethod, str],
        update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]],
        threading: int,
        output_component_types: Optional[Union[Set[str], List[str]]],
    ):
        """
        Core calculation routine

        Args:
            calculation_type:
            symmetric:
            error_tolerance:
            max_iterations:
            calculation_method:
            update_data:
            threading:
            output_component_types:

        Returns:

        """
        if isinstance(calculation_method, str):
            calculation_method = getattr(CalculationMethod, calculation_method)
        if symmetric:
            output_type = "sym_output"
        else:
            output_type = "asym_output"

        # prepare update dataset
        # update data exist for batch calculation
        if update_data is not None:
            batch_calculation = True
        # no update dataset, create one batch with empty set
        else:
            batch_calculation = False
            update_data = {}
        prepared_update: CDataset = prepare_cpp_array(data_type="update", array_dict=update_data)
        batch_size = prepared_update.batch_size

        # prepare result dataset
        all_component_count = self.all_component_count
        # for power flow, there is no need for sensor output
        if calculation_type == "power_flow":
            all_component_count = {k: v for k, v in all_component_count.items() if "sensor" not in k}
        # limit all component count to user specified component types in output
        if output_component_types is None:
            output_component_types = set(all_component_count.keys())
        # raise error is some specified components are unknown
        unknown_components = [x for x in output_component_types if x not in power_grid_meta_data[output_type]]
        if unknown_components:
            raise KeyError(f"You have specified some unknown component types: {unknown_components}")
        all_component_count = {k: v for k, v in all_component_count.items() if k in output_component_types}
        # create result dataset
        result_dict = {}
        for name, count in all_component_count.items():
            # intialize array
            arr = initialize_array(output_type, name, (batch_size, count), empty=True)
            result_dict[name] = arr
        prepared_result: CDataset = prepare_cpp_array(data_type=output_type, array_dict=result_dict)

        # prepare options
        opt: Options = Options()
        opt.calculation_type = calculation_type
        opt.calculation_method = calculation_method
        opt.symmetric = symmetric
        opt.error_tolerance = error_tolerance
        opt.max_iteration = max_iterations
        opt.threading = threading

        # run calculation
        pgc.calculate(
            # model and options
            self._model,
            opt.opt,
            # result dataset
            prepared_result.n_components,
            prepared_result.components,
            prepared_result.data_ptrs_per_component,
            # update dataset
            batch_size,
            prepared_update.n_components,
            prepared_update.components,
            prepared_update.n_component_elements_per_scenario,
            prepared_update.indptrs_per_component,
            prepared_update.data_ptrs_per_component,
        )
        assert_error()

        # flatten array for normal calculation
        if not batch_calculation:
            result_dict = {k: v.ravel() for k, v in result_dict.items()}
        # batch parameters
        self._independent = pgc.is_batch_independent()
        self._cache_topology = pgc.is_batch_cache_topology()

        return result_dict

    def calculate_power_flow(self, *_args, **kwargs):
        return self.calculate(CalculationType.power_flow, **kwargs)

    def calculate_state_estimation(self, *_args, **kwargs):
        return self.calculate(CalculationType.state_estimation, **kwargs)

    def __del__(self):
        pgc.destroy_model(self._model_ptr)
