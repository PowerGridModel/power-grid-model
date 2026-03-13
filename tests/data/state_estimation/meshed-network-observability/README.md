<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
SPDX-License-Identifier: MPL-2.0
-->

# Observability check cases for meshed networks

This module provides test cases for various meshed network observability scenarios.
The grids in tests are illustrated in the `test_network_diagrams.svg`.

Test cases in this directory cover the following scenarios:

1. **01-observable-with-branch-measurement**:
2. Observable system with branch measurement.
3. **02-unobservable-with-branch-measurement**:
4. Unobservable system with branch measurement.
5. **03-observable-without-branch-measurement**:
6. Observable system without branch measurement (using only nodal measurements).
7. **04-observable-without-branch-measurement**:
8. Another scenario of observable system without branch measurement.
9. **05-observable-with-branch-measurement-disconnected**:
10. Observable system with disconnected components (multiple islands) and branch measurements.
11. **06-unobservable-with-branch-measurement-disconnected**:
12. Unobservable system with disconnected components (multiple islands) and branch measurements.
13. **07-observable-2-voltage-sensors**:
14. Observable system with 2 voltage sensors.

Note: In theory, it is impossible to construct an unobservable case with only nodal measurements
(i.e., without branch measurement).
