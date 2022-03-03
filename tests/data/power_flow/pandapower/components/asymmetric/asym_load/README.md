<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->
## Component Test Case: Asymmetrical load

Test case for validation of asymmetrical load component for asymmetrical power flow calculations in pandapower.
- An asymmetrical load can be in open or closed state. 

The circuit diagram is as follows:
```
source_4--node_1--line_3--node_2--asym_load_5 (status=1)
                          node_2--asym_load_6 (status=0)
```

### Modelling incompatibility with pandapower

- Source impedance is set too low. Result of source component here should be ignored