# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import pandas as pd

from power_grid_model import ComponentType, DatasetType, LoadGenType, PowerGridModel, initialize_array

# node
node = initialize_array(DatasetType.input, ComponentType.node, 2)
# The following is also supported
# node = initialize_array("input", "node", 2)
node["id"] = [1, 2]
node["u_rated"] = [10.5e3, 10.5e3]

# line
line = initialize_array(DatasetType.input, ComponentType.line, 1)
line["id"] = [3]
line["from_node"] = [1]
line["to_node"] = [2]
line["from_status"] = [1]
line["to_status"] = [1]
line["r1"] = [0.25]
line["x1"] = [0.2]
line["c1"] = [10e-6]
line["tan1"] = [0.0]
line["i_n"] = [1000]

# load
sym_load = initialize_array(DatasetType.input, ComponentType.sym_load, 1)
sym_load["id"] = [4]
sym_load["node"] = [2]
sym_load["status"] = [1]
sym_load["type"] = [LoadGenType.const_power]
sym_load["p_specified"] = [2e6]
sym_load["q_specified"] = [0.5e6]

# source
source = initialize_array(DatasetType.input, ComponentType.source, 1)
source["id"] = [5]
source["node"] = [1]
source["status"] = [1]
source["u_ref"] = [1.0]

# input_data
input_data = {
    ComponentType.node: node,
    ComponentType.line: line,
    ComponentType.sym_load: sym_load,
    ComponentType.source: source,
}

# call constructor
model = PowerGridModel(input_data, system_frequency=50.0)

result = model.calculate_power_flow()

print("Node Input")
print(pd.DataFrame(input_data[ComponentType.node]))
print("Node Result")
print(pd.DataFrame(result[ComponentType.node]))
