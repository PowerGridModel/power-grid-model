# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0
from importlib import import_module
from numbers import Number
from pathlib import Path
from typing import Any, Callable, Dict, List, Optional, Tuple

import numpy as np
import pandas as pd
import yaml

from .auto_id import AutoID
from .. import initialize_array

LOOKUP = AutoID()


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
    meta_data: Dict[int, Dict[str, Any]] = {}
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
                    units=units
                )
                if component_name not in pgm_data:
                    pgm_data[component_name] = []
                meta_data.update(sheet_meta_data)
                pgm_data[component_name].append(sheet_pgm_data)
    return _merge_pgm_data(pgm_data), meta_data


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
                                           enums: Dict[str, Dict[str, int]], units: Dict[str, float]) \
        -> Tuple[Dict[str, np.ndarray], Dict[int, Dict[str, Any]]]:
    sheet, col_units = input_workbook[sheet_name]
    meta_data = {}
    try:
        pgm_data = initialize_array(data_type="input", component_type=component_name, shape=len(sheet))
    except KeyError as ex:
        raise KeyError(f"Invalid component type '{component_name}'") from ex

    def _parse_col_def(col_def: Any) -> pd.DataFrame:
        if isinstance(col_def, Number):
            return _parse_col_def_const(col_def)
        elif isinstance(col_def, str):
            return _parse_col_def_column_name(col_def)
        elif isinstance(col_def, dict) and len(col_def) == 1:
            return _parse_col_def_map(col_def)
        elif isinstance(col_def, list):
            return _parse_col_def_composite(col_def)
        else:
            raise TypeError(f"Invalid colmn definition: {col_def}")

    def _parse_col_def_const(col_def: Number) -> pd.DataFrame:
        assert isinstance(col_def, Number)
        return pd.DataFrame([col_def])

    def _parse_col_def_column_name(col_def: str) -> pd.DataFrame:
        assert isinstance(col_def, str)
        if col_def not in sheet:
            try:
                return _parse_col_def_const(float(col_def))
            except ValueError:
                raise KeyError(f"Could not find column '{col_def}' in sheet '{sheet_name}' (for {component_name})")
        col_data = sheet[col_def]
        if attr in enums:
            col_data = col_data.map(enums[attr])
        if col_units[col_def] in units:
            col_data *= float(units[col_units[col_def]])
        return pd.DataFrame(col_data, columns=[col_def])

    def _parse_col_def_map(col_def: Dict[str, str]) -> pd.DataFrame:
        assert isinstance(col_def, dict)
        data = pd.DataFrame()
        for col_name, fn_name in col_def.items():
            fn = _get_function(fn_name)
            col_data = _parse_col_def_column_name(col_name)
            for key, col in col_data.items():
                data[key] = col.map(fn)
        return data

    def _parse_col_def_composite(col_def: list) -> pd.DataFrame:
        assert isinstance(col_def, list)
        data = pd.DataFrame()
        columns = [_parse_col_def(sub_def) for sub_def in col_def]
        return pd.concat(columns, axis=1)

    def _id_lookup(component: str, row: pd.Series) -> int:
        data = {col.split(".").pop(): val for col, val in sorted(row.to_dict().items(), key=lambda x: x[0])}
        key = component + ":" + ",".join(f"{k}={v}" for k, v in data.items())
        return LOOKUP[key]

    for attr, col_def in attributes.items():
        if not attr in pgm_data.dtype.names:
            attrs = ", ".join(pgm_data.dtype.names)
            raise KeyError(f"Could not find attribute '{attr}' for '{component_name}'. (choose from: {attrs})")

        col_data = _parse_col_def(col_def)
        if attr == "id":
            meta = col_data.to_dict(orient="records")
            col_data = col_data.apply(lambda row: _id_lookup(component_name, row), axis=1)
            for i, m in zip(col_data, meta):
                meta_data[i] = {"sheet": sheet_name}
                meta_data[i].update(m)
        elif attr.endswith("node"):
            col_data = col_data.apply(lambda row: _id_lookup("node", row), axis=1)
        elif len(col_data.columns) != 1:
            raise ValueError(f"DataFrame for {component_name}.{attr} should contain a single column "
                             f"({col_data.columns})")
        else:
            col_data = col_data.iloc[:, 0]
        pgm_data[attr] = col_data

    return pgm_data, meta_data


def _get_function(fn: str) -> Callable:
    parts = fn.split(".")
    function_name = parts.pop()
    module_path = ".".join(parts) if parts else "builtins"
    try:
        module = import_module(module_path)
    except ModuleNotFoundError:
        raise AttributeError(f"Function: {fn} does not exist")
    try:
        function = getattr(module, function_name)
    except AttributeError:
        raise AttributeError(f"Function: {function_name} does not exist in {module_path}")
    return function
