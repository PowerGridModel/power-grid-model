<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Short Circuit Algorithm Details

This page provides detailed mathematical descriptions of the short circuit algorithms implemented in power-grid-model.
For a summary and guidance, see [Calculations](../user_manual/calculations.md#short-circuit-calculation-algorithms).

## Short Circuit Equations

In the short circuit calculation, the following equations are solved with border conditions of faults added as
constraints.

$$
I_N = Y_{bus}U_N
$$

This gives the initial symmetrical short circuit current ($I_k^{\prime\prime}$) for a fault.
This quantity is then used to derive almost all further calculations of short circuit studies applications.

```{note}
Short-circuit calculations are currently implemented in the phase (abc) domain and therefore require a grounded network, similar to asymmetric power flow calculations.
A potential implementation using the sequence (0-1-2) domain is expected to remove this limitation.
```

## IEC 60909 short circuit calculation

Algorithm call: {py:class}`CalculationMethod.iec60909 <power_grid_model.enum.CalculationMethod.iec60909>`

The assumptions used for calculations in power-grid-model are aligned to the ones mentioned in
[IEC 60909](https://webstore.iec.ch/publication/24100).

- The state of the grid with respect to loads and generations are ignored for the short circuit calculation.
  (Note: Shunt admittances are included in calculation.)
- The pre-fault voltage is considered in the calculation and is calculated based on the grid parameters and topology.
  (Excl. loads and generation)
- The calculations are assumed to be time-independent.
  (Voltages are sine throughout with the fault occurring at a zero crossing of the voltage, the complexity of rotating
  machines and harmonics are neglected, etc.)
- To account for the different operational conditions, a voltage scaling factor of `c` is applied to the voltage source
  while running short circuit calculation function.
  The factor `c` is determined by the nominal voltage of the node that the source is connected to and the API option to
  calculate the `minimum` or `maximum` short circuit currents.
  The table to derive `c` according to IEC 60909 is shown below.

| Algorithm      | c_max | c_min |
| -------------- | ----- | ----- |
| `U_nom` <= 1kV | 1.10  | 0.95  |
| `U_nom` > 1kV  | 1.10  | 1.00  |

```{note}
In the IEC 609090 standard, there is a difference in `c` (for `U_nom` <= 1kV) for systems with a voltage tolerance of 6%
and 10%.
In power-grid-model we only use the value for a 10% voltage tolerance.
```

There are {py:class}`4 types <power_grid_model.enum.FaultType>` of fault situations that can occur in the grid, along
with the following possible combinations of the {py:class}`associated phases <power_grid_model.enum.FaultType>`:

- Three-phase to ground: `abc`
- Single phase to ground: `a`, `b`, `c`
- Two phase: `ab`, `bc`, `ac`
- Two phase to ground: `ab`, `bc`, `ac`
