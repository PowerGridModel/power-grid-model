<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Power sensor from Global Current Sensor test case

This test case is used to validate the output values from the global current sensor. To verify the correctness of the results, the input from the global current sensor is transformed into power sensor input. At the time of writing, the power sensor component has already been validated. However, keep in mind that for state estimation using the iterative linear method, there may be small discrepancies between the results obtained from the current sensor and those from the power sensor due to the approximate nature of the calculation method.

## Conversion from Global Current Sensor to Power Sensor

To convert the input of a global current sensor to a power sensor, use the following relations:

$$
S = I_g^* V \sqrt(3), \quad
p = \Re(S), \quad
q = \Im(S),
$$

where:

- $S$ is the complex power,
- $I_g$ is the global complex current,
- $V$ is the complex voltage at the adjacent node where the sensor is located,
- $p$ and $q$ are the measured active and reactive powers, respectively,
- $\sqrt(3)$ comes from the neutral-line change of frame of reference.
