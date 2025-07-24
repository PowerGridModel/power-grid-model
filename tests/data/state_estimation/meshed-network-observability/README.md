<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
SPDX-License-Identifier: MPL-2.0
-->

# Observability check cases for N-node system

This module provides test cases for an N-node system with N-1 measurements.
The grids in tests are illustrated in the `test_network_diagrams.svg`

Test cases in this directory demonstrate the only three possible scenarios:

1. Observable system with branch measurement.
2. Observable system without branch measurement (using only nodal measurements).
3. Unobservable system with branch measurement (x2).

Note: In theory, it is impossible to construct an unobservable case with only nodal measurements
(i.e., without branch measurement).
