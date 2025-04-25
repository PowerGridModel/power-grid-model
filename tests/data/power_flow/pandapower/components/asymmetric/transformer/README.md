<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Component Test Case: Transformer

Test case for validation of the transformer component for asymmetrical power flow calculations in pandapower.

- A transformer can be 4 states, closed on both ends, open on both ends and open on any one end.

The tap changing functionality is tested using a batch calculation for various tap positions.

The circuit diagram is as follows:

```txt
source_7--node_1--transformer_3--node_2              (Transformer from_status=to_status=1)
```

### Modelling incompatibility with pandapower

- Source impedance is set too low. Result of source component here should be ignored
- Asymmetrical calculations are possible only for grounded network transformer in pandapower.
Hence open cases are not evaluated.
- Relaxed tolerance parameters are used in asymmetric calculation
because only 'T' transformer model is available in pandapower while power-grid-model uses 'pi' model.
