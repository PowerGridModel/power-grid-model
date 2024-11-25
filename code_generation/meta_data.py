# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# define dataclass for meta data

from dataclasses import dataclass

from dataclasses_json import DataClassJsonMixin


@dataclass
class Attribute(DataClassJsonMixin):
    data_type: str
    names: str | list[str]
    description: str
    nan_value: str | None = None


@dataclass
class AttributeClass(DataClassJsonMixin):
    name: str
    attributes: list[Attribute]
    full_attributes: list[Attribute] | None = None
    base: str | None = None
    is_template: bool = False
    full_name: str | None = None
    specification_names: list[str] | None = None
    base_attributes: dict[str, list[Attribute]] | None = None


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
