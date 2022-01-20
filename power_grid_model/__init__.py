# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
The Power Grid Model module

Provides
  1. The Power Grid Model
  2. Helper functions
  3. Enumeration types

Example usage
-------------

Required imports for basic usage
>>> from power_grid_model import initialize_array
>>> from power_grid_model import PowerGridModel

Setting up example input data
>>> node = initialize_array('input', 'node', 2)
>>> node['id'] = [1, 2]
>>> node['u_rated'] = [10.5e3, 10.5e3]

>>> line = initialize_array('input', 'line', 1)
>>> line['id'] = [3]
>>> line['from_node'] = [1]
>>> line['to_node'] = [2]
>>> line['from_status'] = [1]
>>> line['to_status'] = [1]
>>> line['r1'] = [0.25]
>>> line['x1'] = [0.2]
>>> line['c1'] = [10e-6]
>>> line['tan1'] = [0.0]
>>> line['i_n'] = [1000]

>>> sym_load = initialize_array('input', 'sym_load', 1)
>>> sym_load['id'] = [4]
>>> sym_load['node'] = [2]
>>> sym_load['status'] = [1]
>>> sym_load['type'] = [LoadGenType.const_power]
>>> sym_load['p_specified'] = [2e6]
>>> sym_load['q_specified'] = [0.5e6]

>>> source = initialize_array('input', 'source', 1)
>>> source['id'] = [5]
>>> source['node'] = [1]
>>> source['status'] = [1]
>>> source['u_ref'] = [1.0]

Putting it all together
>>> input_data = {
>>>   'node': node,
>>>   'line': line,
>>>   'sym_load': sym_load,
>>>   'source': source
>>> }

Initializing the model with the dummy data
>>> model = PowerGridModel(input_data, system_frequency=50.0)
"""

# pylint: disable=no-name-in-module

# Helper functions
# Power Grid metadata
# Power Grid Model
from ._power_grid_core import PowerGridModel, initialize_array, power_grid_meta_data

# Enumerations
from .enum import BranchSide, CalculationMethod, CalculationType, LoadGenType, MeasuredTerminalType, WindingType
