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
    \begin{eqnarray}
        I_N    & = Y_{bus}U_N
    \end{eqnarray}
$$

Where $I_N$ is the $N$ vector of source currents injected into each bus and $U_N$ is the $N$ vector of bus voltages.
The complex power delivered to bus $k$ is:

$$
    \begin{eqnarray}
        S_{k}    & =  P_k + jQ_k & = U_{k} I_{k}^{*}
    \end{eqnarray}
$$

Power flow equations are based on solving the nodal equations above to obtain the voltage magnitude and voltage angle at
each node and then obtaining the real and reactive power flow through the branches.
The following bus types can be present in the system:

- Slack bus: the reference bus with known voltage and angle; in power-grid-model referred to as the
  [source](../user_manual/components.md#source).
- Load bus: a bus with known $P$ and $Q$.
- Voltage controlled bus: a bus with known $P$ and $U$.
  Note: this bus is not supported by power-grid-model yet.

## Newton-Raphson power flow

Algorithm call: {py:class}`CalculationMethod.newton_raphson <power_grid_model.enum.CalculationMethod.newton_raphson>`

This is the traditional method for power flow calculations.
This method uses a Taylor series, ignoring the higher order terms, to solve the nonlinear set of equations iteratively:

$$
    \begin{eqnarray}
        f(x)    & =  y
    \end{eqnarray}
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
    \begin{eqnarray}
        J(i) \Delta x(i)    & =  \Delta y(i)
    \end{eqnarray}
$$

Where

$$
    \begin{eqnarray}
        \Delta x(i)    & =  x(i+1) - x(i)
        \quad\text{and}\quad
        \Delta y(i)    & =  y - f(x(i))
    \end{eqnarray}
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

## Automatic Tap Changing

This section describes the control logic and algorithmic details for power flow calculations with automatic tap
changing on regulated transformers.
For an overview of regulated power flow and strategy selection, see
[Regulated power flow with automatic tap changing](
  ../user_manual/calculations.md#regulated-power-flow-with-automatic-tap-changing).

### Control logic for power flow with automatic tap changing

The automatic tap changer iterates over all regulators with automatic tap changing control.
The control logic is described as follows:

- For each regulator, calculate the controlled voltage $U_{\text{control}}$ based on `control_side`.
- If the transformer has a `tap_pos` which is not regulated and within `tap_min` and `tap_max` bounds, skip it and
  consider the next regulator.
  In all other cases, go to the next step.
- If $U_{\text{control}}$ is outside the bounds, the tap position changes:
  - If $U_{\text{control}} < u_{\text{set}} - u_{\text{band}} / 2$, the tap position is incremented by 1 (or according
    to the `Strategy` chosen).
  - If $U_{\text{control}} > u_{\text{set}} + u_{\text{band}} / 2$, the tap position is decremented by 1 (or according
    to the `Strategy` chosen).
  - If the updated `tap_pos` reaches `tap_min` or `tap_max`, it is clamped to those values.
- Another power flow calculation is run with new tap positions for the regulated transformers.
- If the maximum number of tap changing iterations is reached, the calculation stops and raises an error of type
  `MaxIterationReached`.

#### Initialization and exploitation of regulated transformers

The initialization and exploitation of different regulated transformer types are unique and depend on whether the
regulator is automatic.
The following table specifies the behaviour of regulated transformers based on the `Strategy` chosen and whether
`tap_pos` is regulated.

|Strategy|tap_pos is regulated?|Initialization phase|Exploitation phase|
|---|---|---|---|
|`Strategy.any`|Yes|Set the tap position to `tap_nom` (default middle tap position)|Binary search (default)|
|`Strategy.any`|No|Clamp the tap position to stay within `tap_min` and `tap_max`|Do not regulate|
|`Strategy.min_voltage_tap` or `Strategy.max_voltage_tap`|Yes|Set the tap position to `tap_min` or `tap_max` respectively|Do not regulate|
|`Strategy.min_voltage_tap` or `Strategy.max_voltage_tap`|No|Clamp the tap position to stay within `tap_min` and `tap_max`|Do not regulate|

Some transformer configurations require clamping due to `regulated_object` and `tap_side` pointing to different sides of
the transformer.
For a three-winding transformer the requirements are similar to a regular transformer with the additional consideration
that the primary side tap changer regulates only the secondary or tertiary side voltages:

|Tapping on side|Regulated object|Voltage control side|`tap_side`|`regulated_object`|`control_side`|
|---|---|---|---|---|---|
|HV (0)|LV (1)|LV (1)|`BranchSide.from_side` / `Branch3Side.side_1`|`TransformerTapSide.side_1` / `Branch3Side.side_2`|`BranchSide.to_side` / `Branch3Side.side_2`|
|HV (0)|LV (1)|HV (0)|`BranchSide.from_side` / `Branch3Side.side_1`|`TransformerTapSide.side_1` / `Branch3Side.side_2`|`BranchSide.from_side` / `Branch3Side.side_1`|
|LV (1)|HV (0)|LV (1)|`BranchSide.to_side` / `Branch3Side.side_2`|`TransformerTapSide.side_2` / `Branch3Side.side_1`|`BranchSide.to_side` / `Branch3Side.side_2`|
|LV (1)|HV (0)|HV (0)|`BranchSide.to_side` / `Branch3Side.side_2`|`TransformerTapSide.side_2` / `Branch3Side.side_1`|`BranchSide.from_side` / `Branch3Side.side_1`|

#### Search methods used for tap changing optimization

By default, the algorithm uses binary search to find the optimal tap position to control voltage within the set band.
Users can also choose other search methods via `Strategy`:

|`Strategy`|Description|
|---|---|
|`Strategy.fast_any`|Very fast search (single step per iteration)|
|`Strategy.any`|Binary search (default)|
|`Strategy.min_voltage_tap` / `Strategy.max_voltage_tap`|Set to extreme tap and do not regulate|

### Regulatable voltage range outside `u_band`

Since the regulated transformer has only a limited number of tap positions, it cannot always attain the target voltage
(i.e., the controlled voltage $U_{\text{control}}$ may remain outside the set band).
The possible scenarios are:

- For `tap_min`: When increasing the tap position from `tap_min` in successive power flow calculations, the voltage
  might still be below the lower bound of the set band.
- For `tap_max`: When decreasing the tap position from `tap_max` in successive power flow calculations, the voltage
  might still be above the upper bound of the set band.

### Error type `MaxIterationReached`

If the algorithm uses binary search, since the number of tap positions is often limited to tens of taps, binary search
will find the optimal tap positions within a small number of iterations, typically under 10.
When linear search is used, however, we avoid convergence if the search continues to iterate in an oscillatory fashion.
Under normal circumstances, the converter flag should be set to true.
However, the following could be a cause for `MaxIterationReached`:

- **If all `tap_pos` are within `tap_min` and `tap_max` and converged to the targeted band**:
  - The tap changing control has found a control point inside the band.
  - The calculation may be at a saddle point, and further iteration may not result in achieving values which are closer
    to the target voltage.
  - This is often expected behaviour.
- **If no voltage control could find tap positions inside** `u_band`:
  - A transformer is constantly oscillating between two tap positions depending on directions of measurement errors.
    In this case, it's difficult for our algorithm to choose the more optimal tap position for controlled voltage.
- **If a few voltages are out of band**: The algorithm keeps oscillating due to constraint saturation at `tap_max` or
  `tap_min`.
  The regulated transformers at saturation cannot regulate further, although the voltage is outside the band.
  This is often expected behaviour.
- **If voltages are inside the band but `tap_pos` are outside `tap_min` and `tap_max` bounds**: This is a logical
  violation.
  It means the clamping inside the algorithm is not functioning.
  This is a bug and should be reported.
