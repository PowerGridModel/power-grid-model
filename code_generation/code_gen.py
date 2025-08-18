# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from copy import deepcopy
from pathlib import Path

from jinja2 import Environment, FileSystemLoader
from meta_data import AllDatasetMapData, AttributeClass, DatasetMapData, DatasetMetaData

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
        for dataset in dataset_meta_data:
            prefixes = ["sym_", "asym_"] if dataset.is_template else [""]
            for prefix in prefixes:
                all_components = {}
                for component in dataset.components:
                    class_def: AttributeClass = self.all_classes[component.class_name]
                    for component_name in component.names:
                        all_components[component_name] = [x.names for x in class_def.full_attributes]
                all_map[f"{prefix}{dataset.name}"] = all_components
        self.render_template(template_path=template_path, output_path=output_path, all_map=all_map)

    def code_gen(self):
        render_funcs = {
            "attribute_classes": self.render_attribute_classes,
            "dataset_class_maps": self.render_dataset_class_maps,
        }

        # render attribute classes
        for template_name, render_func in render_funcs.items():
            for template_path in TEMPLATE_DIR.rglob(f"{template_name}.*.jinja"):
                output_suffix = template_path.with_suffix("").suffix
                output_dir = template_path.parent.relative_to(TEMPLATE_DIR)
                for data_path in DATA_DIR.glob(f"{template_name}/*.json"):
                    output_path = self.base_output_path / output_dir / data_path.with_suffix(output_suffix).name
                    output_path.parent.mkdir(parents=True, exist_ok=True)
                    print(f"Generating file: {output_path}")
                    render_func(template_path=template_path, data_path=data_path, output_path=output_path)


if __name__ == "__main__":
    CodeGenerator(OUTPUT_PATH).code_gen()
