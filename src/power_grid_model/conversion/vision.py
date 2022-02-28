from pathlib import Path
from typing import Any, Dict, Optional

import numpy as np
import pandas as pd
import yaml

from .. import initialize_array


def read_vision_xlsx(input_file: Path) -> Dict[str, pd.DataFrame]:
    return pd.read_excel(io=input_file, sheet_name=None, header=[0, 1])


def read_vision_mapping(mapping_file: Path) -> Dict[str, Dict[str, Any]]:
    with open(mapping_file, "r") as mapping_stream:
        return yaml.safe_load(mapping_stream)


def convert_vision_to_pgm(input_workbook: Dict[str, pd.DataFrame], mapping: Dict[str, Dict[str, Any]]) \
        -> Dict[str, np.ndarray]:
    pgm_data: Dict[str, np.ndarray] = {}
    for component, config in mapping.items():
        sheet = input_workbook[config["sheet"]]
        pgm_data[component] = _convert_vision_sheet_to_pgm_component(
            sheet=sheet,
            component=component,
            attributes=config["attributes"],
            enums=config.get("enums")
        )
    return pgm_data


def _convert_vision_sheet_to_pgm_component(sheet: pd.DataFrame, component: str, attributes: Dict[str, str],
                                           enums: Optional[Dict[int, str]] = None) -> np.ndarray:
    pgm_data = initialize_array(data_type="input", component_type=component, shape=len(sheet))
    column_idx = {}
    for col in sheet.columns:
        if col[0] in attributes.values() and col[0] not in column_idx:
            column_idx[col[0]] = col
    for attr, column_name in attributes.items():
        col_data = sheet[column_idx[column_name]]
        if enums and attr in enums:
            mapping = {v: k for k, v in enums[attr].items()}
            col_data = col_data.map(mapping)
        pgm_data[attr] = col_data
    return pgm_data
