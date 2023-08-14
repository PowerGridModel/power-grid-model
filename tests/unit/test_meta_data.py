# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import numpy as np

from power_grid_model import initialize_array, power_grid_meta_data


def test_nan_scalar():
    assert np.isnan(power_grid_meta_data["input"]["node"].nan_scalar["u_rated"])


def test_initialize_array():
    arr = initialize_array("input", "node", 3)
    assert arr.shape == (3,)
    assert np.all(np.isnan(arr["u_rated"]))
    arr_2d = initialize_array("input", "node", (2, 3))
    assert arr_2d.shape == (2, 3)
    assert np.all(np.isnan(arr_2d["u_rated"]))


def test_sensor_meta_data():
    sensors = ["sym_voltage_sensor", "asym_voltage_sensor", "sym_power_sensor", "asym_power_sensor"]
    input_voltage = ["u_measured", "u_angle_measured", "u_sigma"]
    output_voltage = ["u_residual", "u_angle_residual"]
    input_power = ["p_measured", "q_measured", "power_sigma"]
    output_power = ["p_residual", "q_residual"]
    for sensor in sensors:
        for meta_type in ["input", "update", "sym_output", "asym_output"]:
            meta_data = power_grid_meta_data[meta_type]
            # comp_names = list(meta_data.keys())
            # assert sensor in comp_names
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
    assert power_grid_meta_data["input"]["node"].dtype == power_grid_meta_data["input"]["node"]["dtype"]


def test_all_datasets():
    assert set(power_grid_meta_data.keys()) == {
        "input",
        "update",
        "sym_output",
        "asym_output",
        "sc_output",
    }
