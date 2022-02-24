<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->
## Component Test Case: Symmetrical generator

Test case for validation of a symmetrical generator component for symmetrical power flow calculations in pandapower.
- A symmetrical generator can be in open or closed state. 
- It can be of 3 types: constant power, constant impedance and constant current.

The circuit diagram is as follows:
```
source_4--node_1--line_3--node_2--sym_gen_6    (const_power)
                          node_2--sym_gen_7    (const_current)
                          node_2--sym_gen_8    (const_impedance)
                          node_2--sym_gen_9    (status=0)
```
