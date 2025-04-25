<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Component Test Case: Transformer

Test case for validation of the transformer component for symmetrical power flow calculations in pandapower.

- A transformer can be 4 states, closed on both ends, open on both ends and open on any one end.

The tap changing functionality is tested using a batch calculation for various tap positions.

The circuit diagram is as follows:

```txt
source_7--node_1--transformer_3--node_2              (Transformer from_status=to_status=1)
          node_1--transformer_4--node_2--load_6      (Transformer from_status=0)
          node_1--transformer_5--node_2              (Transformer to_status=0)
          node_1--transformer_8--node_2              (Transformer from_status=to_status=0)
```
