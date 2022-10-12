# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

# cython: language_level = 3
# cython: binding=True
# distutils: language = c++

import ctypes
from typing import Any, Dict, Optional, Union

import numpy as np

from .enum import CalculationMethod

cimport numpy as cnp
from cython.operator cimport dereference as deref
from libc.stdint cimport int8_t
from libcpp cimport bool
from libcpp.map cimport map
from libcpp.string cimport string
from libcpp.vector cimport vector

# idx and id types
from libc.stdint cimport int64_t as idx_t # isort: skip
cdef np_idx_t = np.int64
from libc.stdint cimport int32_t as id_t # isort: skip
cdef np_id_t = np.int32

cdef VALIDATOR_MSG = "Try validate_input_data() or validate_batch_data() to validate your data."

cdef extern from "power_grid_model/auxiliary/meta_data_gen.hpp" namespace "power_grid_model::meta_data":
    cppclass DataAttribute:
        string name
        string numpy_type
        vector[size_t] dims
        size_t offset

    cppclass MetaData:
        string name
        size_t size
        size_t alignment
        vector[DataAttribute] attributes

    bool is_little_endian()
    map[string, map[string, MetaData]] meta_data()

cdef _endianness = '<' if is_little_endian() else '>'
cdef _nan_value_map = {
    f'{_endianness}f8': np.nan,
    f'{_endianness}i4': np.iinfo(f'{_endianness}i4').min,
    f'{_endianness}i1': np.iinfo(f'{_endianness}i1').min
}

cpdef _generate_meta_data():
    py_meta_data = {}
    cdef map[string, map[string, MetaData]] all_meta_data = meta_data()
    cdef map[string, MetaData] input_meta_data = all_meta_data[b'input']
    cdef map[string, MetaData] sym_output_meta_data = all_meta_data[b'sym_output']
    cdef map[string, MetaData] asym_output_meta_data = all_meta_data[b'asym_output']
    cdef map[string, MetaData] update_meta_data = all_meta_data[b'update']
    py_meta_data['input'] = _generate_cluster_meta_data(input_meta_data)
    py_meta_data['sym_output'] = _generate_cluster_meta_data(sym_output_meta_data)
    py_meta_data['asym_output'] = _generate_cluster_meta_data(asym_output_meta_data)
    py_meta_data['update'] = _generate_cluster_meta_data(update_meta_data)
    return py_meta_data

cdef _generate_cluster_meta_data(map[string, MetaData] & cpp_meta_data):
    py_meta_data = {}
    for map_entry in cpp_meta_data:
        name = map_entry.first.decode()
        numpy_dtype_dict = _generate_component_meta_data(map_entry.second)
        dtype = np.dtype({k: v for k, v in numpy_dtype_dict.items() if k != 'nans'})
        if dtype.alignment != map_entry.second.alignment:
            raise TypeError(f'Aligment mismatch for component type: "{name}" !')
        single_meta_data = {
            'dtype': dtype,
            'dtype_dict': numpy_dtype_dict,
            'nans': {x: y for x, y in zip(numpy_dtype_dict['names'], numpy_dtype_dict['nans'])}
        }
        # get single nan scalar
        nan_scalar = np.zeros(1, dtype=single_meta_data['dtype'])
        for k, v in single_meta_data['nans'].items():
            nan_scalar[k] = v
        single_meta_data['nan_scalar'] = nan_scalar
        py_meta_data[name] = single_meta_data
    return py_meta_data

cdef _generate_component_meta_data(MetaData & cpp_component_meta_data):
    numpy_dtype_dict = {
        'names': [],
        'formats': [],
        'offsets': [],
        'itemsize': int(cpp_component_meta_data.size),
        'aligned': True,
        'nans': []
    }
    cdef DataAttribute attr
    for attr in cpp_component_meta_data.attributes:
        field_name = attr.name.decode()
        scalar_type = _endianness + attr.numpy_type.decode()
        # for multi dimension entry
        if attr.dims.size() > 0:
            dims = list(attr.dims)
            dims = tuple(dims)
            field_format = str(dims) + scalar_type
        else:
            field_format = scalar_type
        offset = int(attr.offset)
        numpy_dtype_dict['names'].append(field_name)
        numpy_dtype_dict['formats'].append(field_format)
        numpy_dtype_dict['offsets'].append(offset)
        numpy_dtype_dict['nans'].append(_nan_value_map[scalar_type])
    return numpy_dtype_dict

cdef extern from "power_grid_model/auxiliary/dataset.hpp" namespace "power_grid_model":
    cppclass MutableDataPointer:
        MutableDataPointer()
        MutableDataPointer(void * ptr, const idx_t * indptr, idx_t size)
    cppclass ConstDataPointer:
        ConstDataPointer()
        ConstDataPointer(const void * ptr, const idx_t * indptr, idx_t size)

