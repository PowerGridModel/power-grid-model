# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import pytest

from power_grid_model import AttributeType, ComponentType, DatasetType, power_grid_meta_data


def assert_data_type(pgm_meta_data_types, data_type):
    pgm_types = [pgm_type for pgm_type in pgm_meta_data_types]
    pgm_types.sort()
    generated_types = [member.value for member in data_type]
    generated_types.sort()
    assert pgm_types == generated_types


def test_power_grid_data_types():
    assert_data_type(power_grid_meta_data, DatasetType)


@pytest.mark.parametrize("dataset", [dataset for dataset in DatasetType])
def test_power_grid_components(dataset: DatasetType):
    assert_data_type(power_grid_meta_data[dataset], ComponentType)


def test_power_grid_component_attributes():
    attributes = set()
    for dataset in DatasetType:
        for component in ComponentType:
            attributes.update(power_grid_meta_data[dataset][component].dtype_dict["names"])
    pgm_attributes = list(sorted(attributes))
    assert_data_type(pgm_attributes, AttributeType)
