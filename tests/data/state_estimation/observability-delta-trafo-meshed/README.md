<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Delta-transformer observability with an LV loop

Meshed counterpart of [the radial case](../observability-delta-trafo/README.md)
for [issue #1668](https://github.com/PowerGridModel/power-grid-model/issues/1668).
PGM checks radial and meshed networks separately. This case covers
[`sufficient_condition_meshed_without_voltage_phasor`](../../../../power_grid_model_c/power_grid_model/include/power_grid_model/math_solver/observability.hpp).

We extend the radial network with the measured LV loop `2 -- 8 -- 9 -- 2` and
load 13 at node 9, retaining the original components and measurements.

```text
Source -- MV 1 -- Dyn5 -- LV 2 -- load 5
                         / \
                        /   \
                      LV 8--LV 9 -- load 13 (P/Q measured)
```

LV voltage magnitudes, P/Q at the `from` terminal of each loop line, and load 13's P/Q are
measured. This network can pass the topological observability check.

`asym_output.json` has an empty `data` object because this test expects `NotObservableError`.
The current solver failures are marked `xfail`. Run both cases with:

```sh
python -m pytest tests/unit/test_0Z_model_validation.py -k observability-delta-trafo --no-cov -rx
```
