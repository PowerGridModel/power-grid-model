# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import re
from importlib import import_module
from numbers import Number
from pathlib import Path
from typing import Any, Callable, Dict, List, Optional, Tuple

import numpy as np
import pandas as pd
import yaml

from .auto_id import AutoID
from .. import initialize_array

COL_REF_RE = re.compile(r"([^!]+)!([^\[]+)\[(([^!]+)!)?([^=]+)=(([^!]+)!)?([^\]]+)\]")


def read_vision_xlsx(
    input_file: Path, units: Optional[Dict[str, float]] = None, enums: Optional[Dict[str, Dict[str, Any]]] = None
) -> Dict[str, pd.DataFrame]:
    # Read the vision excel file and assume that the first row contains the column name and the second row is the unit.
    sheets = pd.read_excel(io=input_file, sheet_name=None, header=[0, 1])

    if units is None:
        units = {}

    if enums is None:
        enums = {}

    for sheet_name, sheet_data in sheets.items():
        for col_idx in sheet_data.columns:
            col_name, col_unit = col_idx
            base_col_name = str(col_name).split(".").pop()
            if base_col_name in enums:
                sheet_data[col_idx] = sheet_data[col_idx].map(lambda x: enums[base_col_name].get(x, x))
            if col_unit in units:
                multiplier = float(units[col_unit])
                try:
                    sheet_data[col_idx] *= multiplier
                except TypeError as ex:
                    print(f"WARNING: The column '{col_name}' on sheet '{sheet_name}' does not seem to be numerical "
                          f"while trying to apply a multiplier ({multiplier}) for unit '{col_unit}': {ex}")

        # Let's extract the column names and units from the column indexes
        column_names = [col_idx[0] for col_idx in sheet_data.columns]
        sheet_data.columns = column_names

    return sheets


def read_vision_mapping(mapping_file: Path) -> Dict[str, Dict[str, Any]]:
    with open(mapping_file, "r", encoding="utf-8") as mapping_stream:
        return yaml.safe_load(mapping_stream)


def convert_vision_to_pgm(
    workbook: Dict[str, pd.DataFrame], mapping: Dict[str, Dict[str, Any]]
) -> Tuple[Dict[str, np.ndarray], Dict[int, Dict[str, Any]]]:
    pgm_data: Dict[str, List[np.ndarray]] = {}
    meta_data: Dict[int, Dict[str, Any]] = {}
    lookup = AutoID()
    for sheet_name, components in mapping.items():
        for component_name, attributes in components.items():
            sheet_pgm_data, sheet_meta_data = _convert_vision_sheet_to_pgm_component(
                workbook=workbook,
                sheet_name=sheet_name,
                component_name=component_name,
                attributes=attributes,
                lookup=lookup,
            )
            if sheet_pgm_data is not None:
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
            merged[component_name] = initialize_array(
                data_type="input", component_type=component_name, shape=idx_ptr[-1]
            )
            for i, arr in enumerate(data_set):
                merged[component_name][idx_ptr[i] : idx_ptr[i + 1]] = arr
    return merged


def _convert_vision_sheet_to_pgm_component(
    workbook: Dict[str, pd.DataFrame], sheet_name: str, component_name: str, attributes: Dict[str, str], lookup: AutoID
) -> Tuple[Optional[np.ndarray], Dict[int, Dict[str, Any]]]:
    if sheet_name not in workbook:
        return None, {}

    n_records = len(workbook[sheet_name])

    try:
        pgm_data = initialize_array(data_type="input", component_type=component_name, shape=n_records)
    except KeyError as ex:
        raise KeyError(f"Invalid component type '{component_name}'") from ex

    meta_data = {}

    for attr, col_def in attributes.items():
        if attr not in pgm_data.dtype.names:
            attrs = ", ".join(pgm_data.dtype.names)
            raise KeyError(f"Could not find attribute '{attr}' for '{component_name}'. (choose from: {attrs})")

        col_data = _parse_col_def(workbook=workbook, sheet_name=sheet_name, col_def=col_def)
        if attr == "id":
            meta = col_data.to_dict(orient="records")
            col_data = col_data.apply(lambda row: _id_lookup(lookup, component_name, row), axis=1)
            for i, m in zip(col_data, meta):
                meta_data[i] = {"sheet": sheet_name}
                meta_data[i].update(m)
        elif attr.endswith("node"):
            col_data = col_data.apply(lambda row: _id_lookup(lookup, "node", row), axis=1)
        elif len(col_data.columns) != 1:
            raise ValueError(
                f"DataFrame for {component_name}.{attr} should contain a single column " f"({col_data.columns})"
            )
        else:
            col_data = col_data.iloc[:, 0]
        try:
            pgm_data[attr] = col_data
        except ValueError as ex:
            if "invalid literal" in str(ex) and isinstance(col_def, str):
                raise ValueError(f"Possibly missing enum value for '{col_def}' column on '{sheet_name}' sheet: {ex}") \
                    from ex
            else:
                raise ex

    return pgm_data, meta_data


