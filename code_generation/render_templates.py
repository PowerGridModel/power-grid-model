# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path

from jinja2 import Environment, FileSystemLoader
from meta_data import HPPHeader

DATA_DIR = Path(__file__).parent / "attribute_classes"
TEMPLATE_DIR = Path(__file__).parent / "templates"


def render_template(template_path: Path, data_path: Path, output_path: Path):
    print(f"Generating file: {output_path}")

    environment = Environment(loader=FileSystemLoader(TEMPLATE_DIR))
    template = environment.get_template(str(template_path.relative_to(TEMPLATE_DIR)))

    with open(data_path) as data_file:
        json_data = data_file.read()
    header: HPPHeader = HPPHeader.schema().loads(json_data)

    output = template.render(classes=header.classes)

    with output_path.open(mode="w", encoding="utf-8") as output_file:
        output_file.write(output)


def code_gen(path: Path):
    for data_path in DATA_DIR.glob("*.json"):
        for template_path in TEMPLATE_DIR.glob("*.jinja"):
            output_path = path / template_path.stem / data_path.with_suffix(template_path.suffixes[0]).name
            output_path.parent.mkdir(parents=True, exist_ok=True)
            render_template(template_path=template_path, data_path=data_path, output_path=output_path)