cdef extern from "power_grid_model/main_model.hpp" namespace "power_grid_model":
    cppclass CalculationMethodCPP "::power_grid_model::CalculationMethod":
        pass
    cppclass BatchParameter:
        bool independent
        bool cache_topology
    cppclass MainModel:
        map[string, idx_t] all_component_count()
        BatchParameter calculate_sym_power_flow "calculate_power_flow<true>"(
            double error_tolerance,
            idx_t max_iterations,
            CalculationMethodCPP calculation_method,
            const map[string, MutableDataPointer] & result_data,
            const map[string, ConstDataPointer] & update_data,
            idx_t threading
        ) except+
        BatchParameter calculate_asym_power_flow "calculate_power_flow<false>"(
            double error_tolerance,
            idx_t max_iterations,
            CalculationMethodCPP calculation_method,
            const map[string, MutableDataPointer] & result_data,
            const map[string, ConstDataPointer] & update_data,
            idx_t threading
        ) except+
        BatchParameter calculate_sym_state_estimation "calculate_state_estimation<true>"(
            double error_tolerance,
            idx_t max_iterations,
            CalculationMethodCPP calculation_method,
            const map[string, MutableDataPointer] & result_data,
            const map[string, ConstDataPointer] & update_data,
            idx_t threading
        ) except+
        BatchParameter calculate_asym_state_estimation "calculate_state_estimation<false>"(
            double error_tolerance,
            idx_t max_iterations,
            CalculationMethodCPP calculation_method,
            const map[string, MutableDataPointer] & result_data,
            const map[string, ConstDataPointer] & update_data,
            idx_t threading
        ) except+
        void update_component(
            const map[string, ConstDataPointer] & update_data,
            idx_t pos
        ) except+
        void get_indexer(
            const string& component_type, 
            const id_t* id_begin, 
            idx_t size, 
            idx_t* indexer_begin) except+

cdef extern from "<optional>":
    cppclass OptionalMainModel "::std::optional<::power_grid_model::MainModel>":
        OptionalMainModel()
        bool has_value()
        MainModel & value()
        MainModel & emplace(const MainModel & ) except+
        MainModel & emplace(
            double system_frequency,
            const map[string, ConstDataPointer] & input_data,
            idx_t pos) except+

# internally used meta data, to prevent modification
cdef _power_grid_meta_data = _generate_meta_data()


cdef map[string, ConstDataPointer] generate_const_ptr_map(data: Dict[str, Dict[str, Any]]) except *:
    cdef map[string, ConstDataPointer] result
    cdef cnp.ndarray data_arr
    cdef cnp.ndarray indptr_arr
    for k, v in data.items():
        data_arr = v['data']
        indptr_arr = v['indptr']
        result[k.encode()] = ConstDataPointer(
            cnp.PyArray_DATA(data_arr), < const idx_t*>cnp.PyArray_DATA(indptr_arr),
            v['batch_size'])
    return result

cdef map[string, MutableDataPointer] generate_ptr_map(data: Dict[str, Dict[str, Any]]) except *:
    cdef map[string, MutableDataPointer] result
    cdef cnp.ndarray data_arr
    cdef cnp.ndarray indptr_arr
    for k, v in data.items():
        data_arr = v['data']
        indptr_arr = v['indptr']
        result[k.encode()] = MutableDataPointer(
            cnp.PyArray_DATA(data_arr), < const idx_t * > cnp.PyArray_DATA(indptr_arr),
            v['batch_size'])
    return result

cdef _prepare_cpp_array(data_type: str,
                        array_dict: Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]):
    """
    prepare array for cpp pointers
    Args:
        data_type: input, update, or symmetric/asymmetric output
        array_dict:
            key: component type
            value:
                data array: can be 1D or 2D (in batches)
                or
                dict with
                    key:
                        data -> data array in flat for batches
                        indptr -> index pointer for variable length input
    Returns:
        dict of
            key: component type
            value: dict of following entries:
                data: pointer object containing the data
                indptr: pointer object containing the index pointer
                data_ptr: int representation of data pointer
                indptr_ptr: int representation of indptr pointer
                batch_size: size of batches (can be one)
    """
    schema = _power_grid_meta_data[data_type]
    return_dict = {}
    for component_name, v in array_dict.items():
        if component_name not in schema:
            continue
        # no indptr, create one
        if isinstance(v, np.ndarray):
            data = v
            ndim = v.ndim
            if ndim == 1:
                indptr = np.array([0, v.size], dtype=np_idx_t)
                batch_size = 1
            elif ndim == 2:  # (n_batch, n_component)
                indptr = np.arange(v.shape[0] + 1, dtype=np_idx_t) * v.shape[1]
                batch_size = v.shape[0]
            else:
                raise ValueError(f"Array can only be 1D or 2D. {VALIDATOR_MSG}")
        # with indptr
        else:
            data = v['data']
            indptr = v['indptr']
            batch_size = indptr.size - 1
            if data.ndim != 1:
                raise ValueError(f"Data array can only be 1D. {VALIDATOR_MSG}")
            if indptr.ndim != 1:
                raise ValueError(f"indptr can only be 1D. {VALIDATOR_MSG}")
            if indptr[0] != 0 or indptr[-1] != data.size:
                raise ValueError(
                    f"indptr should start from zero and end at size of data array. {VALIDATOR_MSG}")
            if np.any(np.diff(indptr) < 0):
                raise ValueError(f"indptr should be increasing. {VALIDATOR_MSG}")
        # convert array
        data = np.ascontiguousarray(data, dtype=schema[component_name]['dtype'])
        indptr = np.ascontiguousarray(indptr, dtype=np_idx_t)
        return_dict[component_name] = {
            'data': data,
            'indptr': indptr,
            'batch_size': batch_size
        }
    return return_dict

