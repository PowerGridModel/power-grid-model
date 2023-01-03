# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

# define dataclass for meta data

from dataclasses import dataclass
from dataclasses_json import DataClassJsonMixin
from typing import Optional, Dict, List, Union


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
    template_alias: Optional[Dict[bool, str]] = None


@dataclass
class HPPHeader(DataClassJsonMixin):
    name: str
    classes: List[AttributeClass]
