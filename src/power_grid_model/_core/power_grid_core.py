# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Loader for the dynamic library
"""

import os
import platform
from ctypes import CDLL, POINTER, c_char, c_char_p, c_double, c_size_t, c_void_p
from inspect import signature
from itertools import chain
from pathlib import Path
from typing import Callable

from power_grid_model._core.index_integer import IdC, IdxC

# integer index
IdxPtr = POINTER(IdxC)
"""Pointer to index."""
IdxDoublePtr = POINTER(IdxPtr)
"""Double pointer to index."""
IDPtr = POINTER(IdC)
"""Raw pointer to ids."""

# string data
CStr = c_char_p
"""Null terminated string."""
CStrPtr = POINTER(CStr)
"""Pointer to null terminated string."""
CharPtr = POINTER(c_char)
"""Raw pointer to char."""
CharDoublePtr = POINTER(CharPtr)
"""Double pointer to char."""

# raw data
VoidPtr = c_void_p
"""Raw void pointer."""
VoidDoublePtr = POINTER(c_void_p)
"""Double pointer to void."""

# functions with size_t return
_FUNC_SIZE_T_RES = {"meta_class_size", "meta_class_alignment", "meta_attribute_offset"}
_ARGS_TYPE_MAPPING = {bytes: CharPtr, str: CStr, int: IdxC, float: c_double}

# The c_void_p is extended only for type hinting and type checking; therefore no public methods are required.
# pylint: disable=too-few-public-methods


class HandlePtr(c_void_p):
    """
    Pointer to handle
    """


class OptionsPtr(c_void_p):
    """
    Pointer to option
    """


class ModelPtr(c_void_p):
    """
    Pointer to model
    """


class DatasetPtr(c_void_p):
    """
    Pointer to dataset
    """


class ConstDatasetPtr(c_void_p):
    """
    Pointer to writable dataset
    """


class MutableDatasetPtr(c_void_p):
    """
    Pointer to writable dataset
    """


class WritableDatasetPtr(c_void_p):
    """
    Pointer to writable dataset
    """


class DatasetInfoPtr(c_void_p):
    """
    Pointer to dataset info
    """


class ComponentPtr(c_void_p):
    """
    Pointer to component
    """


class AttributePtr(c_void_p):
    """
    Pointer to attribute
    """


class DeserializerPtr(c_void_p):
    """
    Pointer to deserializer
    """


class SerializerPtr(c_void_p):
    """
    Pointer to serializer
    """


def _load_core() -> CDLL:
    """

    Returns: DLL/SO object

    """
    # first try to find the DLL local
    if platform.system() == "Windows":
        dll_file = "_power_grid_core.dll"
    else:
        dll_file = "_power_grid_core.so"
    dll_path = Path(__file__).parent / dll_file

    # if local DLL is not found, try to find the DLL from conda environment
    if (not dll_path.exists()) and ("CONDA_PREFIX" in os.environ):
        if platform.system() == "Windows":
            dll_file = "power_grid_model_c.dll"
        elif platform.system() == "Darwin":
            dll_file = "libpower_grid_model_c.dylib"
        elif platform.system() == "Linux":
            dll_file = "libpower_grid_model_c.so"
        else:
            raise NotImplementedError(f"Unsupported platform: {platform.system()}")
        # the dll will be found through conda environment
        dll_path = Path(dll_file)

    cdll = CDLL(str(dll_path))
    # assign return types
    # handle
    cdll.PGM_create_handle.argtypes = []
    cdll.PGM_create_handle.restype = HandlePtr
    cdll.PGM_destroy_handle.argtypes = [HandlePtr]
    cdll.PGM_destroy_handle.restype = None
    return cdll


# load dll once
_CDLL: CDLL = _load_core()


def make_c_binding(func: Callable):
    """
    Descriptor to make the function to bind to C

    Args:
        func: method object from PowerGridCore

    Returns:
        Binded function

    """
    name = func.__name__
    sig = signature(func)

    # get and convert types, skip first argument, as it is self
    py_argnames = list(sig.parameters.keys())[1:]
    py_argtypes = [v.annotation for v in sig.parameters.values()][1:]
    py_restype = sig.return_annotation
    c_argtypes = [_ARGS_TYPE_MAPPING.get(x, x) for x in py_argtypes]
    c_restype = _ARGS_TYPE_MAPPING.get(py_restype, py_restype)
    if c_restype == IdxC and name in _FUNC_SIZE_T_RES:
        c_restype = c_size_t
    # set argument in dll
    # mostly with handle pointer, except destroy function
    is_destroy_func = "destroy" in name
    if is_destroy_func:
        getattr(_CDLL, f"PGM_{name}").argtypes = c_argtypes
    else:
        getattr(_CDLL, f"PGM_{name}").argtypes = [HandlePtr] + c_argtypes
    getattr(_CDLL, f"PGM_{name}").restype = c_restype

    # binding function
    def cbind_func(self, *args, **kwargs):
        if "destroy" in name:
            c_inputs = []
        else:
            c_inputs = [self._handle]  # pylint: disable=protected-access
        args = chain(args, (kwargs[key] for key in py_argnames[len(args) :]))
        for arg in args:
            if isinstance(arg, str):
                c_inputs.append(arg.encode())
            else:
                c_inputs.append(arg)

        # call
        res = getattr(_CDLL, f"PGM_{name}")(*c_inputs)
        # convert to string for CStr
        if c_restype == CStr:
            res = res.decode() if res is not None else ""
        return res

    return cbind_func


# pylint: disable=too-many-arguments
# pylint: disable=missing-function-docstring
# pylint: disable=too-many-public-methods
class PowerGridCore:
    """
    DLL caller
    """

    _handle: HandlePtr
    _instance: "PowerGridCore | None" = None

    # singleton of power grid core
    def __new__(cls, *args, **kwargs):
        if cls._instance is None:
            cls._instance = super().__new__(cls, *args, **kwargs)
            cls._instance._handle = _CDLL.PGM_create_handle()
        return cls._instance

    def __del__(self):
        _CDLL.PGM_destroy_handle(self._handle)

    # not copyable
    def __copy__(self):
        raise NotImplementedError("Class not copyable")

    def __deepcopy__(self, memodict):
        raise NotImplementedError("class not copyable")

    @make_c_binding
    def error_code(self) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def error_message(self) -> str:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def n_failed_scenarios(self) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def failed_scenarios(self) -> IdxPtr:  # type: ignore[empty-body, valid-type]  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def batch_errors(self) -> CStrPtr:  # type: ignore[empty-body, valid-type]  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def clear_error(self) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_n_datasets(self) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_get_dataset_by_idx(self, idx: int) -> DatasetPtr:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_dataset_name(self, dataset: DatasetPtr) -> str:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_n_components(self, dataset: DatasetPtr) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_get_component_by_idx(self, dataset: DatasetPtr, idx: int) -> ComponentPtr:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_component_name(self, component: ComponentPtr) -> str:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_component_alignment(self, component: ComponentPtr) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_component_size(self, component: ComponentPtr) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_n_attributes(self, component: ComponentPtr) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_get_attribute_by_idx(self, component: ComponentPtr, idx: int) -> AttributePtr:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_attribute_name(self, attribute: AttributePtr) -> str:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_attribute_ctype(self, attribute: AttributePtr) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def meta_attribute_offset(self, attribute: AttributePtr) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def is_little_endian(self) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def create_options(self) -> OptionsPtr:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def destroy_options(self, opt: OptionsPtr) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def set_calculation_type(self, opt: OptionsPtr, calculation_type: int) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def set_calculation_method(self, opt: OptionsPtr, method: int) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def set_tap_changing_strategy(
        self, opt: OptionsPtr, tap_changing_strategy: int
    ) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def set_short_circuit_voltage_scaling(
        self, opt: OptionsPtr, short_circuit_voltage_scaling: int
    ) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def set_experimental_features(
        self, opt: OptionsPtr, experimental_features: int
    ) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def set_symmetric(self, opt: OptionsPtr, sym: int) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def set_err_tol(self, opt: OptionsPtr, err_tol: float) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def set_max_iter(self, opt: OptionsPtr, max_iter: int) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def set_threading(self, opt: OptionsPtr, threading: int) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def create_model(  # type: ignore[empty-body]
        self,
        system_frequency: float,
        input_data: ConstDatasetPtr,  # type: ignore[valid-type]
    ) -> ModelPtr:
        pass  # pragma: no cover

    @make_c_binding
    def update_model(  # type: ignore[empty-body]
        self,
        model: ModelPtr,
        update_data: ConstDatasetPtr,  # type: ignore[valid-type]
    ) -> None:
        pass  # pragma: no cover

    @make_c_binding
    def copy_model(self, model: ModelPtr) -> ModelPtr:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def get_indexer(  # pylint: disable=too-many-positional-arguments
        self,
        model: ModelPtr,
        component: str,
        size: int,
        ids: IDPtr,  # type: ignore[valid-type]
        indexer: IdxPtr,  # type: ignore[valid-type]
    ) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def destroy_model(self, model: ModelPtr) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def calculate(  # type: ignore[empty-body]
        self,
        model: ModelPtr,
        opt: OptionsPtr,
        output_data: MutableDatasetPtr,  # type: ignore[valid-type]
        update_data: ConstDatasetPtr,  # type: ignore[valid-type]
    ) -> None:
        pass  # pragma: no cover

    @make_c_binding
    def dataset_info_name(self, info: DatasetInfoPtr) -> str:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_info_is_batch(self, info: DatasetInfoPtr) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_info_batch_size(self, info: DatasetInfoPtr) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_info_n_components(self, info: DatasetInfoPtr) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_info_component_name(self, info: DatasetInfoPtr, component_idx: int) -> str:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_info_elements_per_scenario(  # type: ignore[empty-body]
        self, info: DatasetInfoPtr, component_idx: int
    ) -> int:
        pass  # pragma: no cover

    @make_c_binding
    def dataset_info_total_elements(self, info: DatasetInfoPtr, component_idx: int) -> int:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_info_has_attribute_indications(  # type: ignore[empty-body]
        self, info: DatasetInfoPtr, component_idx: int
    ) -> int:
        pass  # pragma: no cover

    @make_c_binding
    def dataset_info_n_attribute_indications(  # type: ignore[empty-body]
        self, info: DatasetInfoPtr, component_idx: int
    ) -> int:
        pass  # pragma: no cover

    @make_c_binding
    def dataset_info_attribute_name(  # type: ignore[empty-body]
        self, info: DatasetInfoPtr, component_idx: int, attribute_idx: int
    ) -> str:
        pass  # pragma: no cover

    @make_c_binding
    def create_dataset_mutable(  # type: ignore[empty-body]
        self, dataset: str, is_batch: int, batch_size: int
    ) -> MutableDatasetPtr:
        pass  # pragma: no cover

    @make_c_binding
    def destroy_dataset_mutable(self, dataset: MutableDatasetPtr) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_mutable_add_buffer(  # type: ignore[empty-body]  # pylint: disable=too-many-positional-arguments
        self,
        dataset: MutableDatasetPtr,
        component: str,
        elements_per_scenario: int,
        total_elements: int,
        indptr: IdxPtr,  # type: ignore[valid-type]
        data: VoidPtr,  # type: ignore[valid-type]
    ) -> None:
        pass  # pragma: no cover

    @make_c_binding
    def dataset_mutable_add_attribute_buffer(
        self,
        dataset: MutableDatasetPtr,
        component: str,
        attribute: str,
        data: VoidPtr,  # type: ignore[valid-type]
    ) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_mutable_get_info(self, dataset: MutableDatasetPtr) -> DatasetInfoPtr:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def create_dataset_const_from_mutable(  # type: ignore[empty-body]
        self, mutable_dataset: MutableDatasetPtr
    ) -> ConstDatasetPtr:
        pass  # pragma: no cover

    @make_c_binding
    def destroy_dataset_const(self, dataset: ConstDatasetPtr) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_const_get_info(self, dataset: ConstDatasetPtr) -> DatasetInfoPtr:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_writable_get_info(self, dataset: WritableDatasetPtr) -> DatasetInfoPtr:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_writable_set_buffer(
        self,
        dataset: WritableDatasetPtr,
        component: str,
        indptr: IdxPtr,  # type: ignore[valid-type]
        data: VoidPtr,  # type: ignore[valid-type]
    ) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def dataset_writable_set_attribute_buffer(
        self,
        dataset: WritableDatasetPtr,
        component: str,
        attribute: str,
        data: VoidPtr,  # type: ignore[valid-type]
    ) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def create_deserializer_from_binary_buffer(  # type: ignore[empty-body]
        self, data: bytes, size: int, serialization_format: int
    ) -> DeserializerPtr:
        pass  # pragma: no cover

    @make_c_binding
    def create_deserializer_from_null_terminated_string(  # type: ignore[empty-body]
        self, data: str, serialization_format: int
    ) -> DeserializerPtr:
        pass  # pragma: no cover

    @make_c_binding
    def deserializer_get_dataset(self, deserializer: DeserializerPtr) -> WritableDatasetPtr:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def deserializer_parse_to_buffer(self, deserializer: DeserializerPtr) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def destroy_deserializer(self, deserializer: DeserializerPtr) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover

    @make_c_binding
    def create_serializer(  # type: ignore[empty-body]
        self, data: ConstDatasetPtr, serialization_format: int
    ) -> SerializerPtr:
        pass  # pragma: no cover

    @make_c_binding
    def serializer_get_to_binary_buffer(  # type: ignore[empty-body]
        self,
        serializer: SerializerPtr,
        use_compact_list: int,
        data: CharDoublePtr,  # type: ignore[valid-type]
        size: IdxPtr,  # type: ignore[valid-type]
    ) -> None:
        pass  # pragma: no cover

    @make_c_binding
    def serializer_get_to_zero_terminated_string(  # type: ignore[empty-body]
        self,
        serializer: SerializerPtr,
        use_compact_list: int,
        indent: int,
    ) -> str:
        pass  # pragma: no cover

    @make_c_binding
    def destroy_serializer(self, serializer: SerializerPtr) -> None:  # type: ignore[empty-body]
        pass  # pragma: no cover


# make one instance
power_grid_core = PowerGridCore()
