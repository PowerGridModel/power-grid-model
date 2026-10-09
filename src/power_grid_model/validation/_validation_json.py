# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""Experimental Python entry points for native JSON validation reports."""

from ctypes import byref
from typing import Any

from power_grid_model._core.dataset_definitions import DatasetType
from power_grid_model._core.error_handling import assert_no_error
from power_grid_model._core.index_integer import IdxC
from power_grid_model._core.options import Options
from power_grid_model._core.power_grid_core import CharPtr, ConstDatasetPtr, get_power_grid_core
from power_grid_model._core.power_grid_dataset import CConstDataset
from power_grid_model.data_types import Dataset


def _native_validation_json(core: Any, result: Any) -> bytes:
    assert_no_error()
    if not result:
        raise RuntimeError("Native validation returned no result without setting a handle error.")

    report_data = CharPtr()
    report_size = IdxC()
    core.validation_result_get_json(result, byref(report_data), byref(report_size))
    assert_no_error()
    if not report_data or report_size.value < 0:
        raise RuntimeError("Native validation returned an invalid JSON report buffer.")
    raw_report = report_data[: report_size.value]
    if not isinstance(raw_report, bytes):
        raise RuntimeError("Native validation returned non-byte report data.")
    return raw_report


def validate_input_data_json(input_data: Dataset, options: Options | None = None) -> bytes:
    """Return the experimental native input validation report as UTF-8 JSON bytes."""
    native_dataset = CConstDataset(data=input_data, dataset_type=DatasetType.input)
    core = get_power_grid_core()
    options = options or Options()
    result = core.validate_input_data(native_dataset.get_dataset_ptr(), options.opt)
    try:
        return _native_validation_json(core, result)
    finally:
        if result:
            core.destroy_validation_result(result)


def _create_empty_batch_update_dataset(core: Any) -> ConstDatasetPtr:
    mutable_dataset = core.create_dataset_mutable(DatasetType.update.value, 1, 0)
    const_dataset = ConstDatasetPtr()
    try:
        assert_no_error()
        if not mutable_dataset:
            raise RuntimeError("Native update dataset creation returned no dataset without setting a handle error.")
        const_dataset = core.create_dataset_const_from_mutable(mutable_dataset)
        assert_no_error()
        if not const_dataset:
            raise RuntimeError("Native update dataset conversion returned no dataset without setting a handle error.")
        return const_dataset
    except Exception:
        if const_dataset:
            core.destroy_dataset_const(const_dataset)
        raise
    finally:
        if mutable_dataset:
            core.destroy_dataset_mutable(mutable_dataset)


def validate_batch_data_json(
    input_data: Dataset, update_data: Dataset, options: Options | None = None
) -> bytes:
    """Return the experimental native batch validation report as UTF-8 JSON bytes."""
    native_input = CConstDataset(data=input_data, dataset_type=DatasetType.input)
    core = get_power_grid_core()
    options = options or Options()
    empty_update_dataset = None
    native_updates = None
    try:
        if update_data:
            native_updates = CConstDataset(data=update_data, dataset_type=DatasetType.update)
            update_dataset_pointer = native_updates.get_dataset_ptr()
        else:
            empty_update_dataset = _create_empty_batch_update_dataset(core)
            update_dataset_pointer = empty_update_dataset

        result = core.validate_batch_data(native_input.get_dataset_ptr(), update_dataset_pointer, options.opt)
        try:
            return _native_validation_json(core, result)
        finally:
            if result:
                core.destroy_validation_result(result)
    finally:
        if empty_update_dataset:
            core.destroy_dataset_const(empty_update_dataset)
