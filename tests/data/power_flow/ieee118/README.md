<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Validation case: IEEE 118-bus system

This validation case models the public IEEE 118-bus power-flow test system as a symmetrical Newton-Raphson power-flow
case for Power Grid Model (PGM).

## Source and scope

The source data are the MATPOWER case `case118.m`, the IEEE 118-bus test case converted from the IEEE Common Data
Format of the University of Washington Power Systems Test Case Archive.

- Original archive: https://labs.ece.uw.edu/pstca/pf118/ieee118cdf.txt
- MATPOWER case file (MATPOWER 8.1): https://github.com/MATPOWER/matpower/blob/8.1/data/case118.m
- System base power: 100 MVA

The case has 118 buses, 186 branches (175 lines and 11 transformers, 9 of them with an off-nominal ratio, no phase
shifters), 99 loads, 53 voltage-controlled generators, 14 fixed shunts and one slack bus (bus 69).
The expected output validates bus voltage magnitude and angle.

## Conversion to PGM

The conversion follows the [IEEE 14-bus case](../ieee14/README.md) and the
[MATPOWER 300-bus case](../matpower-case300/README.md): all nodes have a nominal voltage of 100 kV, which gives an
impedance base of 100 ohm on the 100 MVA system base.
All 186 branches, lines and transformers, are represented by `generic_branch`, with the off-nominal transformer ratio
in `generic_branch.k` (`k = 1` for lines) and the total branch charging susceptance in `generic_branch.b1`.
PQ demands are represented by `sym_load`, fixed shunts by `shunt`.
The 53 generators outside the slack bus are represented by `sym_gen` together with active `voltage_regulator`
components at the generator voltage setpoint, so that Newton-Raphson treats those buses as PV nodes.
Reactive power limits of the generators are not modelled, as in a MATPOWER power flow without `pf.enforce_q_lims`.
The slack generator at bus 69 is represented by a stiff `source` (`sk = 1e40` VA) with `u_ref = 1.035`.

Node ids are the MATPOWER bus numbers.
Lines have ids 10001 to 10175 and transformers 11001 to 11011, each numbered in MATPOWER branch order.
The other components have ids offset by 20000 (loads), 30000 (generators), 40000 (voltage regulators), 50000 (shunts)
and 60000 (source), plus the bus number.

## Expected output

The expected output is the Newton-Raphson power flow solution of `case118` from a flat start, without reactive power
limits, with voltage angles relative to the slack bus, which is at angle 0 in PGM.
It was computed with an independent polar Newton-Raphson power flow in NumPy on the bus admittance matrix built from
the MATPOWER branch model, solved to a power mismatch below 1e-13 p.u.
pandapower 3.5.6, converting the same MATPOWER data with its own converter (`from_ppc`) and solving with
Newton-Raphson without reactive power limits, reproduces this solution within 2e-15 p.u. in magnitude and 6e-15 rad
in angle.

The voltages stored in `case118.m` are not used: they are rounded to three decimals in magnitude and two decimals in
degrees, and they come from the original archive solution, which differs from the no-limit solution by up to 0.017
p.u. (bus 30).
Without reactive power limits, six generators (buses 19, 32, 34, 92, 103 and 105) end up outside their reactive power
limits.

From the default linear start, PGM converges in 8 iterations on this case; from a flat or average source start it
converges in 5.
