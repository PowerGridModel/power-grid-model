# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# define dataclass for meta data

from dataclasses import dataclass
from typing import Optional

from dataclasses_json import DataClassJsonMixin


@dataclass
class Attribute(DataClassJsonMixin):
    data_type: str
    names: str | list[str]
    description: str
    nan_value: Optional[str] = None


@dataclass
class AttributeClass(DataClassJsonMixin):
    name: str
    attributes: list[Attribute]
    full_attributes: Optional[list[Attribute]] = None
    base: Optional[str] = None
    is_template: bool = False
    full_name: Optional[str] = None
    specification_names: Optional[list[str]] = None
    base_attributes: Optional[dict[str, list[Attribute]]] = None


@dataclass
class DatasetMetaData(DataClassJsonMixin):
    name: str
    include_guard: str
    classes: list[AttributeClass]


@dataclass
class ObjectMapData(DataClassJsonMixin):
    names: list[str]
    class_name: str


@dataclass
class DatasetMapData(DataClassJsonMixin):
    name: str
    is_template: bool
    components: list[ObjectMapData]


@dataclass
class AllDatasetMapData(DataClassJsonMixin):
    all_datasets: list[DatasetMapData]
