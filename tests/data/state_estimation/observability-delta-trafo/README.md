<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Delta-transformer observability

Reproducer for [issue #1668](https://github.com/PowerGridModel/power-grid-model/issues/1668).

We use a simple three-phase, two-bus network.

```
                         P/Q sensor  V sensor
                            (P/Q)      (V)
                              :         :
                              :         :
(S)-----====-----(( ))--------+-------====--------[Load/generator]
Source  MV bus   Transformer          LV bus
        solver   &Delta; / grounded Y      solver bus 0
        bus 1
```

The network has an MV source, a Delta/grounded-Y transformer, and an LV load.
The only measurements are LV phase-to-ground voltage magnitudes and per-phase
P/Q at the transformer's LV terminal. The values are synthetic.

In asymmetric state estimation, only the phase-to-phase voltages of the MV side can be calculated.
The MV phase-to-ground voltages cannot be uniquely determined. Therefore, the network is not observable.
However, PGM's radial observability check accepts this network.
The [meshed counterpart](../observability-delta-trafo-meshed/README.md) adds a loop to LV node 2 to examine the separate meshed sufficient-condition check.

Both `iterative_linear` and `newton_raphson` should raise `NotObservableError`. On the tested build,
`iterative_linear` raises `IterationDiverge` and `newton_raphson` raises
`SparseMatrixError`. These failures are marked `xfail` and may vary by platform.
Remove the `xfail` entries once the observability check is fixed.

`asym_output.json` records the manual calculation results from `CALCULATION.md`, given the assumption
that MV zero-sequence voltage are zero. This assumption could be a solution to the [Delta trafo observability issue](https://github.com/PowerGridModel/power-grid-model/issues/1668).

Run from the repository root with the development environment activated:

```sh
python -m pytest tests/unit/test_0Z_model_validation.py -k observability-delta-trafo --no-cov -rx
```
