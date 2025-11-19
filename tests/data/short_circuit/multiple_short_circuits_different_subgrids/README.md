<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Test Case: Multiple short circuits (different subgrids)

Test case in which there are 2 separate, disconnected subgrids, both of which contain a fault.

The circuit diagram is as follows:

```txt
        fault_9
            |
source_7--node_1--line_5--node_2

        fault_10
            |
source_8--node_3--line_6--node_4
```
