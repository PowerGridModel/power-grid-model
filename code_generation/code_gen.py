# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from collections import Counter
from copy import deepcopy
from pathlib import Path

from jinja2 import Environment, FileSystemLoader
from meta_data import (
    AllDatasetMapData,
    AttributeClass,
    DatasetMapData,
    DatasetMetaData,
    ErrorCodeData,
    ErrorCodeMetaData,
    ExceptionMappingData,
)

DATA_DIR = Path(__file__).parent / "data"
TEMPLATE_DIR = Path(__file__).parent / "templates"
OUTPUT_PATH = Path(__file__).parents[1]

JINJA_ENV = Environment(loader=FileSystemLoader(TEMPLATE_DIR), autoescape=True)


def _data_type_nan(data_type: str):
    if data_type == "ID":
        return "na_IntID"
    if data_type == "double" or "RealValue" in data_type:
        return "nan"
    if data_type == "IntS":
        return "na_IntS"
    return f"static_cast<{data_type}>(na_IntS)"


def _categorize_error_codes(
    error_codes: list[ErrorCodeData], level_n_bits: int, max_bits: int
) -> dict[str, list[ErrorCodeData]]:
    """Validate the error code hierarchy and group every top-level error code with all of its descendants.

    The value of an error code at depth d is the value of its base type plus its index shifted by d bytes, e.g.
    0x00010305 is index 1 under 0x00000305, which is index 3 under 0x00000005.
    The numeric values are public contract, so any inconsistency is an error.
    """
    if level_n_bits <= 0 or max_bits <= 0 or max_bits % level_n_bits != 0:
        raise ValueError(
            f"error_code_max_bits ({max_bits}) must be a positive multiple of error_code_level_n_bits ({level_n_bits})"
        )
    max_depth = max_bits // level_n_bits
    max_index = (1 << level_n_bits) - 1

    by_name: dict[str, ErrorCodeData] = {}
    depths: dict[str, int] = {}
    roots: dict[str, str] = {}
    names_by_value: dict[int, str] = {}
    categories: dict[str, list[ErrorCodeData]] = {}

    for error_code in error_codes:
        name = error_code.name
        if name in by_name:
            raise ValueError(f"Duplicate error code name: {name}")

        if error_code.base_type is None:
            depth = 0
            base_value = 0
            roots[name] = name
            min_index = 0
        else:
            base = by_name.get(error_code.base_type)
            if base is None:
                raise ValueError(
                    f"Base type '{error_code.base_type}' of error code '{name}' must be declared before it"
                )
            depth = depths[base.name] + 1
            base_value = int(base.value, 16)
            roots[name] = roots[base.name]
            min_index = 1

        if depth >= max_depth:
            raise ValueError(f"Error code '{name}' is nested too deeply (maximum depth is {max_depth})")
        if not min_index <= error_code.index <= max_index:
            raise ValueError(
                f"Index {error_code.index} of error code '{name}' is out of range [{min_index}, {max_index}]"
            )

        value = int(error_code.value, 16)
        if value >= 1 << max_bits:
            raise ValueError(f"Value {error_code.value} of error code '{name}' exceeds {max_bits} bits")
        expected_value = base_value | (error_code.index << (level_n_bits * depth))
        if value != expected_value:
            raise ValueError(
                f"Value {error_code.value} of error code '{name}' does not match its hierarchy; "
                f"expected 0x{expected_value:08X}"
            )
        if value in names_by_value:
            raise ValueError(
                f"Error codes '{names_by_value[value]}' and '{name}' have the same value {error_code.value}"
            )

        by_name[name] = error_code
        depths[name] = depth
        names_by_value[value] = name
        categories.setdefault(roots[name], []).append(error_code)

    return categories


def _validate_exception_mappings(exception_mappings: list[ExceptionMappingData], error_codes: list[ErrorCodeData]):
    known_error_codes = {error_code.name for error_code in error_codes}
    for mapping in exception_mappings:
        if mapping.error_code not in known_error_codes:
            raise ValueError(f"Exception '{mapping.exception}' is mapped to unknown error code '{mapping.error_code}'")


