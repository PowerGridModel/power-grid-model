<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

## Component Test Case: Basic node

Test case for validation of source component for symmetrical power flow calculations in pandapower.
- While source is present in all cases, this case tests two sources being used together.

The circuit diagram is as follows:
```
source_4--node_1--line_3--node_2--line_6--node_5--source_7
```
