<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Validation case: MATPOWER IEEE 300-bus system, Newton-Raphson divergence

This validation case shows a meshed transmission grid on which the Newton-Raphson power flow diverges
(`IterationDiverge`), although the power flow has a solution: MATPOWER solves it from a flat start.
Power Grid Model starts Newton-Raphson from a linear voltage guess, in which every load and generator is replaced by
a constant admittance.
On this grid, that guess lies outside the region where Newton-Raphson converges.

The case in `linear-start` is therefore marked as `xfail`.

## Source and scope

The source data are the MATPOWER case `case300.m`, the IEEE 300-bus test case converted from the IEEE Common Data
Format of the University of Washington Power Systems Test Case Archive.

- Original archive: https://labs.ece.uw.edu/pstca/pf300/ieee300cdf.txt
- MATPOWER case file (MATPOWER 8.1): https://github.com/MATPOWER/matpower/blob/8.1/data/case300.m
- System base power: 100 MVA

The expected output validates bus voltage magnitude and angle.

## Conversion to PGM

The conversion follows the [IEEE 14-bus case](../ieee14/README.md): all nodes have a nominal voltage of 100 kV, which
gives an impedance base of 100 ohm.
All 411 branches, lines and transformers, are represented by `generic_branch`, with the off-nominal transformer ratio
in `generic_branch.k` and the total branch charging susceptance in `generic_branch.b1`.
The case has no phase-shifting transformers.
PQ demands are represented by `sym_load`, fixed shunts by `shunt`.
The 68 generators outside the slack bus are represented by `sym_gen` together with active `voltage_regulator`
components, so that Newton-Raphson treats those buses as PV nodes.
Reactive power limits of the generators are not modelled, as in a MATPOWER power flow without `pf.enforce_q_lims`.
The slack generator at bus 7049 is represented by a stiff `source` (`sk = 1e40` VA) with `u_ref = 1.0507`.

Node ids are the MATPOWER bus numbers (up to 9533).
The other components have ids offset by 10000 (branches, numbered in MATPOWER order), 20000 (loads), 30000
(generators), 40000 (voltage regulators), 50000 (shunts) and 60000 (source), plus the bus number.

The divergence does not depend on the source strength: it also occurs with the default `sk = 1e10` VA.

## Expected output

The expected output is the MATPOWER power flow solution of `case300` (Newton-Raphson from a flat start, without
reactive power limits), with voltage angles relative to the slack bus, which is at angle 0 in PGM.
An independent polar Newton-Raphson power flow in NumPy, on the bus admittance matrix built from the MATPOWER branch
model, reproduces this solution within 2e-13 p.u. in magnitude and 1e-11 degree in angle.
