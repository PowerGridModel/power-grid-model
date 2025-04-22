<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Component Test Case: Shunt

Test case for validation of shunt component for asymmetrical power flow calculations in pandapower.

- A shunt can be in 2 states: open or closed.

The circuit diagram is as follows:

```txt
 source_1--node_1--line_3--node_2--shunt_6      (status=1)
                           node_2--shunt_7      (status=0)
```

## Modelling incompatibility with pandapower

- Source impedance is set too low. Result of source component here should be ignored
