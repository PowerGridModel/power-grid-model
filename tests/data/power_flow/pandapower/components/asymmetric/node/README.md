<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

## Component Test Case: Node

Second test case for validation of node component for asymmetrical power flow calculations in Pandapower.
- This test case uses a line to validate node operation for voltages other than 0 or 1 p.u.

The circuit diagram is as follows:
```
source_4--node_1--line_3--node_2
```

### Modelling incompatibility with pandapower

- Source impedance is set too low. Result of source component here should be ignored