class CodeGenerator:
    all_classes: dict[str, AttributeClass]
    base_output_path: Path

    def __init__(self, base_output_path: Path) -> None:
        self.all_classes = {}
        self.base_output_path = base_output_path

    def render_template(self, template_path: Path, output_path: Path, **data):
        # jinja expects a string, representing a relative path with forward slashes
        template_path_str = str(template_path.relative_to(TEMPLATE_DIR)).replace("\\", "/")
        template = JINJA_ENV.get_template(template_path_str)

        output = template.render(**data)

        with output_path.open(mode="w", encoding="utf-8") as output_file:
            output_file.write(output)

    def render_attribute_classes(self, template_path: Path, data_path: Path, output_path: Path):
        with data_path.open() as data_file:
            json_data = data_file.read()
        dataset_meta_data: DatasetMetaData = DatasetMetaData.schema().loads(json_data)
        # flatten attribute list
        # assign full name
        for attribute_class in dataset_meta_data.classes:
            if attribute_class.is_template:
                attribute_class.full_name = f"{attribute_class.name}<sym>"
                attribute_class.specification_names = [
                    f"{attribute_class.name}<symmetric_t>",
                    f"{attribute_class.name}<asymmetric_t>",
                    f"Sym{attribute_class.name}",
                    f"Asym{attribute_class.name}",
                ]
            else:
                attribute_class.full_name = attribute_class.name
                attribute_class.specification_names = [attribute_class.name]
            new_attribute_list = []
            for attribute in attribute_class.attributes:
                attribute.nan_value = _data_type_nan(attribute.data_type)
                if isinstance(attribute.names, str):
                    new_attribute_list.append(attribute)
                else:
                    # flatten list
                    for name in attribute.names:
                        new_attribute = deepcopy(attribute)
                        new_attribute.names = name
                        new_attribute_list.append(new_attribute)
            attribute_class.attributes = new_attribute_list
            # get full attribute
            if attribute_class.base is not None:
                base_class = next(filter(lambda x: x.name == attribute_class.base, dataset_meta_data.classes))
                attribute_class.full_attributes = base_class.full_attributes + attribute_class.attributes
                attribute_class.base_attributes = base_class.base_attributes.copy()
                attribute_class.base_attributes[base_class.name] = base_class.full_attributes
            else:
                attribute_class.full_attributes = attribute_class.attributes
                attribute_class.base_attributes = {}
            # add to class dict
            self.all_classes[attribute_class.name] = attribute_class

        self.render_template(
            template_path=template_path,
            output_path=output_path,
            classes=dataset_meta_data.classes,
            include_guard=dataset_meta_data.include_guard,
            name=dataset_meta_data.name,
        )

    def render_dataset_class_maps(self, template_path: Path, data_path: Path, output_path: Path):
        with data_path.open() as data_file:
            json_data = data_file.read()
        dataset_meta_data: list[DatasetMapData] = AllDatasetMapData.schema().loads(json_data).all_datasets

        # create list
        all_map = {}
        all_attributes = set()
        for dataset in dataset_meta_data:
            prefixes = ["sym_", "asym_"] if dataset.is_template else [""]
            for prefix in prefixes:
                all_components = {}
                for component in dataset.components:
                    class_def: AttributeClass = self.all_classes[component.class_name]
                    component_attributes = [attribute.names for attribute in class_def.full_attributes]
                    all_attributes.update(component_attributes)
                    for component_name in component.names:
                        all_components[component_name] = component_attributes
                all_map[f"{prefix}{dataset.name}"] = all_components
        self.render_template(
            template_path=template_path, output_path=output_path, all_map=all_map, all_attributes=sorted(all_attributes)
        )

    def render_error_codes(self, template_path: Path, data_path: Path, output_path: Path):
        with data_path.open() as data_file:
            json_data = data_file.read()
        error_code_data: ErrorCodeMetaData = ErrorCodeMetaData.schema().loads(json_data)
        error_codes = error_code_data.error_codes
        error_code_categories = _categorize_error_codes(
            error_codes, error_code_data.error_code_level_n_bits, error_code_data.error_code_max_bits
        )
        _validate_exception_mappings(error_code_data.exception_mappings, error_codes)
        user_types = sorted(
            {error_code.user_type for error_code in error_codes if error_code.user_type},
            key=str.lower,
        )
        # a user type shared by several error codes (e.g. PowerGridError) cannot identify a single code
        user_type_counts = Counter(error_code.user_type for error_code in error_codes)
        named_error_codes = [
            error_code
            for error_code in error_codes
            if error_code.user_type and user_type_counts[error_code.user_type] == 1
        ]

        self.render_template(
            template_path=template_path,
            output_path=output_path,
            error_code_level_n_bits=error_code_data.error_code_level_n_bits,
            error_code_max_bits=error_code_data.error_code_max_bits,
            error_code_categories=error_code_categories,
            error_codes=error_codes,
            named_error_codes=named_error_codes,
            exception_mappings=error_code_data.exception_mappings,
            user_types=user_types,
        )

    def code_gen(self):
        render_funcs = {
            "attribute_classes": self.render_attribute_classes,
            "dataset_class_maps": self.render_dataset_class_maps,
            "error_codes": self.render_error_codes,
        }

        # render attribute classes
        for template_name, render_func in render_funcs.items():
            for template_path in TEMPLATE_DIR.rglob(f"*{template_name}.*.jinja"):
                # a template may prefix the data name, e.g. test_api_error_codes.cpp.jinja
                output_prefix = template_path.name.split(template_name)[0]
                output_suffix = template_path.with_suffix("").suffix
                output_dir = template_path.parent.relative_to(TEMPLATE_DIR)
                for data_path in DATA_DIR.glob(f"{template_name}/*.json"):
                    output_name = f"{output_prefix}{data_path.with_suffix(output_suffix).name}"
                    output_path = self.base_output_path / output_dir / output_name
                    output_path.parent.mkdir(parents=True, exist_ok=True)
                    print(f"Generating file: {output_path}")
                    render_func(template_path=template_path, data_path=data_path, output_path=output_path)


if __name__ == "__main__":
    CodeGenerator(OUTPUT_PATH).code_gen()
