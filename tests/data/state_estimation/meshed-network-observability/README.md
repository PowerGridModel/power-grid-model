<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
SPDX-License-Identifier: MPL-2.0
-->

# Observability check cases for N-node system

This module provides test cases for meshed network observability checks.
The grids in tests are illustrated in the `test_network_diagrams.svg`

## Test Cases

The following test cases are included:

1. **01-observable-with-branch-measurement**: Observable system with branch power measurement
2. **02-unobservable-with-branch-measurement**: Unobservable system with branch power measurement
3. **03-observable-without-branch-measurement**: Observable system using only nodal measurements
4. **04-observable-without-branch-measurement**: Another observable system using only nodal measurements
5. **05-observable-with-branch-measurement-disconnected**: Observable system with disconnected components
and branch measurement
6. **06-unobservable-with-branch-measurement-disconnected**: Unobservable system with disconnected components
and branch measurement
7. **07-observable-2-voltage-sensors**: Meshed network with two voltage phasor sensors (see note below)

Note: In theory, it is impossible to construct an unobservable case with only nodal measurements
(i.e., without branch measurement).

## Multiple Voltage Phasor Sensors in Meshed Networks

Test case `07-observable-2-voltage-sensors` represents a meshed network with two voltage phasor sensors
(voltage measurements with both magnitude and angle). This configuration is currently treated as
**non-observable** due to a known limitation: the current meshed network sufficient-condition
implementation cannot handle multiple voltage phasor sensors.

### Unit Test Coverage

This edge case is comprehensively covered by unit tests in `tests/cpp_unit_tests/test_observability.cpp`
(specifically the test case "Test ObservabilityResult - use_perturbation with non-observable network").
The unit tests verify:

- Networks with `n_voltage_phasor_sensors > 1 && !is_radial` are correctly identified as non-observable
- The `ObservabilityResult.is_observable` flag is set to `false`
- The `use_perturbation()` method returns `false` (no perturbation applied to non-observable networks)
- The specific code path in `observability_check()` that triggers early return for this condition

### Why No "Expected to Fail" Integration Test?

An explicit integration test marked as "expected to fail" is **not necessary** because:

1. **Unit test coverage is sufficient**: The unit tests provide precise, maintainable verification at the
   appropriate level of granularity.
2. **Maintenance burden**: "Expected to fail" tests require special infrastructure and documentation,
   and risk being forgotten when the limitation is eventually addressed.
3. **Rare use case**: Multiple voltage phasor sensors (with precise angle measurements) in meshed
   networks are uncommon in real-world applications.

When support for multiple voltage phasors in meshed networks is implemented, the unit tests can be
updated accordingly, and test case `07-observable-2-voltage-sensors` can be populated with expected
output values.