def _parse_col_def(workbook: Dict[str, pd.DataFrame], sheet_name: str, col_def: Any) -> pd.DataFrame:
    if isinstance(col_def, Number):
        return _parse_col_def_const(col_def=col_def)
    elif isinstance(col_def, str) and "!" in col_def:
        return _parse_col_def_column_reference(workbook=workbook, sheet_name=sheet_name, col_def=col_def)
    elif isinstance(col_def, str):
        return _parse_col_def_column_name(workbook=workbook, sheet_name=sheet_name, col_def=col_def)
    elif isinstance(col_def, dict):
        return _parse_col_def_function(workbook=workbook, sheet_name=sheet_name, col_def=col_def)
    elif isinstance(col_def, list):
        return _parse_col_def_composite(workbook=workbook, sheet_name=sheet_name, col_def=col_def)
    else:
        raise TypeError(f"Invalid column definition: {col_def}")


def _parse_col_def_const(col_def: Number) -> pd.DataFrame:
    assert isinstance(col_def, Number)
    return pd.DataFrame([col_def])


def _parse_col_def_column_name(workbook: Dict[str, pd.DataFrame], sheet_name: str, col_def: str) -> pd.DataFrame:
    assert isinstance(col_def, str)
    sheet = workbook[sheet_name]

    columns = [col_name.strip() for col_name in col_def.split("|")]
    for col_name in columns:
        if col_name in sheet:
            return pd.DataFrame(sheet[col_name])

    try:  # Maybe it is not a column name, but a float value like 'inf'
        return _parse_col_def_const(float(col_def))
    except ValueError:
        columns = ' and '.join(f"'{col_name}'" for col_name in columns)
        raise KeyError(f"Could not find column {columns} on sheet '{sheet_name}'")


def _parse_col_def_column_reference(workbook: Dict[str, pd.DataFrame], sheet_name: str, col_def: str) -> pd.DataFrame:
    assert isinstance(col_def, str)
    match = COL_REF_RE.fullmatch(col_def)
    if match is None:
        raise ValueError(
            f"Invalid column reference '{col_def}' " "(should be 'OtherSheet!ValueColumn[IdColumn=RefColumn])"
        )
    other_sheet, value_col_name, _, other_sheet_, id_col_name, _, this_sheet_, ref_col_name = match.groups()
    if (other_sheet_ is not None and other_sheet_ != other_sheet) or (
        this_sheet_ is not None and this_sheet_ != sheet_name
    ):
        raise ValueError(
            f"Invalid column reference '{col_def}'.\n"
            "It should be something like "
            f"{other_sheet}!{value_col_name}[{other_sheet}!{{id_column}}={sheet_name}!{{ref_column}}] "
            f"or simply {other_sheet}!{value_col_name}[{{id_column}}={{ref_column}}]"
        )
    ref_column = _parse_col_def_column_name(workbook=workbook, sheet_name=sheet_name, col_def=ref_col_name)
    id_column = _parse_col_def_column_name(workbook=workbook, sheet_name=other_sheet, col_def=id_col_name)
    val_column = _parse_col_def_column_name(workbook=workbook, sheet_name=other_sheet, col_def=value_col_name)
    other = pd.concat([id_column, val_column], axis=1)
    result = ref_column.merge(other, how="left", left_on=ref_col_name, right_on=id_col_name)
    return result[value_col_name]


def _parse_col_def_function(
    workbook: Dict[str, pd.DataFrame], sheet_name: str, col_def: Dict[str, str]
) -> pd.DataFrame:
    assert isinstance(col_def, dict)
    data = []
    for fn_name, sub_def in col_def.items():
        fn = _get_function(fn_name)
        col_data = _parse_col_def(workbook=workbook, sheet_name=sheet_name, col_def=sub_def)
        data.append(col_data.apply(lambda row: fn(*row), axis=1, raw=True))
    return pd.concat(data, axis=1)


def _parse_col_def_composite(workbook: Dict[str, pd.DataFrame], sheet_name: str, col_def: list) -> pd.DataFrame:
    assert isinstance(col_def, list)
    columns = [_parse_col_def(workbook=workbook, sheet_name=sheet_name, col_def=sub_def) for sub_def in col_def]
    return pd.concat(columns, axis=1)


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


def _id_lookup(lookup: AutoID, component: str, row: pd.Series) -> int:
    data = {col.split(".").pop(): val for col, val in sorted(row.to_dict().items(), key=lambda x: x[0])}
    key = component + ":" + ",".join(f"{k}={v}" for k, v in data.items())
    return lookup[key]
