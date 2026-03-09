# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import json

import numpy as np

from power_grid_model import PowerGridModel
from power_grid_model._core.dataset_definitions import ComponentAttribute, ComponentType
from power_grid_model.utils import json_deserialize

input_data = {
    "version": "1.0",
    "type": "input",
    "is_batch": False,
    "attributes": {},
    "data": {
        ComponentType.sym_load: [
            {
                ComponentAttribute.id: 2,
                ComponentAttribute.node: 0,
                ComponentAttribute.status: 1,
                ComponentAttribute.type: 0,
                ComponentAttribute.p_specified: 0,
                ComponentAttribute.q_specified: 0,
            }
        ],
        ComponentType.source: [
            {
                ComponentAttribute.id: 1,
                ComponentAttribute.node: 0,
                ComponentAttribute.status: 1,
                ComponentAttribute.u_ref: 1,
                ComponentAttribute.sk: 1e20,
            }
        ],
        ComponentType.node: [{ComponentAttribute.id: 0, ComponentAttribute.u_rated: 10e3}],
    },
}

input_data_json = json.dumps(input_data)


def test_multi_dimensional_batch():
    input_dataset = json_deserialize(input_data_json)
    pgm = PowerGridModel(input_dataset)

    u_rated = 10e3
    u_ref = np.array([0.9, 1.0, 1.1], dtype=np.float64).reshape(-1, 1)
    p_specified = np.array([1e6, 2e6, 3e6, 4e6], dtype=np.float64).reshape(-1, 1)
    q_specified = np.array([0.1e6, 0.2e6, 0.3e6, 0.4e6, 0.5e6], dtype=np.float64).reshape(-1, 1)
    i_source_ref = np.abs(p_specified.reshape(1, -1, 1) + 1j * q_specified.reshape(1, 1, -1)) / (
        u_ref.reshape(-1, 1, 1) * u_rated * np.sqrt(3)
    )
    i_source_ref = i_source_ref.ravel()

    u_ref_batch = {ComponentType.source: {ComponentAttribute.u_ref: u_ref}}
    p_specified_batch = {ComponentType.sym_load: {ComponentAttribute.p_specified: p_specified}}
    q_specified_batch = {ComponentType.sym_load: {ComponentAttribute.q_specified: q_specified}}

    result = pgm.calculate_power_flow(
        update_data=[u_ref_batch, p_specified_batch, q_specified_batch],
        output_component_types={ComponentType.source: [ComponentAttribute.i]},
    )

    assert np.allclose(result[ComponentType.source][ComponentAttribute.i].ravel(), i_source_ref)
