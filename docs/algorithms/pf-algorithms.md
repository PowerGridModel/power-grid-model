<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Power Flow Algorithm Details

This page provides detailed mathematical descriptions of the power flow algorithms implemented in power-grid-model.
For a summary and guidance on choosing the right algorithm, see
[Calculations](../user_manual/calculations.md#power-flow-algorithms).

## Nodal Equations

The nodal equations of a power system network can be written as:

$$
I_N = Y_{bus}U_N
$$

Where $I_N$ is the $N$ vector of source currents injected into each bus and $U_N$ is the $N$ vector of bus voltages.
The complex power delivered to bus $k$ is:

$$
S_{k} = P_k + jQ_k = U_{k} I_{k}^{*}
$$

Power flow equations are based on solving the nodal equations above to obtain the voltage magnitude and voltage angle at
each node and then obtaining the real and reactive power flow through the branches.
The following bus types can be present in the system:

- Slack bus: the reference bus with known voltage and angle; in power-grid-model referred to as the
  [source](../user_manual/components.md#source).
- Load bus: a bus with known $P$ and $Q$.
- Voltage controlled bus: a bus with known $P$ and $U$.
  Note: this bus is not supported by power-grid-model yet.

```{note}
Asymmetric power flow calculations require the network to have a reference to ground. For details and internal solution of asymmetric floating grids calculations in power-grid-model, please refer to[Symmetric vs asymmetric calculations](../user_manual/calculations.md#short-circuit-calculations). 
```

## Newton-Raphson power flow

Algorithm call: {py:class}`CalculationMethod.newton_raphson <power_grid_model.enum.CalculationMethod.newton_raphson>`

This is the traditional method for power flow calculations.
This method uses a Taylor series, ignoring the higher order terms, to solve the nonlinear set of equations iteratively:

$$
f(x) =  y
$$

Where:

$$
\begin{aligned}
    x    =  \begin{bmatrix}
            \delta \\
            U
            \end{bmatrix} =
            \begin{bmatrix}
            \delta_2 \\
            \vdots \\
            \delta_N \\
            U_2 \\
            \vdots \\
            U_N
            \end{bmatrix}
    \quad\text{and}\quad
    y    =  \begin{bmatrix}
            P \\
            Q
            \end{bmatrix} =
            \begin{bmatrix}
            P_2 \\
            \vdots \\
            P_N \\
            Q_2 \\
            \vdots \\
            Q_N
            \end{bmatrix}
    \quad\text{and}\quad
    f(x) =  \begin{bmatrix}
            P(x) \\
            Q(x)
            \end{bmatrix} =
            \begin{bmatrix}
            P_{2}(x) \\
            \vdots \\
            P_{N}(x) \\
            Q_{2}(x) \\
            \vdots \\
            Q_{N}(x)
            \end{bmatrix}
\end{aligned}
$$

As can be seen in the equations above $\delta_1$ and $V_1$ are omitted, because they are known for the slack bus.
In each iteration $i$ the following equation is solved:

$$
J(i) \Delta x(i) =  \Delta y(i)
$$

Where

$$
\begin{aligned}
    \Delta x(i)    & =  x(i+1) - x(i)
    \quad\text{and}\quad
    \Delta y(i)    & =  y - f(x(i))
\end{aligned}
$$

$J$ is the [Jacobian](https://en.wikipedia.org/wiki/Jacobian_matrix_and_determinant), a matrix with all partial
derivatives of
$\dfrac{\partial P}{\partial \delta}$, $\dfrac{\partial P}{\partial U}$, $\dfrac{\partial Q}{\partial \delta}$
and $\dfrac{\partial Q}{\partial U}$.

For each iteration the following steps are executed:

- Compute $\Delta y(i)$
- Compute the Jacobian $J(i)$
- Using LU decomposition, solve $J(i) \Delta x(i)  =  \Delta y(i)$ for $\Delta x(i)$
- Compute $x(i+1)$ from $\Delta x(i) =  x(i+1) - x(i)$

## Iterative current power flow

Algorithm call:
{py:class}`CalculationMethod.iterative_current <power_grid_model.enum.CalculationMethod.iterative_current>`

This algorithm is a Jacobi-like method for powerflow analysis.
It has a linear convergence rate as opposed to the quadratic convergence rate in the
[Newton-Raphson](#newton-raphson-power-flow) method.
This means that more iterations is needed to achieve similar convergence.
Additionally, [Newton-Raphson](#newton-raphson-power-flow) will be more robust converging in case of greater meshed
configurations.
Nevertheless, the iterative current algorithm will be faster most of the time.

The algorithm is as follows:

1. Build $Y_{bus}$ matrix
2. Initialization of $U_N^0$ to $1$ plus the intrinsic phase shift of transformers
3. Calculate injected currents: $I_N^i$ for $i^{th}$ iteration.
   The injected currents are calculated as per ZIP model of loads and generation using $U_N$:
   $I_N = \overline{S_{Z}} \cdot U_{N} + \overline{(\frac{S_{I}}{U_{N}})} \cdot |U_{N}| +\overline{(\frac{S_{P}}{U_N})}$
4. Solve linear equation: $YU_N^i = I_N^i$
5. Check convergence: If maximum voltage deviation from the previous iteration is greater than the tolerance setting
   (i.e., $u^{(i-1)}_\sigma > u_\epsilon$), then go back to step 3.

The iterative current algorithm only needs to calculate injected currents before solving linear equations.
This is more straightforward than calculating the Jacobian, which was done in the Newton-Raphson algorithm.

Factorizing the matrix of linear equation is the most computationally heavy task.
The $Y_{bus}$ matrix here does not change across iterations which means it only needs to be factorized once to solve the
linear equations in all iterations.
The $Y_{bus}$ matrix also remains unchanged in certain batch calculations like timeseries calculations.

## Linear power flow

Algorithm call: {py:class}`CalculationMethod.linear <power_grid_model.enum.CalculationMethod.linear>`

This is an approximation method where we assume that all loads and generations are of constant impedance type regardless
of their actual {py:class}`LoadGenType <power_grid_model.enum.LoadGenType>`.
By doing so, we obtain huge performance benefits as the computation required is equivalent to a single iteration of the
iterative methods.
It will be more accurate when most of the load/generation types are of constant impedance or the actual node voltages
are close to 1 p.u.
When all the load/generation types are of constant impedance, the [Linear](#linear-power-flow) method will be the
fastest without loss of accuracy.
Therefore power-grid-model will use this method regardless of the input provided by the user in this case.

The algorithm is as follows:

1. Assume injected currents by loads $I_N=0$ since we model loads/generation as impedance.
   Admittance of each load/generation is calculated from rated power as $y=-\overline{S}$
2. Build $Y{bus}$.
   Add the admittances of loads/generation to the diagonal elements.
3. Solve linear equation: $YU_N = I_N$ for $U_N$

## Linear current power flow

Algorithm call: {py:class}`CalculationMethod.linear_current <power_grid_model.enum.CalculationMethod.linear_current>`

**This algorithm is essentially a single iteration of [iterative current power flow](#iterative-current-power-flow).**

This approximation method will give better results when most of the load/generation types resemble constant current.
Similar to [iterative current](#iterative-current-power-flow), batch calculations like timeseries will also be faster.
Same reason applies here: the $Y_{bus}$ matrix does not change across batches and as a result the same factorization
ould be used.

In practical grids most loads and generations correspond to the constant power type.
Linear current would give a better approximation than [Linear](#linear-power-flow) in such case.
This is because we approximate the load as current instead of impedance.
There is a correlation in voltage error of approximation with respect to the actual voltage for all approximations.
They are most accurate when the actual voltages are close to 1 p.u. and the error increases as we deviate from this
level.
When we approximate the load as impedance at 1 p.u., the voltage error has quadratic relation to the actual voltage.
When it is approximated as a current at 1 p.u., the voltage error is only linearly dependent in comparison.
