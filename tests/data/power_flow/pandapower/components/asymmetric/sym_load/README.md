<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->
## Component Test Case: Symmetrical Load

Test case for validation of a symmetrical load component  for asymmetrical power flow calculations in pandapower.
- A symmetrical load can be in open or closed state. 
- It can be of 3 types: constant power, constant impedance and constant current.

The circuit diagram is as follows:
```
source_4--node_1--line_3--node_2--sym_load_6    (const_power)
                          node_2--sym_load_7    (status=0)
```

### Modelling incompatibility with pandapower

- Source impedance is set too low. Result of source component here should be ignored
- Only constant power implementation is possible in pandapower for asymmetrical calculations.
