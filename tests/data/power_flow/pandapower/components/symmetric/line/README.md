<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->
## Component Test Case: Line

Test case for validation of the line component for symmetrical power flow calculations in pandapower. 
- A line can be 4 states, closed on both ends, open on both ends and open on any one end.

The circuit diagram is as follows:
```
source_4--node_1--line_3--node_2--line_6--node_5              (Line from_status=to_status=1)
                          node_2--line_7--node_5--load_9      (Line from_status=0)
                          node_2--line_8--node_5              (Line to_status=0)
                          node_2--line_10--node_5             (Line from_status=to_status=0)
```
