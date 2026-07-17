<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Test case with line into itself

A line into itself is a line that is connected to the same node on both ends. It acts as a shunt:

$$
Y_{\text{shunt}} = 2 \pi f c \left(\tan \delta +\mathrm{j}\right)
$$

Note that $Z_{\text{series}}$ is equivalent to $0$ (no net power flow).

This is tested as follows:

1. Create some complicated grid.
2. Add 8 components to the grid: 2 lines into itself and 2 shunts on the same node, as well as sensors on them.
   The attributes are chosen such, that $Y_{\text{shunt}}$ is the same for both components:
   1. $c_{1,\text{line}} = \frac{b_{1,\text{shunt}}}{2 \pi f}$
   2. $\tan \delta_{1,\text{line}} = \frac{b_{1,\text{shunt}}}{g_{1,\text{shunt}}}$
   3. $c_{0,\text{line}} = \frac{b_{0,\text{shunt}}}{2 \pi f}$
   4. $\tan \delta_{0,\text{line}} = \frac{b_{0,\text{shunt}}}{g_{0,\text{shunt}}}$
3. Create a batch update with the following scenarios:
   1. Benchmark case: the lines are fully disconnected, the shunts are connected.
      Idem for the associated sensors.
   2. Actual test case: the lines into itself are connected on both ends, the shunts are disconnected.
      Idem for the associated sensors.
4. Run a power flow - use the results as input for the power and voltage sensors.
5. The output of both scenarios should be the same and can be generated from a run and eyeballed to compare.
