# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from argparse import ArgumentParser
from pathlib import Path
from typing import Optional

from power_grid_model import PowerGridModel
from power_grid_model.conversion.vision import read_vision_xlsx, read_vision_mapping, convert_vision_to_pgm
from power_grid_model.manual_testing import export_json_data
from power_grid_model.validation import validate_input_data, errors_to_string

BASE_DIR = Path(__file__).parent


def convert_vision_xlsx_file_to_pgm_json_file(input_file: Path, mapping_file: Path,
                                              output_file: Optional[Path] = None) -> None:
    # Input Workbook
    if input_file.suffix.lower() != ".xlsx":
        raise ValueError(f"Input file should be a .xlsx file, {input_file.suffix} provided.")
    input_workbook = read_vision_xlsx(input_file)

    # Mapping
    if mapping_file.suffix.lower() != ".yaml":
        raise ValueError(f"Mapping file should be a .yaml file, {mapping_file.suffix} provided.")
    mapping = read_vision_mapping(mapping_file)

    # Output file
    if output_file is None:
        output_file = input_file.with_suffix(".json")
    if output_file.suffix.lower() != ".json":
        raise ValueError(f"Output file should be a .json file, {output_file.suffix} provided.")

    # Convert XLSX
    input_data, meta_data = convert_vision_to_pgm(input_workbook=input_workbook, mapping=mapping)

    errors = validate_input_data(input_data)
    if errors:
        print(errors_to_string(errors, details=True))

    # Store Input JSON
    export_json_data(json_file=output_file, data=input_data, meta_data=meta_data)

    model = PowerGridModel(input_data=input_data)
    model.calculate_power_flow()


if __name__ == "__main__":
    parser = ArgumentParser(description="Convert a Vision .xslx export file to a Power Grid Model .json file")
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--mapping", type=Path, default=BASE_DIR / "mapping_en.yaml")
    args = parser.parse_args()
    convert_vision_xlsx_file_to_pgm_json_file(input_file=args.input, mapping_file=args.mapping, output_file=args.output)
