# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import numpy as np

from power_grid_model import ComponentType, DatasetType, initialize_array, power_grid_meta_data
from power_grid_model.enum import ComponentAttributeFilterOptions


def test_nan_scalar():
    assert np.isnan(power_grid_meta_data[DatasetType.input][ComponentType.node].nan_scalar["u_rated"])


def test_initialize_array():
    arr = initialize_array(DatasetType.input, ComponentType.node, 3)
    assert arr.shape == (3,)
    assert np.all(np.isnan(arr["u_rated"]))
    arr_2d = initialize_array(DatasetType.input, ComponentType.node, (2, 3))
    assert arr_2d.shape == (2, 3)
    assert np.all(np.isnan(arr_2d["u_rated"]))


def test_initialize_array__columnar():
    some_attributes = ["id", "from_node", "r1"]
    actual = initialize_array(DatasetType.input, ComponentType.line, 3, attributes=some_attributes)
    assert isinstance(actual, dict)
    assert actual.keys() == set(some_attributes)
    assert all(v.shape == (3,) for v in actual.values())
    assert np.all(np.isnan(actual["r1"]))

    actual_2d = initialize_array(DatasetType.input, ComponentType.line, (2, 3), attributes=some_attributes)
    assert actual.keys() == set(some_attributes)
    assert all(v.shape == (2, 3) for v in actual_2d.values())
    assert np.all(np.isnan(actual_2d["r1"]))

    all_attributes_data = initialize_array(
        DatasetType.input, ComponentType.node, (2, 3), attributes=ComponentAttributeFilterOptions.everything
    )
    assert set(["id", "u_rated"]) == all_attributes_data.keys()


def test_sensor_meta_data():
    sensors = [
        ComponentType.sym_voltage_sensor,
        ComponentType.asym_voltage_sensor,
        ComponentType.sym_power_sensor,
        ComponentType.asym_power_sensor,
    ]
    input_voltage = ["u_measured", "u_angle_measured", "u_sigma"]
    output_voltage = ["u_residual", "u_angle_residual"]
    input_power = ["p_measured", "q_measured", "power_sigma"]
    output_power = ["p_residual", "q_residual"]
    for sensor in sensors:
        for meta_type in [DatasetType.input, DatasetType.update, DatasetType.sym_output, DatasetType.asym_output]:
            meta_data = power_grid_meta_data[meta_type]
            assert sensor in meta_data
            meta_data_sensor = meta_data[sensor]
            attr_names = meta_data_sensor.dtype_dict["names"]
            assert "id" in attr_names
            # check specific attributes
            if "voltage" in sensor:
                if "output" in meta_type:
                    expected_attrs = output_voltage
                else:
                    expected_attrs = input_voltage
            else:
                if "output" in meta_type:
                    expected_attrs = output_power
                else:
                    expected_attrs = input_power

            for name in expected_attrs:
                assert name in attr_names


def test_dict_like_access():
    assert (
        power_grid_meta_data[DatasetType.input][ComponentType.node].dtype
        == power_grid_meta_data[DatasetType.input][ComponentType.node]["dtype"]
    )


def test_all_datasets():
    assert set(power_grid_meta_data.keys()) == {
        DatasetType.input,
        DatasetType.update,
        DatasetType.sym_output,
        DatasetType.asym_output,
        DatasetType.sc_output,
    }
