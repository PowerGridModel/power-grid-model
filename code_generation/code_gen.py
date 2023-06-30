# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from copy import deepcopy
from pathlib import Path

from jinja2 import Environment, FileSystemLoader
from meta_data import DatasetMetaData

DATA_DIR = Path(__file__).parent / "data"
TEMPLATE_DIR = Path(__file__).parent / "templates"
OUTPUT_PATH = Path(__file__).parents[1]

JINJA_ENV = Environment(loader=FileSystemLoader(TEMPLATE_DIR))


def render_template(template_path: Path, data_path: Path, output_path: Path):
    print(f"Generating file: {output_path}")

    # jinja expects a string, representing a relative path with forward slashes
    template_path_str = str(template_path.relative_to(TEMPLATE_DIR)).replace("\\", "/")
    template = JINJA_ENV.get_template(template_path_str)

    with open(data_path) as data_file:
        json_data = data_file.read()
    dataset_meta_data: DatasetMetaData = DatasetMetaData.schema().loads(json_data)
    # flatten attribute list
    # assign full name
    for attribute_class in dataset_meta_data.classes:
        if attribute_class.is_template:
            attribute_class.full_name = f"{attribute_class.name}<sym>"
        else:
            attribute_class.full_name = attribute_class.name
        new_attribute_list = []
        for attribute in attribute_class.attributes:
            if isinstance(attribute.names, str):
                new_attribute_list.append(attribute)
            else:
                # flatten list
                for name in attribute.names:
                    new_attribute = deepcopy(attribute)
                    new_attribute.names = name
                    new_attribute_list.append(new_attribute)
        attribute_class.attributes = new_attribute_list

    output = template.render(classes=dataset_meta_data.classes, include_guard=dataset_meta_data.include_guard, name=dataset_meta_data.name)

    with output_path.open(mode="w", encoding="utf-8") as output_file:
        output_file.write(output)


def code_gen(path: Path):
    for template_path in TEMPLATE_DIR.rglob("*.jinja"):
        template_name = template_path.with_suffix("").stem
        output_suffix = template_path.with_suffix("").suffix
        output_dir = template_path.parent.relative_to(TEMPLATE_DIR)
        for data_path in DATA_DIR.glob(f"{template_name}/*.json"):
            output_path = path / output_dir / data_path.with_suffix(output_suffix).name
            output_path.parent.mkdir(parents=True, exist_ok=True)
            render_template(template_path=template_path, data_path=data_path, output_path=output_path)


if __name__ == "__main__":
    code_gen(OUTPUT_PATH)
