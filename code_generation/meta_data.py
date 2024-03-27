# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# define dataclass for meta data

from dataclasses import dataclass
from typing import List, Optional, Union

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
    full_attributes: Optional[List[Attribute]] = None
    base: Optional[str] = None
    is_template: bool = False
    full_name: Optional[str] = None


@dataclass
class DatasetMetaData(DataClassJsonMixin):
    name: str
    include_guard: str
    classes: List[AttributeClass]


@dataclass
class ObjectMapData(DataClassJsonMixin):
    names: List[str]
    class_name: str


@dataclass
class DatasetMapData(DataClassJsonMixin):
    name: str
    is_template: bool
    components: List[ObjectMapData]


@dataclass
class AllDatasetMapData(DataClassJsonMixin):
    all_datasets: List[DatasetMapData]
