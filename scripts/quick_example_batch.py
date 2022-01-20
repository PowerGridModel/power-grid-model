# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import numpy as np
import pandas as pd

from power_grid_model import LoadGenType, PowerGridModel, initialize_array

"""
node_1 ---line_3--- node_2 ---line_6--- node_7
|                    |                   |
|                    |                   |
source_4          asym_load_5          asym_load_8
"""

# node
node = initialize_array("input", "node", 3)
node["id"] = [1, 2, 7]
node["u_rated"] = [10.5e3, 10.5e3, 10.5e3]

# line
line = initialize_array("input", "line", 2)
line["id"] = [3, 6]
line["from_node"] = [1, 2]
line["to_node"] = [2, 7]
line["from_status"] = [1, 1]
line["to_status"] = [1, 1]
line["r1"] = [0.25, 0.25]
line["x1"] = [0.2, 0.2]
line["c1"] = [10e-6, 10e-6]
line["tan1"] = [0.0, 0.0]
line["r0"] = [0.25, 0.25]  # zero sequence parameters
line["x0"] = [0.2, 0.2]  # zero sequence parameters
line["c0"] = [10e-6, 10e-6]  # zero sequence parameters
line["tan0"] = [0.0, 0.0]  # zero sequence parameters

# load
asym_load = initialize_array("input", "asym_load", 2)
asym_load["id"] = [4, 8]
asym_load["node"] = [2, 7]
asym_load["status"] = [1, 1]
asym_load["type"] = [LoadGenType.const_power]

# input for three phase per entry
asym_load["p_specified"] = [[2e6, 0.0, 0.0], [0.0, 1e6, 0.0]]
asym_load["q_specified"] = [[0.5e6, 0.0, 0.0], [0.0, 0.2e6, 0.0]]  # input for three phase per entry

# source
source = initialize_array("input", "source", 1)
source["id"] = [5]
source["node"] = [1]
source["status"] = [1]
source["u_ref"] = [1.0]

# input_data
input_data = {"node": node, "line": line, "asym_load": asym_load, "source": source}

# call constructor
model = PowerGridModel(input_data, system_frequency=50.0)

result = model.calculate_power_flow(symmetric=False)

print("Node Input")
print(pd.DataFrame(input_data["node"]))
print("Node Result")
print(result["node"]["u"])  # N*3 array, in symmetric calculation is this N array
print(result["asym_load"]["p"])  # N*3 array, in symmetric calculation is this N array

# batch calculation
scaler = np.linspace(0, 1, 1000)
batch_p = asym_load["p_specified"].reshape(1, 2, 3) * scaler.reshape(-1, 1, 1)
batch_load = initialize_array("update", "asym_load", (1000, 2))
batch_load["id"] = [[4, 8]]
batch_load["p_specified"] = batch_p
batch_update = {"asym_load": batch_load}

result = model.calculate_power_flow(symmetric=False, update_data=batch_update)
print(result["node"]["u"].shape)  # 1000 (scenarios) *3 (nodes) *3 (phases)
