# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
Loader for the dynamic library
"""

import platform
from ctypes import CDLL, POINTER, c_char_p, c_double, c_size_t, c_void_p
from inspect import signature
from pathlib import Path
from typing import Callable, List

from power_grid_model.core.index_integer import IdC, IdxC

# integer index
IdxPtr = POINTER(IdxC)
IdxDoublePtr = POINTER(IdxPtr)
IDPtr = POINTER(IdC)
# double pointer to char
CharDoublePtr = POINTER(c_char_p)
# double pointer to void
VoidDoublePtr = POINTER(c_void_p)

# functions with size_t return
_FUNC_SIZE_T_RES = {"meta_class_size", "meta_class_alignment", "meta_attribute_offset"}
_ARGS_TYPE_MAPPING = {str: c_char_p, int: IdxC, float: c_double}

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


def _load_core() -> CDLL:
    """

    Returns: DLL/SO object

    """
    if platform.system() == "Windows":
        dll_file = "_power_grid_core.dll"
    else:
        dll_file = "_power_grid_core.so"
    cdll = CDLL(str(Path(__file__).parent / dll_file))
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
    py_argnames = list(sig.parameters.keys())[1:]  # pylint: disable=unused-variable
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
    def cbind_func(self, *args):
        if "destroy" in name:
            c_inputs = []
        else:
            c_inputs = [self._handle]  # pylint: disable=protected-access
        for arg, arg_type in zip(args, c_argtypes):
            if arg_type == c_char_p:
                c_inputs.append(arg.encode())
            else:
                c_inputs.append(arg)
        # call
        res = getattr(_CDLL, f"PGM_{name}")(*c_inputs)
        # convert to string for c_char_p
        if c_restype == c_char_p:
            res = res.decode()
        return res

    return cbind_func


class WrapperFunc:
    """
    Functor to wrap the C function
    """

    # pylint: disable=too-many-arguments
    def __init__(self, handle: HandlePtr, name: str, c_argtypes: List, c_restype):
        """

        Args:
            handle: pointer to handle
            name: name of the function
            c_argtypes: list of C argument types
            c_restype: C return type
        """
        self._cfunc = getattr(_CDLL, f"PGM_{name}")
        self._handle = handle
        self._name = name
        self._c_argtypes = c_argtypes
        self._c_restype = c_restype

    def __call__(self, *args):
        if "destroy" in self._name:
            c_inputs = []
        else:
            c_inputs = [self._handle]
        for arg, arg_type in zip(args, self._c_argtypes):
            if arg_type == c_char_p:
                c_inputs.append(arg.encode())
            else:
                c_inputs.append(arg)
        # call
        res = self._cfunc(*c_inputs)
        # convert to string for c_char_p
        if self._c_restype == c_char_p:
            res = res.decode()
        return res


# pylint: disable=too-many-arguments
# pylint: disable=missing-function-docstring
# pylint: disable=too-many-public-methods
class PowerGridCore:
    """
    DLL caller
    """

    _handle: HandlePtr

    def __new__(cls, *args, **kwargs):
        instance = super().__new__(cls, *args, **kwargs)
        instance._handle = _CDLL.PGM_create_handle()
        return instance

    def __init__(self):
        for name, function in PowerGridCore.__annotations__.items():  # pylint: disable=E1101
            if name.startswith("_"):
                continue
            # get and convert types
            py_argtypes = function.__args__[:-1]
            py_restype = function.__args__[-1]
            c_argtypes = [_ARGS_TYPE_MAPPING.get(x, x) for x in py_argtypes]
            c_restype = _ARGS_TYPE_MAPPING.get(py_restype, py_restype)
            if c_restype == IdxC and name in _FUNC_SIZE_T_RES:
                c_restype = c_size_t
            # bug in Python 3.10 https://bugs.python.org/issue43208
            if id(c_restype) == id(type(None)):
                c_restype = None
            # set argument in dll
            # mostly with handle pointer, except destroy function
            is_destroy_func = "destroy" in name
            if is_destroy_func:
                getattr(_CDLL, f"PGM_{name}").argtypes = c_argtypes
            else:
                getattr(_CDLL, f"PGM_{name}").argtypes = [HandlePtr] + c_argtypes
            getattr(_CDLL, f"PGM_{name}").restype = c_restype
            # set wrapper functor to instance
            setattr(
                self,
                name,
                WrapperFunc(handle=self._handle, name=name, c_argtypes=c_argtypes, c_restype=c_restype),
            )

    def __del__(self):
        _CDLL.PGM_destroy_handle(self._handle)

    # not copyable
    def __copy__(self):
        raise NotImplementedError("Class not copyable")

    def __deepcopy__(self, memodict):
        raise NotImplementedError("class not copyable")

    @make_c_binding
    def error_code(self) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def error_message(self) -> str:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def n_failed_scenarios(self) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def failed_scenarios(self) -> IdxPtr:  # type: ignore[empty-body, valid-type]
        pass

    @make_c_binding
    def batch_errors(self) -> CharDoublePtr:  # type: ignore[empty-body, valid-type]
        pass

    @make_c_binding
    def clear_error(self) -> None:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def is_batch_independent(self) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def is_batch_cache_topology(self) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def meta_n_datasets(self) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def meta_dataset_name(self, idx: int) -> str:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def meta_n_components(self, dataset: str) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def meta_component_name(self, dataset: str, idx: int) -> str:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def meta_component_alignment(self, dataset: str, component: str) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def meta_component_size(self, dataset: str, component: str) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def meta_n_attributes(self, dataset: str, component: str) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def meta_attribute_name(self, dataset: str, component: str, idx: int) -> str:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def meta_attribute_ctype(self, dataset: str, component: str, attribute: str) -> str:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def meta_attribute_offset(self, dataset: str, component: str, attribute: str) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def is_little_endian(self) -> int:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def create_options(self) -> OptionsPtr:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def destroy_options(self, opt: OptionsPtr) -> None:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def set_calculation_type(self, opt: OptionsPtr, calculation_type: int) -> None:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def set_calculation_method(self, opt: OptionsPtr, method: int) -> None:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def set_symmetric(self, opt: OptionsPtr, sym: int) -> None:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def set_err_tol(self, opt: OptionsPtr, err_tol: float) -> None:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def set_max_iter(self, opt: OptionsPtr, max_iter: int) -> None:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def set_threading(self, opt: OptionsPtr, threading: int) -> None:  # type: ignore[empty-body]
        pass

    @make_c_binding
    def create_model(  # type: ignore[empty-body]
        self,
        system_frequency: float,
        n_components: int,
        components: CharDoublePtr,  # type: ignore[valid-type]
        component_sizes: IdxPtr,  # type: ignore[valid-type]
        input_data: VoidDoublePtr,  # type: ignore[valid-type]
    ) -> ModelPtr:
        pass

    @make_c_binding
    def update_model(  # type: ignore[empty-body]
        self,
        model: ModelPtr,
        n_components: int,
        components: CharDoublePtr,  # type: ignore[valid-type]
        component_sizes: IdxPtr,  # type: ignore[valid-type]
        update_data: VoidDoublePtr,  # type: ignore[valid-type]
    ) -> None:
        pass

    @make_c_binding
    def copy_model(  # type: ignore[empty-body]
        self,
        model: ModelPtr,
    ) -> ModelPtr:
        pass

    @make_c_binding
    def get_indexer(  # type: ignore[empty-body]
        self,
        model: ModelPtr,
        component: str,
        size: int,
        ids: IDPtr,  # type: ignore[valid-type]
        indexer: IdxPtr,  # type: ignore[valid-type]
    ) -> None:
        pass

    @make_c_binding
    def destroy_model(  # type: ignore[empty-body]
        self,
        model: ModelPtr,
    ) -> None:
        pass

    @make_c_binding
    def calculate(  # type: ignore[empty-body]
        self,
        model: ModelPtr,
        opt: OptionsPtr,
        # output
        n_output_components: int,
        output_components: CharDoublePtr,  # type: ignore[valid-type]
        output_data: VoidDoublePtr,  # type: ignore[valid-type]
        # update
        n_scenarios: int,
        n_update_components: int,
        update_components: CharDoublePtr,  # type: ignore[valid-type]
        n_component_elements_per_scenario: IdxPtr,  # type: ignore[valid-type]
        indptrs_per_component: IdxDoublePtr,  # type: ignore[valid-type]
        update_data: VoidDoublePtr,  # type: ignore[valid-type]
    ) -> None:
        pass


# make one instance
power_grid_core = PowerGridCore()
