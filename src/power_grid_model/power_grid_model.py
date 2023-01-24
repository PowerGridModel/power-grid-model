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
from power_grid_model.power_grid_core import IDPtr, IdxPtr, ModelPtr
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
        ids_c = np.ascontiguousarray(ids, dtype=ID_np).ctypes.data_as(IDPtr)
        indexer = np.empty_like(ids, dtype=Idx_np, order="C")
        indexer_c = indexer.ctypes.data_as(IdxPtr)
        size = ids.size
        # call c function
        pgc.get_indexer(self._model, component_type, size, ids_c, indexer_c)
        return indexer

    def _calculate(
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
        if calculation_type == CalculationType.power_flow:
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
        opt.calculation_type = calculation_type.value
        opt.calculation_method = calculation_method.value
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

    def calculate_power_flow(
        self,
        *,
        symmetric: bool = True,
        error_tolerance: float = 1e-8,
        max_iterations: int = 20,
        calculation_method: Union[CalculationMethod, str] = CalculationMethod.newton_raphson,
        update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]] = None,
        threading: int = -1,
        output_component_types: Optional[Union[Set[str], List[str]]] = None,
    ) -> Dict[str, np.ndarray]:
        """
        Calculate power flow once with the current model attributes.
        Or calculate in batch with the given update dataset in batch

        Args:
            symmetric:
                True: three-phase symmetric calculation, even for asymmetric loads/generations
                False: three-phase asymmetric calculation
            error_tolerance:
                error tolerance for voltage in p.u., only applicable when iterative=True
            max_iterations:
                maximum number of iterations, only applicable when iterative=True
            calculation_method: an enumeration or string
                newton_raphson: use Newton-Raphson iterative method (default)
                linear: use linear method
            update_data:
                None: calculate power flow once with the current model attributes
                A dictionary for batch calculation with batch update
                    key: component type name to be updated in batch
                    value:
                        a 2D numpy structured array for homogeneous update batch
                            Dimension 0: each batch
                            Dimension 1: each updated element per batch for this component type
                        **or**
                        a dictionary containing two keys, for inhomogeneous update batch
                            indptr: a 1D integer numpy array with length n_batch + 1
                                given batch number k, the update array for this batch is
                                data[indptr[k]:indptr[k + 1]]
                                This is the concept of compressed sparse structure
                                https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.csr_matrix.html
                            data: 1D numpy structured array in flat
            threading:
                only applicable for batch calculation
                < 0 sequential
                = 0 parallel, use number of hardware threads
                > 0 specify number of parallel threads
            output_component_types: list or set of component types you want to be present in the output dict.
                By default all component types will be in the output

        Returns:
            dictionary of results of all components
                key: component type name to be updated in batch
                value:
                    for single calculation: 1D numpy structured array for the results of this component type
                    for batch calculation: 2D numpy structured array for the results of this component type
                        Dimension 0: each batch
                        Dimension 1: the result of each element for this component type
            Error handling:
                in case an error in the core occurs, an exception will be thrown
        """
        return self._calculate(
            CalculationType.power_flow,
            symmetric=symmetric,
            error_tolerance=error_tolerance,
            max_iterations=max_iterations,
            calculation_method=calculation_method,
            update_data=update_data,
            threading=threading,
            output_component_types=output_component_types,
        )

    def calculate_state_estimation(
        self,
        *,
        symmetric: bool = True,
        error_tolerance: float = 1e-8,
        max_iterations: int = 20,
        calculation_method: Union[CalculationMethod, str] = CalculationMethod.iterative_linear,
        update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]] = None,
        threading: int = -1,
        output_component_types: Optional[Union[Set[str], List[str]]] = None,
    ) -> Dict[str, np.ndarray]:
        """
        Calculate state estimation once with the current model attributes.
        Or calculate in batch with the given update dataset in batch

        Args:
            symmetric:
                True: three-phase symmetric calculation, even for asymmetric loads/generations
                False: three-phase asymmetric calculation
            error_tolerance:
                error tolerance for voltage in p.u., only applicable when iterative=True
            max_iterations:
                maximum number of iterations, only applicable when iterative=True
            calculation_method: an enumeration
                iterative_linear: use iterative linear method
            update_data:
                None: calculate state estimation once with the current model attributes
                A dictionary for batch calculation with batch update
                    key: component type name to be updated in batch
                    value:
                        a 2D numpy structured array for homogeneous update batch
                            Dimension 0: each batch
                            Dimension 1: each updated element per batch for this component type
                        **or**
                        a dictionary containing two keys, for inhomogeneous update batch
                            indptr: a 1D integer numpy array with length n_batch + 1
                                given batch number k, the update array for this batch is
                                data[indptr[k]:indptr[k + 1]]
                                This is the concept of compressed sparse structure
                                https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.csr_matrix.html
                            data: 1D numpy structured array in flat
            threading:
                only applicable for batch calculation
                < 0 sequential
                = 0 parallel, use number of hardware threads
                > 0 specify number of parallel threads
            output_component_types: list or set of component types you want to be present in the output dict.
                By default all component types will be in the output

        Returns:
            dictionary of results of all components
                key: component type name to be updated in batch
                value:
                    for single calculation: 1D numpy structured array for the results of this component type
                    for batch calculation: 2D numpy structured array for the results of this component type
                        Dimension 0: each batch
                        Dimension 1: the result of each element for this component type
            Error handling:
                in case an error in the core occurs, an exception will be thrown
        """
        return self._calculate(
            CalculationType.state_estimation,
            symmetric=symmetric,
            error_tolerance=error_tolerance,
            max_iterations=max_iterations,
            calculation_method=calculation_method,
            update_data=update_data,
            threading=threading,
            output_component_types=output_component_types,
        )

    def __del__(self):
        pgc.destroy_model(self._model_ptr)
