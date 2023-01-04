# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

# define dataclass for meta data

from dataclasses import dataclass
from typing import Dict, List, Optional, Union

from dataclasses_json import DataClassJsonMixin


@dataclass
class Attribute(DataClassJsonMixin):
    data_type: str
    names: Union[str, List[str]]
    description: str


@dataclass
class AttributeClass(DataClassJsonMixin):
    name: str
    attributes: List[Attribute]
    base: Optional[str] = None
    is_template: bool = False


@dataclass
class HPPHeader(DataClassJsonMixin):
    name: str
    include_guard: str
    classes: List[AttributeClass]
