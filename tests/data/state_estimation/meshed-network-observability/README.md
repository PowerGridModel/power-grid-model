<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
SPDX-License-Identifier: MPL-2.0
-->

# Observability check cases for meshed networks

This module provides test cases for various meshed network observability scenarios.
The grids in tests are illustrated in the `test_network_diagrams.svg`.

Test cases in this directory cover the following scenarios:

**01-observable-with-branch-measurement**:
Observable system with branch measurement.

**02-unobservable-with-branch-measurement**:
Unobservable system with branch measurement.

**03-observable-without-branch-measurement**:
Observable system without branch measurement (using only nodal measurements).

**04-observable-without-branch-measurement**:
Another scenario of observable system without branch measurement.

**05-observable-with-branch-measurement-disconnected**:
Observable system with disconnected components (multiple islands) and branch measurements.

**06-unobservable-with-branch-measurement-disconnected**:
Unobservable system with disconnected components (multiple islands) and branch measurements.

**07-observable-2-voltage-sensors**:
Observable system with 2 voltage sensors.

Note: In theory, it is impossible to construct an unobservable case with only nodal measurements
(i.e., without branch measurement).
