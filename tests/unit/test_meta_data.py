# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import numpy as np

from power_grid_model import (
    AttributeType as AT,
    ComponentType as CT,
    DatasetType,
    attribute_dtype,
    attribute_empty_value,
    initialize_array,
    power_grid_meta_data,
)


def test_nan_scalar():
    assert np.isnan(power_grid_meta_data[DatasetType.input][CT.node].nan_scalar[AT.u_rated])


def test_initialize_array():
    arr = initialize_array(DatasetType.input, CT.node, 3)
    assert arr.shape == (3,)
    assert np.all(np.isnan(arr[AT.u_rated]))
    arr_2d = initialize_array(DatasetType.input, CT.node, (2, 3))
    assert arr_2d.shape == (2, 3)
    assert np.all(np.isnan(arr_2d[AT.u_rated]))


def test_attribute_dtype():
    assert attribute_dtype(DatasetType.input, CT.node, AT.u_rated) == np.float64
    assert attribute_dtype(DatasetType.input, CT.node, AT.id) == np.int32


def test_attribute_empty_value():
    empty_value = attribute_empty_value(DatasetType.input, CT.node, AT.u_rated)
    assert np.isnan(empty_value)
    empty_value = attribute_empty_value(DatasetType.input, CT.node, AT.id)
    assert empty_value == np.iinfo(np.int32).min


def test_sensor_meta_data():
    sensors = [
        CT.sym_voltage_sensor,
        CT.asym_voltage_sensor,
        CT.sym_power_sensor,
        CT.asym_power_sensor,
    ]
    input_voltage = [AT.u_measured, AT.u_angle_measured, AT.u_sigma]
    output_voltage = [AT.u_residual, AT.u_angle_residual]
    input_power = [AT.p_measured, AT.q_measured, AT.power_sigma]
    output_power = [AT.p_residual, AT.q_residual]
    for sensor in sensors:
        for meta_type in [DatasetType.input, DatasetType.update, DatasetType.sym_output, DatasetType.asym_output]:
            meta_data = power_grid_meta_data[meta_type]
            assert sensor in meta_data
            meta_data_sensor = meta_data[sensor]
            attr_names = meta_data_sensor.dtype_dict["names"]
            assert AT.id in attr_names
            # check specific attributes
            if "voltage" in sensor:
                expected_attrs = output_voltage if "output" in meta_type else input_voltage
            else:
                expected_attrs = output_power if "output" in meta_type else input_power

            for name in expected_attrs:
                assert name in attr_names


def test_dict_like_access():
    assert (
        power_grid_meta_data[DatasetType.input][CT.node].dtype
        == power_grid_meta_data[DatasetType.input][CT.node]["dtype"]
    )


def test_all_datasets():
    assert set(power_grid_meta_data.keys()) == {
        "input",
        "update",
        "sym_output",
        "asym_output",
        "sc_output",
        DatasetType.input,
        DatasetType.update,
        DatasetType.sym_output,
        DatasetType.asym_output,
        DatasetType.sc_output,
    }
