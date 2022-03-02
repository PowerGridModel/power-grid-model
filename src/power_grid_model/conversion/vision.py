# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0
import numbers
from importlib import import_module
from pathlib import Path
from typing import Any, Callable, Dict, List, Optional, Tuple

import numpy as np
import pandas as pd
import yaml

from .. import initialize_array


def read_vision_xlsx(input_file: Path) -> Dict[str, Tuple[pd.DataFrame, Dict[str, Optional[str]]]]:
    # The first row contains the column name, the second row is the (optional) unit
    # Together they forn the column index
    sheets = pd.read_excel(io=input_file, sheet_name=None, header=[0, 1])

    vision_data: Dict[str, Tuple[pd.DataFrame, List[Optional[str]]]] = {}
    for sheet_name, sheet_data in sheets.items():
        # Let's extract the column names and units from the column indexes
        column_names = [
            col_idx[0]
            for col_idx in sheet_data.columns
        ]
        units = {
            col_idx[0]: col_idx[1]
            if not col_idx[1].startswith("Unnamed") else None
            for col_idx in sheet_data.columns
        }

        # Index the columns by their name only
        sheet_data.columns = column_names

        # Store the data and the units
        vision_data[sheet_name] = (sheet_data, units)
    return vision_data


def read_vision_mapping(mapping_file: Path) -> Dict[str, Dict[str, Any]]:
    with open(mapping_file, "r") as mapping_stream:
        return yaml.safe_load(mapping_stream)


def convert_vision_to_pgm(input_workbook: Dict[str, Tuple[pd.DataFrame, Dict[str, Optional[str]]]],
                          mapping: Dict[str, Dict[str, Any]]) \
        -> Tuple[Dict[str, np.ndarray], Dict[int, Dict[str, Any]]]:
    enums = mapping.get("enums", {})
    units = mapping.get("units", {})
    pgm_data: Dict[str, List[np.ndarray]] = {}
    meta_data: List[Dict[str, Any]] = []
    for sheet_name, components in mapping["grid"].items():
        for component_name, instances in components.items():
            if not isinstance(instances, list):
                instances = [instances]
            for attributes in instances:
                sheet_pgm_data, sheet_meta_data = _convert_vision_sheet_to_pgm_component(
                    input_workbook=input_workbook,
                    sheet_name=sheet_name,
                    component_name=component_name,
                    attributes=attributes,
                    enums=enums,
                    units=units,
                    base_id=len(meta_data)
                )
                if component_name not in pgm_data:
                    pgm_data[component_name] = []
                meta_data += sheet_meta_data
                pgm_data[component_name].append(sheet_pgm_data)
    return _merge_pgm_data(pgm_data), dict(enumerate(meta_data))


def _merge_pgm_data(pgm_data: Dict[str, List[np.ndarray]]) -> Dict[str, np.ndarray]:
    merged = {}
    for component_name, data_set in pgm_data.items():
        if len(data_set) == 1:
            merged[component_name] = data_set[0]
        elif len(data_set) > 1:
            idx_ptr = [0]
            for arr in data_set:
                idx_ptr.append(idx_ptr[-1] + len(arr))
            merged[component_name] = initialize_array(data_type="input", component_type=component_name,
                                                      shape=idx_ptr[-1])
            for i, arr in enumerate(data_set):
                merged[component_name][idx_ptr[i]:idx_ptr[i + 1]] = arr
    return merged


def _convert_vision_sheet_to_pgm_component(input_workbook: Dict[str, Tuple[pd.DataFrame, List[Optional[str]]]],
                                           sheet_name: str, component_name: str, attributes: Dict[str, str],
                                           enums: Dict[str, Dict[str, int]], units: Dict[str, float], base_id: int) \
        -> Tuple[Dict[str, np.ndarray], List[Dict[str, Any]]]:
    sheet, col_units = input_workbook[sheet_name]
    meta_data = [{"sheet": sheet_name} for _ in range(len(sheet))]
    pgm_data = initialize_array(data_type="input", component_type=component_name, shape=len(sheet))

    def _parse_col_def(col_def: Any) -> Tuple[Any, List[str]]:
        if isinstance(col_def, numbers.Number):
            return _parse_col_def_const(col_def)
        elif isinstance(col_def, str):
            return _parse_col_def_column_name(col_def)
        elif isinstance(col_def, dict) and len(col_def) == 1:
            return _parse_col_def_map(col_def)
        elif isinstance(col_def, list):
            return _parse_col_def_composite(col_def)
        else:
            raise TypeError(col_def)

    def _parse_col_def_const(col_def: numbers.Number) -> Tuple[Any, List[str]]:
        assert isinstance(col_def, numbers.Number)
        return [col_def], ['const']

    def _parse_col_def_column_name(col_def: str) -> Tuple[Any, List[str]]:
        assert isinstance(col_def, str)
        if col_def not in sheet:
            raise KeyError(f"Could not find column '{col_def}' in sheet '{sheet_name}' (for {component_name})")
        col_data = sheet[col_def]
        if attr in enums:
            col_data = col_data.map(enums[attr])
        if col_units[col_def] in units:
            col_data *= float(units[col_units[col_def]])
        return [col_data], [col_def]

    def _parse_col_def_map(col_def: Dict[str, str]) -> Tuple[Any, List[str]]:
        assert isinstance(col_def, dict) and len(col_def) == 1
        col_name, fn_name = col_def.popitem()
        data, col_name = _parse_col_def_column_name(col_name)
        fn = _get_function(fn_name)
        return [data[0].map(fn)], col_name

    def _parse_col_def_composite(col_def: list) -> Tuple[Any, List[str]]:
        assert isinstance(col_def, list)
        data = []
        labels = []
        for sub_def in col_def:
            sub_data, sub_labels = _parse_col_def(sub_def)
            data += sub_data
            labels += sub_labels
        return data, labels

    for attr, col_def in attributes.items():
        if not attr in pgm_data.dtype.names:
            attrs = ", ".join(pgm_data.dtype.names)
            raise KeyError(f"Could not find attribute '{attr}' for '{component_name}'. (choose from: {attrs})")

        if attr == "id":
            for data, label in zip(*_parse_col_def(col_def)):
                for i, meta in enumerate(meta_data):
                    meta[label] = data[i].item()
            col_data = range(base_id, base_id + len(sheet))
        else:
            col_data, _ = _parse_col_def(col_def)
            if len(col_data) != 1:
                raise NotImplementedError("Can't use composite values for regular columns")
            col_data = col_data.pop()
        pgm_data[attr] = col_data

    return pgm_data, meta_data


def _get_function(fn: str) -> Callable:
    module_path, function_name = fn.rsplit('.', 1)
    module = import_module(module_path)
    function = getattr(module, function_name)
    return function
