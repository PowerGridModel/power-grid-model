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

## Prefault voltages

IEC 60909 prescribes use of rated votlage of fault node on thevenian equivalent of grid impedance for calculation of
$I_k^{\prime\prime}$.
This isolates the short circuit calculations from the actual loading conditions of the grid.

Along with IEC 60909, PGM also wanted to support simulation of multiple faults occuring simultaneously to tackle certain
advanced simulations.
IEC 60909 standard does not not prescribe a method for simulating multiple simultaneous faults.

Hence, to accomodate this situation in a single calculation,
PGM had to make a design decision to deviate a bit from IEC 60909 standard.
The prefault voltages and hence corresponding $I_k^{\prime\prime}$ can be assumed to deviate by an
"equivalent transformation ratio" introduced by all tranasformers.

An example is shown below to demonstrate this effect.
In the following grid, lets say we assumed the rated voltage of the source and fault node is $u_{rated-1}$ and
$u_{rated-2}$ respectively.
The transformer component has a $u1$/$u2$ transformation ratio.
Let $z_k$ and $z_t$ be the impedance of source and transformer respectively.

```{tikz}
:alt: transformer

\draw (0,3) node[gridnode, anchor=west]{} to (1,3);
\draw [black, ultra thick] (1,2) -- (1,4);
\draw (1,3) [nos] to (2,3) [oosourcetrans] to (4,3) [nos] to (5,3);
\draw [black, ultra thick] (5,2) -- (5,4);
\draw[thick, ->] (5,1.4) +(0.05,0.5) -- +(-0.1,-0.1) -- +(0.1,0.1) -- +(0,-0.5);
```

IEC 60909 calculation should give $I_k^{\prime\prime} = \frac{u_{rated-2}}{ \sqrt{3} \cdot (z_k + z_t)}$

PGM calculates short circuit by setting source voltage as $u_{rated-1}$ and calculating the fault current. Hence
$I_k^{\prime\prime} = \frac{u_{rated-1} \cdot k}{ \sqrt{3} \cdot (z_k + z_t)}$
where $k =\frac{u1 \cdot u_{rated-2}}{u2 \cdot u_{rated-1}} $

When the voltage rating of transformer is the same as rated voltage of nodes, this factor is exactly `1.0`.
In radial grids it can be easily accumulated by chain multipliying the transformation ratio of all transformers
($k_{t1} \cdot k_{t2} \cdot k_{t3} ...$) in the path of fault to the source node.
However when multiple sources and and meshing of grid is involved it becomes overly complicated.
This factor is roughly `0.97` to `1.03` on a practical grid.
An easy approach is then to simply leave a margin of ~`1.03` in the maximal short circuit current.
