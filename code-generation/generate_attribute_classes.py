from meta_data import AttributeClass, Attribute, HPPHeader
from pathlib import Path

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
        attribute_class: AttributeClass
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
            f.write(f"""
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

namespace power_grid_model {{ \n\n""")
            
            f.write("\n}  // namespace power_grid_model")
            f.write("#endif\n")
            f.write("// clang-format off\n")


def code_gen(header_path: Path):
    HeaderGenerator(header_name="input", header_path=header_path).generate_code()
