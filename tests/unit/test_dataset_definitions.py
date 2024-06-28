# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import pytest

from power_grid_model import ComponentType, DatasetType, power_grid_meta_data


def test_power_grid_data_types():
    power_grid_data_types = [data_type for data_type in power_grid_meta_data]
    gen_power_grid_data_types = [member.value for member in DatasetType]
    power_grid_data_types.sort()
    gen_power_grid_data_types.sort()
    assert power_grid_data_types == gen_power_grid_data_types


def test_power_grid_components():
    power_grid_components = [component for component in power_grid_meta_data[DatasetType.input]]
    gen_power_grid_components = [member.value for member in ComponentType]
    power_grid_components.sort()
    gen_power_grid_components.sort()
    assert power_grid_components == gen_power_grid_components
