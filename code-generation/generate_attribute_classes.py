# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from io import TextIOWrapper
from pathlib import Path
from typing import List

from meta_data import Attribute, AttributeClass, HPPHeader

ATTRIBUTE_CLASS_PATH = Path(__file__).parent / "attribute_classes"


class HeaderGenerator:
    header_meta_data: HPPHeader
    header_file: Path

    def __init__(self, header_name: str, header_path: Path) -> None:
        with open(ATTRIBUTE_CLASS_PATH / f"{header_name}.json") as f:
            json_string = f.read()
            self.header_meta_data = HPPHeader.schema().loads(json_string)
        self.header_file = header_path / f"{header_name}.hpp"

        # flatten attribute list
        for attribute_class in self.header_meta_data.classes:
            new_attribute_list = []
            for attribute in attribute_class.attributes:
                if isinstance(attribute.names, str):
                    new_attribute_list.append(attribute)
                else:
                    # flatten list
                    for name in attribute.names:
                        new_attribute = attribute.copy()
                        new_attribute.names = name
                        new_attribute_list.append(new_attribute)

    def generate_code(self):
        print(f"Generate header file: {self.header_file}")
        with open(self.header_file, "w") as f:
            # begin
            f.write(
                f"""
// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT mondify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_{self.header_meta_data.include_guard}_HPP
#define POWER_GRID_MODEL_AUXILIARY_{self.header_meta_data.include_guard}_HPP

#include \"../enum.hpp\"
#include \"../power_grid_model.hpp\"
#include \"../three_phase_tensor.hpp\"
#include \"meta_data.hpp\"

namespace power_grid_model {{ \n\n"""
            )
            # generate classes
            for attribute_class in self.header_meta_data.classes:
                self.generate_class(attribute_class=attribute_class, f=f)
            # generate get_meta functors
            f.write("\n\n// template specialization functors to get meta data\n")
            f.write("namespace meta_data {\n\n")
            for attribute_class in self.header_meta_data.classes:
                self.generate_get_meta(attribute_class=attribute_class, f=f)
            f.write("} // namespace meta_data\n\n")
            # ending
            f.write("\n}  // namespace power_grid_model")
            f.write("#endif\n")
            f.write("// clang-format off\n")

    def generate_class(self, attribute_class: AttributeClass, f: TextIOWrapper):
        # begin class
        if attribute_class.is_template:
            f.write("template <bool sym>\n")
        if attribute_class.base is not None:
            f.write(f"struct {attribute_class.name} : {attribute_class.base} {{\n")
        else:
            f.write(f"struct {attribute_class.name} {{\n")
        # attributes
        self.generate_attributes(attributes=attribute_class.attributes, f=f)
        # get meta function
        # self.generate_get_meta(attribute_class=attribute_class, f=f)
        # end class and alias
        f.write("};\n")
        if attribute_class.is_template:
            f.write(f"using Sym{attribute_class.name} = {attribute_class.name}<true>;\n")
            f.write(f"using Asym{attribute_class.name} = {attribute_class.name}<false>;\n")
        f.write("\n")

    def generate_attributes(self, attributes: List[Attribute], f: TextIOWrapper):
        for attribute in attributes:
            f.write(f"    {attribute.data_type} {attribute.names};")
            if attribute.description is not None:
                f.write(f" // {attribute.description}")
            f.write("\n")

    def generate_get_meta(self, attribute_class: AttributeClass, f: TextIOWrapper):
        # begin
        if attribute_class.is_template:
            # partial specialization for symmetric template
            f.write("template<bool sym>\n")
            f.write(f"struct get_meta<{attribute_class.name}<sym>> {{\n")
        else:
            # full specialization
            f.write("template<>\n")
            f.write(f"struct get_meta<{attribute_class.name}> {{\n")
        # write function
        f.write("    MetaData operator() () {\n")
        f.write("        MetaData meta{};\n")
        f.write(f'        meta.name = "{attribute_class.name}";\n')
        f.write(f"        meta.size = sizeof({attribute_class.name});\n")
        f.write(f"        meta.alignment = alignof({attribute_class.name});\n")
        if attribute_class.base is not None:
            f.write(f"        meta.attributes = get_meta<{attribute_class.base}>{{}}().attributes;\n")
        # all attributes
        for attribute in attribute_class.attributes:
            f.write(
                f'        meta.attributes.push_back(get_data_attribute<&{attribute_class.name}::{attribute.names}>("{attribute.names}"));\n'
            )
        f.write("    }\n")
        # closing
        f.write("};\n\n")


def code_gen(header_path: Path):
    HeaderGenerator(header_name="input", header_path=header_path).generate_code()
