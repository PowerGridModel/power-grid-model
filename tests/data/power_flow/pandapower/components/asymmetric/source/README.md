<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

## Component Test Case: Source

Test case for validation of source component for asymmetrical power flow calculations in pandapower.
- While source is present in all cases, this case tests two sources being used together.
```
source_4--node_1--line_3--node_2--line_6--node_5--source_7
```

### Modelling incompatibility with pandapower

- Source impedance is set too low. Result of source component here should be ignored