cdef class PowerGridModel:
    cdef OptionalMainModel _main_model
    cdef readonly bool independent  # all update datasets consists of exactly the same components
    cdef readonly bool cache_topology  # there are no changes in topology (branch, source) in the update datasets

    cdef MainModel * _get_model(self) except *:
        """
        get pointer to main model
        if model does not exist, raise exception
        This prevents undefined behaviour if the model is empty

        Returns:
            pointer to main model
        """
        if not self._main_model.has_value():
            raise TypeError("You have an empty instance of PowerGridModel!")
        return & (self._main_model.value())

    def __init__(self,
                 input_data: Dict[str, np.ndarray],
                 double system_frequency=50.0):
        """
        Initialize the model from an input data set.

        Args:
            input_data: input data dictionary
                key: component type name
                value: 1D numpy structured array for this component input
            system_frequency: frequency of the power system, default 50 Hz
        """
        cdef map[string, ConstDataPointer] input_set
        prepared_input = _prepare_cpp_array(data_type='input', array_dict=input_data)
        input_set = generate_const_ptr_map(prepared_input)
        self._main_model.emplace(system_frequency, input_set, 0)
        self.independent = False
        self.cache_topology = False

    def get_indexer(self, 
                    component_type: str, 
                    ids: np.ndarray):
        """
        Get array of indexers given array of ids for component type

        Args:
            component_type: type of component
            ids: array of ids
        
        Returns:
            array of inderxers, same shape as input array ids

        """
        cdef cnp.ndarray ids_c = np.ascontiguousarray(ids, dtype=np_id_t)
        cdef cnp.ndarray indexer = np.empty_like(ids_c, dtype=np_idx_t, order='C')
        cdef const id_t* id_begin = <const id_t*> cnp.PyArray_DATA(ids_c)
        cdef idx_t* indexer_begin = <idx_t*> cnp.PyArray_DATA(indexer)
        cdef idx_t size = ids.size
        # call c function
        self._get_model().get_indexer(component_type.encode(), id_begin, size, indexer_begin)
        return indexer

    def copy(self) -> PowerGridModel:
        """

        Copy the current model

        Returns:
            a copy of PowerGridModel
        """
        cdef PowerGridModel new_model = PowerGridModel.__new__(PowerGridModel)
        new_model._main_model.emplace(deref(self._get_model()))
        return new_model

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
        cdef map[string, ConstDataPointer] update_set
        prepared_update = _prepare_cpp_array(data_type='update', array_dict=update_data)
        update_set = generate_const_ptr_map(prepared_update)
        self._get_model().update_component(update_set, 0)

    cdef calculate(self,
                   calculation_type,
                   bool symmetric,
                   double error_tolerance,
                   idx_t max_iterations,
                   calculation_method: Union[CalculationMethod, str],
                   update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]],
                   idx_t threading
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

        Returns:

        """
        cdef map[string, MutableDataPointer] result_set
        cdef map[string, ConstDataPointer] update_set
        cdef int8_t calculation_method_int
        cdef CalculationMethodCPP calculation_method_cpp
        cdef BatchParameter batch_parameter

        if isinstance(calculation_method, str):
            calculation_method = getattr(CalculationMethod, calculation_method)
        calculation_method_int = calculation_method.value
        calculation_method_cpp = <CalculationMethodCPP> calculation_method_int

        if symmetric:
            output_type = 'sym_output'
        else:
            output_type = 'asym_output'

        # update data exist for batch calculation
        if update_data is not None:
            batch_calculation = True
        # no update dataset, create one batch with empty set
        else:
            batch_calculation = False
            update_data = {}

        # update set and n batch
        prepared_update = _prepare_cpp_array(data_type='update', array_dict=update_data)
        update_set = generate_const_ptr_map(prepared_update)
        if prepared_update:
            n_batch_vec = [x['batch_size'] for x in prepared_update.values()]
            if np.unique(n_batch_vec).size != 1:
                raise ValueError(
                    f"number of batches in the update set is not consistent across all components! {VALIDATOR_MSG}")
            n_batch = n_batch_vec[0]
        else:
            # empty dict, one time calculation
            n_batch = 1

        # create result dict
        result_dict = {}
        all_component_count = self.all_component_count

        # for power flow, there is no need for sensor output
        if calculation_type == 'power_flow':
            all_component_count = {
                k: v for k, v in all_component_count.items() if 'sensor' not in k
            }

        for name, count in all_component_count.items():
            # intialize array
            arr = initialize_array(output_type, name, (n_batch, count), empty=True)
            result_dict[name] = arr

        prepared_result = _prepare_cpp_array(data_type=output_type, array_dict=result_dict)
        result_set = generate_ptr_map(prepared_result)

        # run power flow or state estimation
        if calculation_type == 'power_flow':
            try:
                if symmetric:
                    batch_parameter = self._get_model().calculate_sym_power_flow(
                        error_tolerance, max_iterations, calculation_method_cpp, result_set, update_set,
                        threading
                    )
                else:
                    batch_parameter = self._get_model().calculate_asym_power_flow(
                        error_tolerance, max_iterations, calculation_method_cpp, result_set, update_set,
                        threading
                    )
            except RuntimeError as ex:
                raise RuntimeError(str(ex) + "\n" + VALIDATOR_MSG) from ex

        elif calculation_type == 'state_estimation':
            try:
                if symmetric:
                    batch_parameter = self._get_model().calculate_sym_state_estimation(
                        error_tolerance, max_iterations, calculation_method_cpp, result_set, update_set,
                        threading
                    )
                else:
                    batch_parameter = self._get_model().calculate_asym_state_estimation(
                        error_tolerance, max_iterations, calculation_method_cpp, result_set, update_set,
                        threading
                    )
            except RuntimeError as ex:
                raise RuntimeError(str(ex) + "\n" + VALIDATOR_MSG) from ex
        else:
            raise TypeError(f"Unknown calculation type {calculation_type}. Choose 'power_flow' or 'state_estimation'")

        # flatten array for normal calculation
        if not batch_calculation:
            result_dict = {k: v.ravel() for k, v in result_dict.items()}
        # batch parameters
        self.independent = batch_parameter.independent
        self.cache_topology = batch_parameter.cache_topology

        return result_dict

    def calculate_power_flow(self, *,
                             bool symmetric=True,
                             double error_tolerance=1e-8,
                             idx_t max_iterations=20,
                             calculation_method: Union[CalculationMethod, str] = CalculationMethod.newton_raphson,
                             update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]] = None,
                             idx_t threading=-1
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
        return self.calculate(
            'power_flow',
            symmetric=symmetric,
            error_tolerance=error_tolerance,
            max_iterations=max_iterations,
            calculation_method=calculation_method,
            update_data=update_data,
            threading=threading
        )

    def calculate_state_estimation(self, *,
                                   bool symmetric=True,
                                   double error_tolerance=1e-8,
                                   idx_t max_iterations=20,
                                   calculation_method: Union[CalculationMethod, str] = CalculationMethod.iterative_linear,
                                   update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]] = None,
                                   idx_t threading=-1
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
        return self.calculate(
            'state_estimation',
            symmetric=symmetric,
            error_tolerance=error_tolerance,
            max_iterations=max_iterations,
            calculation_method=calculation_method,
            update_data=update_data,
            threading=threading
        )

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
        all_component_count = {}
        cdef map[string, idx_t] cpp_count = self._get_model().all_component_count()
        for map_entry in cpp_count:
            all_component_count[map_entry.first.decode()] = map_entry.second
        return all_component_count

    def __copy__(self):
        return self.copy()


def initialize_array(data_type: str, component_type: str, shape: Union[tuple, int], empty=False):
    """
    Initializes an array for use in Power Grid Model calculations

    Args:
        data_type: input, update, sym_output, or asym_output
        component_type: one component type, e.g. node
        shape: shape of initialization
            integer, it is a 1-dimensional array
            tuple, it is an N-dimensional (tuple.shape) array
        empty: if leave the memory block un-initialized

    Returns:
        np structured array with all entries as null value
    """
    if not isinstance(shape, tuple):
        shape = (shape, )
    if empty:
        return np.empty(
            shape=shape,
            dtype=_power_grid_meta_data[data_type][component_type]['dtype'],
            order='C'
        )
    else:
        return np.full(
            shape=shape,
            fill_value=_power_grid_meta_data[data_type][component_type]['nan_scalar'],
            dtype=_power_grid_meta_data[data_type][component_type]['dtype'],
            order='C'
        )


# external used meta data
power_grid_meta_data = _generate_meta_data()
