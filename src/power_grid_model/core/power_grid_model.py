# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Main power grid model class
"""
from typing import Dict, List, Optional, Set, Union

import numpy as np

from power_grid_model.core.error_handling import PowerGridBatchError, assert_no_error, find_error
from power_grid_model.core.index_integer import IdNp, IdxNp
from power_grid_model.core.options import Options
from power_grid_model.core.power_grid_core import IDPtr, IdxPtr, ModelPtr
from power_grid_model.core.power_grid_core import power_grid_core as pgc
from power_grid_model.core.power_grid_meta import CDataset, initialize_array, power_grid_meta_data, prepare_cpp_array
from power_grid_model.enum import CalculationMethod, CalculationType


class PowerGridModel:
    """
    Main class for Power Grid Model
    """

    _model_ptr: ModelPtr
    _all_component_count: Optional[Dict[str, int]]
    _batch_error: Optional[PowerGridBatchError]

    @property
    def batch_error(self) -> Optional[PowerGridBatchError]:
        """
        Get the batch error object, if present

        Returns: Batch error object, or None

        """
        return self._batch_error

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
        if self._all_component_count is None:
            raise TypeError("You have an empty instance of PowerGridModel!")
        return self._all_component_count

    def copy(self) -> "PowerGridModel":
        """

        Copy the current model

        Returns:
            a copy of PowerGridModel
        """
        new_model = PowerGridModel.__new__(PowerGridModel)
        new_model._model_ptr = pgc.copy_model(self._model)  # pylint: disable=W0212
        assert_no_error()
        new_model._all_component_count = self._all_component_count  # pylint: disable=W0212
        return new_model

    def __copy__(self):
        return self.copy()

    def __new__(cls, *_args, **_kwargs):
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
        # destroy old instance
        pgc.destroy_model(self._model_ptr)
        self._all_component_count = None
        # create new
        prepared_input: CDataset = prepare_cpp_array("input", input_data)
        self._model_ptr = pgc.create_model(
            system_frequency,
            components=prepared_input.components,
            n_components=prepared_input.n_components,
            component_sizes=prepared_input.n_component_elements_per_scenario,
            input_data=prepared_input.data_ptrs_per_component,
        )
        assert_no_error()
        self._all_component_count = {
            k: v.n_elements_per_scenario for k, v in prepared_input.dataset.items() if v.n_elements_per_scenario > 0
        }

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
        assert_no_error()

    def get_indexer(self, component_type: str, ids: np.ndarray):
        """
        Get array of indexers given array of ids for component type

        Args:
            component_type: type of component
            ids: array of ids

        Returns:
            array of inderxers, same shape as input array ids

        """
        ids_c = np.ascontiguousarray(ids, dtype=IdNp).ctypes.data_as(IDPtr)
        indexer = np.empty_like(ids, dtype=IdxNp, order="C")
        indexer_c = indexer.ctypes.data_as(IdxPtr)
        size = ids.size
        # call c function
        pgc.get_indexer(self._model, component_type, size, ids_c, indexer_c)
        assert_no_error()
        return indexer

    # pylint: disable=too-many-locals
    # pylint: disable=too-many-branches
    # pylint: disable=too-many-arguments
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
        continue_on_batch_error: bool,
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
            continue_on_batch_error:

        Returns:

        """
        if isinstance(calculation_method, str):
            calculation_method = CalculationMethod[calculation_method]
        if symmetric:
            output_type = "sym_output"
        else:
            output_type = "asym_output"
        self._batch_error = None

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

        # error handling
        if not continue_on_batch_error:
            assert_no_error(batch_size=batch_size)
        else:
            # continue on batch error
            error: Optional[RuntimeError] = find_error(batch_size=batch_size)
            if error is not None:
                if isinstance(error, PowerGridBatchError):
                    # continue on batch error
                    self._batch_error = error
                else:
                    # raise normal error
                    raise error

        # flatten array for normal calculation
        if not batch_calculation:
            result_dict = {k: v.ravel() for k, v in result_dict.items()}

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
        continue_on_batch_error: bool = False,
    ) -> Dict[str, np.ndarray]:
        """
        Calculate power flow once with the current model attributes.
        Or calculate in batch with the given update dataset in batch.

        Args:
            symmetric (bool): Whether to perform a three-phase symmetric calculation.

                - True: Three-phase symmetric calculation, even for asymmetric loads/generations.
                - False: Three-phase asymmetric calculation.

            error_tolerance: Error tolerance for voltage in p.u., applicable only when iterative=True.

            max_iterations: Maximum number of iterations, applicable only when iterative=True.

            calculation_method (an enumeration or string): The calculation method to use.

                - Newton_raphson: Use Newton-Raphson iterative method (default).
                - Linear: Use linear method.
           
            update_data:
              None: Calculate power flow once with the current model attributes.
              Or a dictionary for batch calculation with batch update.
                  
                    key: Component type name to be updated in batch.
                        - For homogeneous update batch (a 2D numpy structured array):

                            - Dimension 0: Each batch.
                            - Dimension 1: Each updated element per batch for this component type.
                        - For inhomogeneous update batch (a dictionary containing two keys):

                            - indptr: A 1D integer numpy array with length n_batch + 1. Given batch number k, the update array for this batch is
                              data[indptr[k]:indptr[k + 1]]. This is the concept of compressed sparse structure.  
                              https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.csr_matrix.html
                            - data: 1D numpy structured array in flat.     

            threading: Applicable only for batch calculation.

                - < 0: Sequential
                - = 0: Parallel, use number of hardware threads
                - > 0: Specify number of parallel threads  
            
            output_component_types: List or set of component types to be included in the
                output dict. By default, all component types will be in the output.

            continue_on_batch_error: If the program continues (instead of throwing error) if some scenarios fails.
    
        Returns: Dictionary of results of all components.
            
            Key: Component type name to be updated in batch.      
                - For single calculation: 1D numpy structured array for the results of this component type.
                - For batch calculation: 2D numpy structured array for the results of this component type.              
                    - Dimension 0: Each batch.
                    - Dimension 1: The result of each element for this component type.
    
        Raises: In case an error in the core occurs, an exception will be thrown.
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
            continue_on_batch_error=continue_on_batch_error,
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
        continue_on_batch_error: bool = False,
    ) -> Dict[str, np.ndarray]:
        """This is an example of a module level function.

        Function parameters should be documented in the ``Args`` section. The name
        of each parameter is required. The type and description of each parameter
        is optional, but should be included if not obvious.

        If \*args or \*\*kwargs are accepted,
        they should be listed as ``*args`` and ``**kwargs``.

        The format for a parameter is::

            name (type): description
                The description may span multiple lines. Following
                lines should be indented. The "(type)" is optional.

                Multiple paragraphs are supported in parameter
                descriptions.

        Args:
            param1 (int): The first parameter.
            param2 (:obj:`str`, optional): The second parameter. 
                Defaults to None.
                Second line of description should be indented.
            *args: Variable length argument list.
            **kwargs: Arbitrary keyword arguments.

        Returns:
            bool: True if successful, False otherwise.

            The return type is optional and may be specified at the beginning of
            the ``Returns`` section followed by a colon.

            The ``Returns`` section may span multiple lines and paragraphs.
            Following lines should be indented to match the first line.

            The ``Returns`` section supports any reStructuredText formatting,
            including literal blocks::

                {
                    'param1': param1,
                    'param2': param2
                }

            .. _PEP 484:
                https://www.python.org/dev/peps/pep-0484/
                
        Raises:
            AttributeError: The ``Raises`` section is a list of all exceptions
                that are relevant to the interface.
            ValueError: If `param2` is equal to `param1`.

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
            continue_on_batch_error=continue_on_batch_error,
        )

    def __del__(self):
        pgc.destroy_model(self._model_ptr)
