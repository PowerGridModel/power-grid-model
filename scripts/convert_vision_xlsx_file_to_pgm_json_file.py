# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from argparse import ArgumentParser
from pathlib import Path

import numpy as np
import pandas as pd
from power_grid_model import PowerGridModel
from power_grid_model.conversion.vision import read_vision_xlsx, read_vision_mapping, convert_vision_to_pgm
from power_grid_model.manual_testing import export_json_data
from power_grid_model.validation import assert_valid_input_data, errors_to_string, ValidationException

BASE_DIR = Path(__file__).parent


def convert_vision_xlsx_file_to_pgm_json_file(input_file: Path, mapping_file: Path) -> None:
    # Mapping
    if mapping_file.suffix.lower() != ".yaml":
        raise ValueError(f"Mapping file should be a .yaml file, {mapping_file.suffix} provided.")
    mapping = read_vision_mapping(mapping_file)

    # Input Workbook
    if input_file.suffix.lower() != ".xlsx":
        raise ValueError(f"Input file should be a .xlsx file, {input_file.suffix} provided.")
    workbook = read_vision_xlsx(input_file=input_file, units=mapping.get("units"), enums=mapping.get("enums"))

    # dump file
    input_str = input_file.with_suffix("")
    dump_input = Path(f"{input_str}_input.json")
    dump_sym_output = Path(f"{input_str}_sym_output.json")
    dump_asym_output = Path(f"{input_str}_asym_output.json")

    # Convert XLSX
    input_data, meta_data = convert_vision_to_pgm(workbook=workbook, mapping=mapping.get("grid"))

    # Store Input JSON
    export_json_data(json_file=dump_input, data=input_data, meta_data=meta_data, compact=True)

    # Validate data
    try:
        assert_valid_input_data(input_data)
        assert_valid_input_data(input_data, symmetric=False)
    except ValidationException as ex:
        print(errors_to_string(ex.errors, details=True))
        raise

    model = PowerGridModel(input_data=input_data)
    sym_output_data = model.calculate_power_flow()
    # store sym output
    export_json_data(json_file=dump_sym_output, data=sym_output_data, meta_data=meta_data, compact=True)

    # print symmetric results
    for component in sym_output_data:
        df_input = pd.DataFrame(sym_output_data[component])
        df_output = pd.DataFrame(sym_output_data[component])
        print(component)
        print(df_input)
        print(df_output)
    u_pu = sym_output_data["node"]["u_pu"][sym_output_data["node"]["energized"] == 1]
    print("u_pu", np.min(u_pu), np.max(u_pu))

    # asymmetric
    asym_output_data = model.calculate_power_flow(symmetric=False, error_tolerance=1e-5)
    export_json_data(json_file=dump_asym_output, data=asym_output_data, meta_data=meta_data, compact=True)


if __name__ == "__main__":
    parser = ArgumentParser(description="Convert a Vision .xslx export file to a Power Grid Model .json file")
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--mapping", type=Path, default=BASE_DIR / "mapping_en.yaml")
    args = parser.parse_args()
    convert_vision_xlsx_file_to_pgm_json_file(input_file=args.input, mapping_file=args.mapping)
