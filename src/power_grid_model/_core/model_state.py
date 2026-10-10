# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""Typed Python views of native power-flow calculation state."""

from ctypes import pointer
from dataclasses import dataclass
from enum import IntEnum
from typing import TYPE_CHECKING, cast

import numpy as np
from numpy.typing import NDArray

from power_grid_model._core.index_integer import IdNp, IdxNp
from power_grid_model._core.power_grid_core import (
    StateGroupViewC,
    StateJacobianViewC,
    StateNodalStateViewC,
    StateOutputPtr,
    StateScenarioViewC,
    get_power_grid_core,
)

if TYPE_CHECKING:
    from numpy.typing import DTypeLike


class ModelStateBusKind(IntEnum):
    """Bus mapping origin in a solver group."""

    input_node = 0
    synthetic_branch3 = 1


@dataclass(frozen=True)
class StateOutputRequest:
    y_bus: bool = False
    jacobian: bool = False
    nodal_state: bool = False


class _OwnedArray(np.ndarray):
    _state_owner: "_NativeStateOutput | None"

    def __array_finalize__(self, source):
        self._state_owner = getattr(source, "_state_owner", None)


class _NativeStateOutput:
    def __init__(self, pointer: StateOutputPtr):
        self.pointer = pointer

    def __del__(self):
        if self.pointer:
            get_power_grid_core().destroy_state_output(self.pointer)
            self.pointer = StateOutputPtr()


def _array(pointer, size: int, dtype: "DTypeLike", owner: _NativeStateOutput) -> NDArray:
    result = np.empty(0, dtype=dtype) if size == 0 else np.ctypeslib.as_array(pointer, shape=(size,))
    result = cast(_OwnedArray, result.view(_OwnedArray))
    result._state_owner = owner
    return result


@dataclass(frozen=True)
class ModelStateYBus:
    row_indptr: NDArray[np.int64]
    col_indices: NDArray[np.int64]
    admittance_real: NDArray[np.float64]
    admittance_imag: NDArray[np.float64]


@dataclass(frozen=True)
class ModelStateJacobianStructure:
    """Shared Jacobian sparsity pattern in the solver group's bus ordering."""

    row_indptr_lu: NDArray[np.int64]
    col_indices_lu: NDArray[np.int64]


@dataclass(frozen=True)
class ModelStateJacobian:
    h: NDArray[np.float64]
    n: NDArray[np.float64]
    m: NDArray[np.float64]
    l_block: NDArray[np.float64]
    iteration: int = 0


@dataclass(frozen=True)
class ModelStateNodalState:
    """Per-iteration bus-major voltages; asymmetric groups contain three phases per bus."""

    voltage_magnitude: NDArray[np.float64]
    voltage_angle: NDArray[np.float64]
    iteration: int


def _build_jacobian(
    view: StateJacobianViewC,
    owner: _NativeStateOutput,
) -> ModelStateJacobian:
    return ModelStateJacobian(
        h=_array(view.jacobian_h, view.n_jacobian_values, np.float64, owner),
        n=_array(view.jacobian_n, view.n_jacobian_values, np.float64, owner),
        m=_array(view.jacobian_m, view.n_jacobian_values, np.float64, owner),
        l_block=_array(view.jacobian_l, view.n_jacobian_values, np.float64, owner),
        iteration=view.iteration,
    )


def _build_nodal_state(view: StateNodalStateViewC, owner: _NativeStateOutput) -> ModelStateNodalState:
    return ModelStateNodalState(
        voltage_magnitude=_array(view.voltage_magnitude, view.n_voltage_values, np.float64, owner),
        voltage_angle=_array(view.voltage_angle, view.n_voltage_values, np.float64, owner),
        iteration=view.iteration,
    )


@dataclass(frozen=True)
class ModelStateGroupMapping:
    group: int
    n_bus: int
    is_symmetric: bool
    bus_user_indptr: NDArray[np.int64]
    bus_user_sequence: NDArray[np.int64]
    bus_user_id: NDArray[np.int32]
    bus_kind: NDArray[np.int8]
    origin_branch3_id: NDArray[np.int32]


@dataclass(frozen=True)
class ModelStateGroup:
    mapping: ModelStateGroupMapping
    y_bus: ModelStateYBus | None
    jacobian_structure: ModelStateJacobianStructure | None
    jacobians: tuple[ModelStateJacobian, ...] = ()
    nodal_states: tuple[ModelStateNodalState, ...] = ()


