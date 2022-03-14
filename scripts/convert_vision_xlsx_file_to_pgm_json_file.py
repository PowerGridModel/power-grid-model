# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from argparse import ArgumentParser
from pathlib import Path
from typing import Optional

from power_grid_model.conversion.vision import read_vision_xlsx, read_vision_mapping, convert_vision_to_pgm
from power_grid_model.manual_testing import export_json_data
from power_grid_model.validation import validate_input_data, errors_to_string

BASE_DIR = Path(__file__).parent


def convert_vision_xlsx_file_to_pgm_json_file(
    input_file: Path, mapping_file: Path, output_file: Optional[Path], verbose: bool = False
) -> None:
    # Read mapping
    if mapping_file.suffix.lower() != ".yaml":
        raise ValueError(f"Mapping file should be a .yaml file, {mapping_file.suffix} provided.")
    if verbose:
        print(f"> Reading mapping file: {mapping_file}")
    mapping = read_vision_mapping(mapping_file)

    # Read input Workbook
    if input_file.suffix.lower() != ".xlsx":
        raise ValueError(f"Input file should be a .xlsx file, {input_file.suffix} provided.")
    if verbose:
        print(f"> Reading workbook file: {input_file}")
    workbook = read_vision_xlsx(input_file=input_file, units=mapping.get("units"), enums=mapping.get("enums"))

    # Check JSON file name
    if output_file is None:
        output_file = input_file.with_suffix(".json")
    if output_file.suffix.lower() != ".json":
        raise ValueError(f"Output file should be a .json file, {output_file.suffix} provided.")

    # Convert XLSX
    input_data, meta_data = convert_vision_to_pgm(workbook=workbook, mapping=mapping.get("grid"))
    if verbose:
        print(f"> Converted {input_file} using {mapping_file}, resulted in {len(meta_data)} objects.")

    # Store JSON data
    if verbose:
        print(f"> Writing JSON file: {output_file}")
    export_json_data(json_file=output_file, data=input_data, meta_data=meta_data, compact=True)

    # Validate data
    if verbose:
        print(f"> Validating Power Grid Model data")
    errors = validate_input_data(input_data, symmetric=False)
    if not errors:
        print("  Validation OK")
    else:
        print(errors_to_string(errors))
        if verbose:
            print()
            for error in errors:
                print(f"{type(error).__name__}: {error}")
                for obj_id in error.ids:
                    sheet = meta_data[obj_id].pop("sheet")
                    info = ", ".join(f"{key}={val}" for key, val in meta_data[obj_id].items())
                    print(f"{obj_id:>6}. {sheet}: {info}")


if __name__ == "__main__":
    parser = ArgumentParser(description="Convert a Vision .xslx export file to a Power Grid Model .json file")
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--mapping", type=Path, default=BASE_DIR / "mapping_en.yaml")
    parser.add_argument("--verbose", default=False, action="store_true")
    args = parser.parse_args()
    convert_vision_xlsx_file_to_pgm_json_file(
        input_file=args.input, mapping_file=args.mapping, output_file=args.output, verbose=args.verbose
    )
