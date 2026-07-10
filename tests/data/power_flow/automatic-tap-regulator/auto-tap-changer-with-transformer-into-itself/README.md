<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Test case with transformer into itself

A transformer into itself is a transformer that is connected to the same node on both ends. It acts as a shunt:

$$
\begin{aligned}
   |Y_{\text{shunt}}| &= i_0 p.u. \\
   \mathrm{Re}(Y_{\text{shunt}}) &= \frac{p_0}{s_n} p.u. \\
   \mathrm{Im}(Y_{\text{shunt}}) &= -\sqrt{|Y_{\text{shunt}}|^2-\mathrm{Re}(Y_{\text{shunt}})^2} \\
\end{aligned}
$$

Note that $Z_{\text{series}}$ is equivalent to $0$ (no net power flow).

This is tested as follows:

1. Create some complicated grid.
2. Add 4 components to the grid: 2 transformers into itself and 2 shunts on the same node.
   The attributes are chosen such, that $Y_{\text{shunt}}$ is the same for both components:
   1. $c_{1,\text{transformer}} = \frac{b_{1,\text{shunt}}}{2 \pi f}$
   2. $\tan \delta_{1,\text{transformer}} = \frac{b_{1,\text{shunt}}}{g_{1,\text{shunt}}}$
   3. $c_{0,\text{transformer}} = \frac{b_{0,\text{shunt}}}{2 \pi f}$
   4. $\tan \delta_{0,\text{transformer}} = \frac{b_{0,\text{shunt}}}{g_{0,\text{shunt}}}$
3. Create a batch update with the following scenarios:
   1. Benchmark case: the transformers are fully disconnected, the shunts are connected.
   2. Actual test case: the transformers into itself are connected on both ends, the shunts are disconnected.
4. The output of both scenarios should be the same, except for the transformers into itself and shunts.
   For those, it is true that
   $p_{\text{transformer},\text{from}} = p_{\text{transformer},\text{to}} = \frac{1}{2}p_{\text{shunt}}$,
   $q_{\text{transformer},\text{from}} = q_{\text{transformer},\text{to}} = \frac{1}{2}q_{\text{shunt}}$,
   $s_{\text{transformer},\text{from}} = s_{\text{transformer},\text{to}} = \frac{1}{2}s_{\text{shunt}}$ and
   $i_{\text{transformer},\text{from}} = i_{\text{transformer},\text{to}} = \frac{1}{2}i_{\text{shunt}}$.
   To verify, the output can be generated from a normal run and eyeballed to compare.