@dataclass(frozen=True)
class ModelState:
    y_bus_requested: bool
    jacobian_requested: bool
    nodal_state_requested: bool
    input_node_group: NDArray[np.int64]
    input_node_bus: NDArray[np.int64]
    input_node_id: NDArray[np.int32]
    groups: tuple[ModelStateGroup, ...]


def _build_scenario(owner: _NativeStateOutput, scenario_idx: int) -> ModelState | None:
    pgc = get_power_grid_core()
    scenario_view = StateScenarioViewC()
    pgc.state_output_get_scenario(owner.pointer, scenario_idx, pointer(scenario_view))
    if not scenario_view.has_state:
        return None

    input_node_group = _array(scenario_view.input_node_group, scenario_view.n_input_nodes, IdxNp, owner)
    input_node_bus = _array(scenario_view.input_node_bus, scenario_view.n_input_nodes, IdxNp, owner)
    input_node_id = _array(scenario_view.input_node_id, scenario_view.n_input_nodes, IdNp, owner)
    groups = []
    for group_idx in range(scenario_view.n_groups):
        view = StateGroupViewC()
        pgc.state_output_get_group(owner.pointer, scenario_idx, group_idx, pointer(view))
        mapping = ModelStateGroupMapping(
            group=view.group,
            n_bus=view.n_bus,
            is_symmetric=bool(view.is_symmetric),
            bus_user_indptr=_array(view.bus_user_indptr, view.n_bus + 1, IdxNp, owner),
            bus_user_sequence=_array(view.bus_user_sequence, view.n_user_node_refs, IdxNp, owner),
            bus_user_id=_array(view.bus_user_id, view.n_user_node_refs, IdNp, owner),
            bus_kind=_array(view.bus_kind, view.n_bus, np.int8, owner),
            origin_branch3_id=_array(view.origin_branch3_id, view.n_bus, IdNp, owner),
        )
        y_bus = None
        if view.has_y_bus:
            y_bus = ModelStateYBus(
                row_indptr=_array(view.y_bus_row_indptr, view.n_bus + 1, IdxNp, owner),
                col_indices=_array(view.y_bus_col_indices, view.y_bus_nnz, IdxNp, owner),
                admittance_real=_array(view.admittance_real, view.n_admittance_values, np.float64, owner),
                admittance_imag=_array(view.admittance_imag, view.n_admittance_values, np.float64, owner),
            )
        jacobians: tuple[ModelStateJacobian, ...] = ()
        nodal_states: tuple[ModelStateNodalState, ...] = ()
        jacobian_structure = None
        if view.has_jacobians:
            jacobian_row_indptr = _array(view.jacobian_row_indptr, view.n_bus + 1, IdxNp, owner)
            jacobian_col_indices = _array(view.jacobian_col_indices, view.jacobian_nnz, IdxNp, owner)
            jacobian_structure = ModelStateJacobianStructure(
                row_indptr_lu=jacobian_row_indptr,
                col_indices_lu=jacobian_col_indices,
            )
            jacobians_items = []
            for jacobian_idx in range(view.n_jacobians):
                jacobian_view = StateJacobianViewC()
                pgc.state_output_get_jacobian(
                    owner.pointer, scenario_idx, group_idx, jacobian_idx, pointer(jacobian_view)
                )
                jacobians_items.append(_build_jacobian(jacobian_view, owner))
            jacobians = tuple(jacobians_items)
        nodal_states_items = []
        for nodal_state_idx in range(view.n_nodal_states):
            nodal_state_view = StateNodalStateViewC()
            pgc.state_output_get_nodal_state(
                owner.pointer, scenario_idx, group_idx, nodal_state_idx, pointer(nodal_state_view)
            )
            nodal_states_items.append(_build_nodal_state(nodal_state_view, owner))
        nodal_states = tuple(nodal_states_items)
        groups.append(
            ModelStateGroup(
                mapping=mapping,
                y_bus=y_bus,
                jacobian_structure=jacobian_structure,
                jacobians=jacobians,
                nodal_states=nodal_states,
            )
        )

    return ModelState(
        y_bus_requested=bool(scenario_view.y_bus_requested),
        jacobian_requested=bool(scenario_view.jacobian_requested),
        nodal_state_requested=bool(scenario_view.nodal_state_requested),
        input_node_group=input_node_group,
        input_node_bus=input_node_bus,
        input_node_id=input_node_id,
        groups=tuple(groups),
    )


def build_model_states(pointer: StateOutputPtr) -> list[ModelState | None]:
    owner = _NativeStateOutput(pointer)
    pgc = get_power_grid_core()
    n_scenarios = pgc.state_output_scenario_count(owner.pointer)
    return [_build_scenario(owner, scenario_idx) for scenario_idx in range(n_scenarios)]
