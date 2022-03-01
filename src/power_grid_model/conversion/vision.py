# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import numpy as np
import pandas as pd
import yaml

from .. import initialize_array


def read_vision_xlsx(input_file: Path) -> Dict[str, Tuple[pd.DataFrame, List[Optional[str]]]]:
    # The first row contains the column name, the second row is the (optional) unit
    # Together they forn the column index
    sheets = pd.read_excel(io=input_file, sheet_name=None, header=[0, 1])

    vision_data: Dict[str, Tuple[pd.DataFrame, List[Optional[str]]]] = {}
    for sheet_name, sheet_data in sheets.items():
        # Let's extract the column names and units from the column indexes
        column_names = [col_idx[0] for col_idx in sheet_data.columns]
        units = [col_idx[1] if not col_idx[1].startswith("Unnamed") else None for col_idx in sheet_data.columns]

        # Index the columns by their name only
        sheet_data.columns = column_names

        # Store the data and the units
        vision_data[sheet_name] = (sheet_data, units)
    return vision_data


def read_vision_mapping(mapping_file: Path) -> Dict[str, Dict[str, Any]]:
    with open(mapping_file, "r") as mapping_stream:
        return yaml.safe_load(mapping_stream)


def convert_vision_to_pgm(input_workbook: Dict[str, Tuple[pd.DataFrame, List[Optional[str]]]],
                          mapping: Dict[str, Dict[str, Any]]) \
        -> Tuple[Dict[str, np.ndarray], Dict[int, Dict[str, Any]]]:
    pgm_data: Dict[str, List[np.ndarray]] = {}
    meta_data: List[Dict[str, Any]] = []
    for component, config in mapping.items():
        sheet_names = config["sheet"] if isinstance(config["sheet"], list) else [config["sheet"]]
        for sheet_name in sheet_names:
            sheet, units = input_workbook[sheet_name]
            sheet_pgm_data, sheet_obj_ids = _convert_vision_sheet_to_pgm_component(
                sheet=sheet,
                component=component,
                attributes=config["attributes"],
                enums=config.get("enums", {}),
                units=units,
                base_id=len(meta_data)
            )
            meta_data += [{"sheet": sheet_name, config["attributes"]["id"]: obj_id} for obj_id in sheet_obj_ids]
            for component, component_data in sheet_pgm_data.items():
                if component not in pgm_data:
                    pgm_data[component] = []
                pgm_data[component].append(component_data)
    return _merge_pgm_data(pgm_data), dict(enumerate(meta_data))


def _merge_pgm_data(pgm_data: Dict[str, List[np.ndarray]]) -> Dict[str, np.ndarray]:
    merged = {}
    for component, data_set in pgm_data.items():
        if len(data_set) == 1:
            merged[component] = data_set[0]
        elif len(data_set) > 1:
            idx_ptr = [0]
            for arr in data_set:
                idx_ptr.append(idx_ptr[-1] + len(arr))
            merged[component] = initialize_array(data_type="input", component_type=component, shape=idx_ptr[-1])
            for i, arr in enumerate(data_set):
                merged[component][idx_ptr[i]:idx_ptr[i + 1]] = arr
    return merged


def _convert_vision_sheet_to_pgm_component(sheet: pd.DataFrame, component: str, attributes: Dict[str, str],
                                           enums: Dict[int, str], units: List[str], base_id: int) \
        -> Tuple[Dict[str, np.ndarray], List[Any]]:
    object_ids = []
    pgm_data = initialize_array(data_type="input", component_type=component, shape=len(sheet))
    for attr, column_name in attributes.items():
        col_data = sheet[column_name]
        if enums and attr in enums:
            mapping = {v: k for k, v in enums[attr].items()}
            col_data = col_data.map(mapping)
        if attr == "id":
            object_ids += col_data.tolist()
            col_data = range(base_id, base_id + len(col_data))
        pgm_data[attr] = col_data
    return {component: pgm_data}, object_ids
