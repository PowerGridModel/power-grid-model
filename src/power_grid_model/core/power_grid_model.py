# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Main power grid model class
"""
from enum import IntEnum
from typing import Dict, List, Optional, Set, Type, Union

import numpy as np

from power_grid_model.core.data_handling import (
    create_output_data,
    get_output_type,
    prepare_input_view,
    prepare_output_view,
    prepare_update_view,
)
from power_grid_model.core.error_handling import PowerGridBatchError, assert_no_error, handle_errors
from power_grid_model.core.index_integer import IdNp, IdxNp
from power_grid_model.core.options import Options
from power_grid_model.core.power_grid_core import ConstDatasetPtr, IDPtr, IdxPtr, ModelPtr, power_grid_core as pgc
from power_grid_model.enum import CalculationMethod, CalculationType, ShortCircuitVoltageScaling, _ExperimentalFeatures


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

        Returns:
            Batch error object, or None
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
            A dictionary with

                - key: Component type name
                - value: Integer count of elements of this type
        """
        if self._all_component_count is None:
            raise TypeError("You have an empty instance of PowerGridModel!")
        return self._all_component_count

    def copy(self) -> "PowerGridModel":
        """
        Copy the current model

        Returns:
            A copy of PowerGridModel
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
            input_data: Input data dictionary

                - key: Component type name
                - value: 1D numpy structured array for this component input

            system_frequency: Frequency of the power system, default 50 Hz
        """
        # destroy old instance
        pgc.destroy_model(self._model_ptr)
        self._all_component_count = None
        # create new
        prepared_input = prepare_input_view(input_data)
        self._model_ptr = pgc.create_model(system_frequency, input_data=prepared_input.get_dataset_ptr())
        assert_no_error()
        self._all_component_count = {k: v for k, v in prepared_input.get_info().total_elements().items() if v > 0}

    def update(self, *, update_data: Dict[str, np.ndarray]):
        """
        Update the model with changes.

        Args:
            update_data: Update data dictionary

                - key: Component type name
                - value: 1D numpy structured array for this component update

        Returns:
            None
        """
        prepared_update = prepare_update_view(update_data)
        pgc.update_model(self._model, prepared_update.get_dataset_ptr())
        assert_no_error()

    def get_indexer(self, component_type: str, ids: np.ndarray):
        """
        Get array of indexers given array of ids for component type

        Args:
            component_type: Type of component
            ids: Array of ids

        Returns:
            Array of indexers, same shape as input array ids
        """
        ids_c = np.ascontiguousarray(ids, dtype=IdNp).ctypes.data_as(IDPtr)
        indexer = np.empty_like(ids, dtype=IdxNp, order="C")
        indexer_c = indexer.ctypes.data_as(IdxPtr)
        size = ids.size
        # call c function
        pgc.get_indexer(self._model, component_type, size, ids_c, indexer_c)
        assert_no_error()
        return indexer

    def _get_output_component_count(self, calculation_type: CalculationType):
        exclude_types = {
            CalculationType.power_flow: ["sensor", "fault"],
            CalculationType.state_estimation: ["fault"],
            CalculationType.short_circuit: ["sensor"],
        }.get(calculation_type, [])

        def include_type(component_type: str):
            for exclude_type in exclude_types:
                if exclude_type in component_type:
                    return False
            return True

        return {k: v for k, v in self.all_component_count.items() if include_type(k)}

    # pylint: disable=too-many-arguments
    def _construct_output(
        self,
        output_component_types: Optional[Union[Set[str], List[str]]],
        calculation_type: CalculationType,
        symmetric: bool,
        is_batch: bool,
        batch_size: int,
    ) -> Dict[str, np.ndarray]:
        all_component_count = self._get_output_component_count(calculation_type=calculation_type)

        # limit all component count to user specified component types in output
        if output_component_types is None:
            output_component_types = set(all_component_count.keys())

        return create_output_data(
            output_component_types=output_component_types,
            output_type=get_output_type(calculation_type=calculation_type, symmetric=symmetric),
            all_component_count=all_component_count,
            is_batch=is_batch,
            batch_size=batch_size,
        )

    @staticmethod
    def _options(**kwargs) -> Options:
        def as_enum_value(key_enum: str, type_: Type[IntEnum]):
            if key_enum in kwargs:
                value_enum = kwargs[key_enum]
                if isinstance(value_enum, str):
                    kwargs[key_enum] = type_[value_enum]

        as_enum_value("calculation_method", CalculationMethod)
        as_enum_value("short_circuit_voltage_scaling", ShortCircuitVoltageScaling)
        as_enum_value("experimental_features", _ExperimentalFeatures)

        opt = Options()
        for key, value in kwargs.items():
            setattr(opt, key, value.value if isinstance(value, IntEnum) else value)
        return opt

    def _handle_errors(self, continue_on_batch_error: bool, batch_size: int):
        self._batch_error = handle_errors(continue_on_batch_error=continue_on_batch_error, batch_size=batch_size)

    # pylint: disable=too-many-arguments
    def _calculate_impl(
        self,
        calculation_type: CalculationType,
        symmetric: bool,
        update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]],
        output_component_types: Optional[Union[Set[str], List[str]]],
        options: Options,
        continue_on_batch_error: bool,
    ):
        """
        Core calculation routine

        Args:
            calculation_type:
            symmetric:
            update_data:
            output_component_types:
            options:
            continue_on_batch_error:

        Returns:
        """
        self._batch_error = None
        is_batch = update_data is not None

        if update_data is not None:
            prepared_update = prepare_update_view(update_data)
            update_ptr = prepared_update.get_dataset_ptr()
            batch_size = prepared_update.get_info().batch_size()
        else:
            update_ptr = ConstDatasetPtr()
            batch_size = 1

        output_data = self._construct_output(
            output_component_types=output_component_types,
            calculation_type=calculation_type,
            symmetric=symmetric,
            is_batch=is_batch,
            batch_size=batch_size,
        )
        prepared_result = prepare_output_view(
            output_data=output_data,
            output_type=get_output_type(calculation_type=calculation_type, symmetric=symmetric),
        )

        # run calculation
        pgc.calculate(
            # model and options
            self._model,
            options.opt,
            output_data=prepared_result.get_dataset_ptr(),
            update_data=update_ptr,
        )

        self._handle_errors(continue_on_batch_error=continue_on_batch_error, batch_size=batch_size)

        return output_data

    def _calculate_power_flow(
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
        experimental_features: Union[_ExperimentalFeatures, str] = _ExperimentalFeatures.disabled,
    ):
        calculation_type = CalculationType.power_flow
        options = self._options(
            calculation_type=calculation_type,
            symmetric=symmetric,
            error_tolerance=error_tolerance,
            max_iterations=max_iterations,
            calculation_method=calculation_method,
            threading=threading,
            experimental_features=experimental_features,
        )
        return self._calculate_impl(
            calculation_type=calculation_type,
            symmetric=symmetric,
            update_data=update_data,
            output_component_types=output_component_types,
            options=options,
            continue_on_batch_error=continue_on_batch_error,
        )

    def _calculate_state_estimation(
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
        experimental_features: Union[_ExperimentalFeatures, str] = _ExperimentalFeatures.disabled,
    ) -> Dict[str, np.ndarray]:
        calculation_type = CalculationType.state_estimation
        options = self._options(
            calculation_type=calculation_type,
            symmetric=symmetric,
            error_tolerance=error_tolerance,
            max_iterations=max_iterations,
            calculation_method=calculation_method,
            threading=threading,
            experimental_features=experimental_features,
        )
        return self._calculate_impl(
            calculation_type=calculation_type,
            symmetric=symmetric,
            update_data=update_data,
            output_component_types=output_component_types,
            options=options,
            continue_on_batch_error=continue_on_batch_error,
        )

    def _calculate_short_circuit(
        self,
        *,
        calculation_method: Union[CalculationMethod, str] = CalculationMethod.iec60909,
        update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]] = None,
        threading: int = -1,
        output_component_types: Optional[Union[Set[str], List[str]]] = None,
        continue_on_batch_error: bool = False,
        short_circuit_voltage_scaling: Union[ShortCircuitVoltageScaling, str] = ShortCircuitVoltageScaling.maximum,
        experimental_features: Union[_ExperimentalFeatures, str] = _ExperimentalFeatures.disabled,
    ) -> Dict[str, np.ndarray]:
        calculation_type = CalculationType.short_circuit
        symmetric = False

        options = self._options(
            calculation_type=calculation_type,
            symmetric=symmetric,
            calculation_method=calculation_method,
            threading=threading,
            short_circuit_voltage_scaling=short_circuit_voltage_scaling,
            experimental_features=experimental_features,
        )
        return self._calculate_impl(
            calculation_type=calculation_type,
            symmetric=symmetric,
            update_data=update_data,
            output_component_types=output_component_types,
            options=options,
            continue_on_batch_error=continue_on_batch_error,
        )

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
            symmetric (bool, optional): Whether to perform a three-phase symmetric calculation.

                - True: Three-phase symmetric calculation, even for asymmetric loads/generations (Default).
                - False: Three-phase asymmetric calculation.
            error_tolerance (float, optional): Error tolerance for voltage in p.u., applicable only when the
                calculation method is iterative.
            max_iterations (int, optional): Maximum number of iterations, applicable only when the calculation method
                is iterative.
            calculation_method (an enumeration or string): The calculation method to use.

                - newton_raphson: Use Newton-Raphson iterative method (default).
                - linear: Use linear method.
            update_data (dict, optional):
                None: Calculate power flow once with the current model attributes.
                Or a dictionary for batch calculation with batch update.

                    - key: Component type name to be updated in batch.
                    - value:

                        - For homogeneous update batch (a 2D numpy structured array):

                            - Dimension 0: Each batch.
                            - Dimension 1: Each updated element per batch for this component type.
                        - For inhomogeneous update batch (a dictionary containing two keys):

                            - indptr: A 1D numpy int64 array with length n_batch + 1. Given batch number k, the
                              update array for this batch is data[indptr[k]:indptr[k + 1]]. This is the concept of
                              compressed sparse structure.
                              https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.csr_matrix.html
                            - data: 1D numpy structured array in flat.
            threading (int, optional): Applicable only for batch calculation.

                - < 0: Sequential
                - = 0: Parallel, use number of hardware threads
                - > 0: Specify number of parallel threads
            output_component_types ({set, list}, optional): List or set of component types you want to be present in
                the output dict. By default, all component types will be in the output.
            continue_on_batch_error (bool, optional): If the program continues (instead of throwing error) if some
                scenarios fail.

        Returns:
            Dictionary of results of all components.

                - key: Component type name to be updated in batch.
                - value:

                    - For single calculation: 1D numpy structured array for the results of this component type.
                    - For batch calculation: 2D numpy structured array for the results of this component type.

                        - Dimension 0: Each batch.
                        - Dimension 1: The result of each element for this component type.

        Raises:
            Exception: In case an error in the core occurs, an exception will be thrown.
        """
        return self._calculate_power_flow(
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
        """
        Calculate state estimation once with the current model attributes.
        Or calculate in batch with the given update dataset in batch.

        Args:
            symmetric (bool, optional): Whether to perform a three-phase symmetric calculation.

                - True: Three-phase symmetric calculation, even for asymmetric loads/generations (Default).
                - False: Three-phase asymmetric calculation.
            error_tolerance (float, optional): error tolerance for voltage in p.u., only applicable when the
                calculation method is iterative.
            max_iterations (int, optional): Maximum number of iterations, applicable only when the calculation method
                is iterative.
            calculation_method (an enumeration): Use iterative linear method.
            update_data (dict, optional):
                None: Calculate state estimation once with the current model attributes.
                Or a dictionary for batch calculation with batch update.

                    - key: Component type name to be updated in batch.
                    - value:

                        - For homogeneous update batch (a 2D numpy structured array):

                            - Dimension 0: Each batch.
                            - Dimension 1: Each updated element per batch for this component type.
                        - For inhomogeneous update batch (a dictionary containing two keys):

                            - indptr: A 1D numpy int64 array with length n_batch + 1. Given batch number k, the
                              update array for this batch is data[indptr[k]:indptr[k + 1]]. This is the concept of
                              compressed sparse structure.
                              https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.csr_matrix.html
                            - data: 1D numpy structured array in flat.
            threading (int, optional): Applicable only for batch calculation.

                - < 0: Sequential
                - = 0: Parallel, use number of hardware threads
                - > 0: Specify number of parallel threads
            output_component_types ({set, list}, optional): List or set of component types you want to be present in
                the output dict. By default, all component types will be in the output.
            continue_on_batch_error (bool, optional): If the program continues (instead of throwing error) if some
                scenarios fail.

        Returns:
            Dictionary of results of all components.

                - key: Component type name to be updated in batch.
                - value:

                    - For single calculation: 1D numpy structured array for the results of this component type.
                    - For batch calculation: 2D numpy structured array for the results of this component type.

                        - Dimension 0: Each batch.
                        - Dimension 1: The result of each element for this component type.

        Raises:
            Exception: In case an error in the core occurs, an exception will be thrown.
        """
        return self._calculate_state_estimation(
            symmetric=symmetric,
            error_tolerance=error_tolerance,
            max_iterations=max_iterations,
            calculation_method=calculation_method,
            update_data=update_data,
            threading=threading,
            output_component_types=output_component_types,
            continue_on_batch_error=continue_on_batch_error,
        )

    def calculate_short_circuit(
        self,
        *,
        calculation_method: Union[CalculationMethod, str] = CalculationMethod.iec60909,
        update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]] = None,
        threading: int = -1,
        output_component_types: Optional[Union[Set[str], List[str]]] = None,
        continue_on_batch_error: bool = False,
        short_circuit_voltage_scaling: Union[ShortCircuitVoltageScaling, str] = ShortCircuitVoltageScaling.maximum,
    ) -> Dict[str, np.ndarray]:
        """
        Calculate a short circuit once with the current model attributes.
        Or calculate in batch with the given update dataset in batch

        Args:
            calculation_method (an enumeration): Use the iec60909 standard.
            update_data:
                None: calculate a short circuit once with the current model attributes.
                Or a dictionary for batch calculation with batch update

                    - key: Component type name to be updated in batch
                    - value:

                        - For homogeneous update batch (a 2D numpy structured array):

                            - Dimension 0: each batch
                            - Dimension 1: each updated element per batch for this component type
                        - For inhomogeneous update batch (a dictionary containing two keys):

                            - indptr: A 1D numpy int64 array with length n_batch + 1. Given batch number k, the
                              update array for this batch is data[indptr[k]:indptr[k + 1]]. This is the concept of
                              compressed sparse structure.
                              https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.csr_matrix.html
                            - data: 1D numpy structured array in flat.
            threading (int, optional): Applicable only for batch calculation.

                - < 0: Sequential
                - = 0: Parallel, use number of hardware threads
                - > 0: Specify number of parallel threads
            output_component_types ({set, list}, optional):
                List or set of component types you want to be present in the output dict.
                By default, all component types will be in the output.
            continue_on_batch_error (bool, optional):
                If the program continues (instead of throwing error) if some scenarios fail.
            short_circuit_voltage_scaling ({ShortCircuitVoltageSaling, str}, optional):
                Whether to use the maximum or minimum voltage scaling.
                By default, the maximum voltage scaling is used to calculate the short circuit.

        Returns:
            Dictionary of results of all components.

                - key: Component type name to be updated in batch.
                - value:

                    - For single calculation: 1D numpy structured array for the results of this component type.
                    - For batch calculation: 2D numpy structured array for the results of this component type.

                        - Dimension 0: Each batch.
                        - Dimension 1: The result of each element for this component type.
        Raises:
            Exception: In case an error in the core occurs, an exception will be thrown.
        """
        return self._calculate_short_circuit(
            calculation_method=calculation_method,
            update_data=update_data,
            threading=threading,
            output_component_types=output_component_types,
            continue_on_batch_error=continue_on_batch_error,
            short_circuit_voltage_scaling=short_circuit_voltage_scaling,
        )

    def __del__(self):
        pgc.destroy_model(self._model_ptr)
