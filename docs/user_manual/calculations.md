<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Calculations

## Calculation types

With power-grid-model it is possible to perform three different types of calculations:

- [Power flow](#power-flow-algorithms): a "what-if" scenario calculation.
  This calculation can be performed by using the
  {py:class}`calculate_power_flow <power_grid_model.PowerGridModel.calculate_power_flow>` method.
  An example of usage of the power-flow calculation function is given in
  [Power flow Example](../examples/Power%20Flow%20Example.ipynb)
- [State estimation](#state-estimation-algorithms): a statistical method that calculates the most probabilistic state of
  the grid, given sensor values with an uncertainty.
  This calculation can be performed by using the
  {py:class}`calculate_state_estimation <power_grid_model.PowerGridModel.calculate_state_estimation>` method.
  An example of usage of the power-flow calculation function is given in
  [State Estimation Example](../examples/State%20Estimation%20Example.ipynb)
- [Short circuit](#short-circuit-calculation-algorithms): a "what-if" scenario calculation with short circuit entries.
  This calculation can be performed by using the
  {py:class}`calculate_short_circuit <power_grid_model.PowerGridModel.calculate_short_circuit>` method.

### Calculation types explained

#### Power flow

Power flow is a "what-if" based grid calculation that will calculate the node voltages and the power flow through the
branches, based on assumed load/generation profiles.
Some typical use-cases are network planning and contingency analysis.

Input:

- Network data: topology + component attributes
- Assumed load/generation profile

Output:

- Node voltage magnitude and angle
- Power flow through branches

See [Power flow algorithms](#power-flow-algorithms) for detailed documentation on the calculation methods.

##### Regulated power flow

For most power flow calculations, the grid is fixed as the user dictates.
However, in practice, the grid often contains regulators for certain components.
When including those regulators in the calculations, the grid may be optimized according to the power flow results and
the behaviour of the regulators.

See [Regulated power flow calculations](#regulated-power-flow-calculations) for detailed documentation on regulated
power flow calculations.

#### State estimation

State estimation is a statistical calculation method that determines the most probable state of the grid, based on
network data and measurements.
Here, measurements can be power flow or voltage values with certain kind of uncertainty, which were either measured,
estimated or forecasted.

Input:

- Network data: topology + component attributes
- Power flow / voltage measurements with uncertainty

Output:

- Node voltage magnitude and angle
- Power flow through branches
- Deviation between measurement values and estimated state

In order to perform a state estimation, the system should be observable.
If the system is not observable, the calculation will raise either a `NotObservableError` or a `SparseMatrixError`.
In short, meeting the requirement of observability indicates that the system is either an overdetermined system (when
the number of independent measurements is larger than the number of unknowns) or an exactly determined system (the
number of independent measurements equals the number of unknowns).
For each node, there are two unknowns, `u` and `u_angle`.
Due to the relative nature of `u_angle` (relevant only in systems with at least two nodes), in total the following
conditions should be met:

$$
    \begin{eqnarray}
        n_{measurements}    & >= & n_{unknowns}
    \end{eqnarray}
$$

Where

$$
    \begin{eqnarray}
        n_{unknowns}    & = & 2 & \cdot & n_{nodes} - 1
    \end{eqnarray}
$$

The number of measurements can be found by taking the sum of the following:

- number of nodes with a voltage sensor with magnitude only
- two times the number of nodes with a voltage sensor with magnitude and angle
- two times the number of nodes without appliances connected
- two times the number of nodes where all connected appliances are measured by a power sensor
- two times the number of branches with a power sensor and/or a current sensor

```{note}
Having enough measurements does not necessarily mean that the system is observable.
The location of the measurements is also of importance, i.e., the measurements should be topologically independent.
Additionally, there should be at least one voltage measurement.
```

```{note}
Global angle current measurements require at least one voltage angle measurement to make sense.
See also the [current sensor component documentation](./components.md#global-angle-current-sensors).
```

```{note}
It is not possible to mix [power sensors](./components.md#generic-power-sensor) with
[current sensors](./components.md#generic-current-sensor) on the same terminal of the same component.
It is also not possible to mix
[current sensors with global angle measurement type](./components.md#generic-current-sensor) with
[current sensors with local angle measurement type](./components.md#generic-current-sensor) on the same terminal of the
same component.
However, such mixing of sensor types is allowed as long as they are on different terminals.
```

```{warning}
The [iterative linear](#iterative-linear-state-estimation) and [Newton-Raphson](#newton-raphson-state-estimation) state
estimation algorithms will assume angles to be zero by default (see the details about voltage sensors).
In observable systems this helps better outputting correct results.
On the other hand with unobservable systems, exceptions raised from calculations due to faulty results will be
prevented.
```

##### Necessary observability condition

Based on the requirements of observability mentioned above, users need to satisfy at least the following conditions for
state estimation calculation in `power-grid-model`.

- `n_voltage_sensor >= 1`
- If no voltage phasor sensors are available, then both the following conditions shall be satisfied:
  - There are no global angle current sensors.
  - `n_unique_power_or_current_sensor >= n_bus - 1`.
- Otherwise (if there are voltage phasor sensors available, one will be reserved as reference),
the following condition shall be satisfied:
  - `n_unique_power_or_current_sensor + n_voltage_sensor_with_phasor - 1 >= n_bus - 1`

`n_unique_power_or_current_sensor` can be calculated as sum of following:

- Zero injection or zero power flow constraint if present for all nodes.
- Complete injections for all nodes: All appliances in a node are measured or a node injection sensor is present.
Either of them counts as one.
- Any sensor on a `Branch` for all branches: Parallel branches with either side of measurements count as one.
- All `Branch3` sensors.

##### Sufficient observability condition

The condition check above only checks the necessary condition for observability.
When the measurements are not independent enough, the system may still be unobservable even if the necessary condition
is met.
It is rather complicated to do a full sufficient and necessary observability check in generic cases.
However, `power-grid-model` performs the sufficient condition check when the underlying grid is radial.

In this case, the validation of the independent measurements is rather straightforward.
If the system is not observable, the calculation will raise a `NotObservableError` instead of `SparseMatrixError`.

#### Short circuit calculations

Short circuit calculation is carried out to analyze the worst case scenario when a fault has occurred.
The currents flowing through branches and node voltages are calculated.
Some typical use-cases are selection or design of components like conductors or breakers and power system protection,
e.g. relay co-ordination.

Input:

- Network data: topology + component attributes
- Fault type and impedance.
- In the API call: choose between `minimum` and `maximum` voltage scaling to calculate the minimum or maximum short
- circuit currents (according to IEC 60909).

Output:

- Node voltage magnitude and angle
- Current flowing through branches and fault.

#### Common calculations

Power flowing through a branch is calculated by voltage and current for any type of calculations in the following way:

$$
    \begin{eqnarray}
        \underline{S_{branch-side}} = \sqrt{3} \cdot \underline{U_{LL-side-node}} \cdot \underline{I_{branch-side}}
    \end{eqnarray}
$$

These quantities are in complex form.
Hence, they can be constructed by PGM output attributes in the following way:

- For  $\underline{U}$ of nodes, `u` is the magnitude and `u_angle` is the angle.
  Also the line to neutral voltage can be converted into line to line voltage by $ U_{LN} = U_{LL} / \sqrt{3}$.
  Check [Node Steady State Output](components.md#steady-state-output) to find out which quantity is relevant in your
  calculation.

- For  $\underline{I}$ of branches, `i_side` is the magnitude.
  Its angle can be found from `p_side` and `q_side` by:
  $\arctan(\frac{P_{side} + j \cdot Q_{side}}{\underline{U}})^{*}$.
  The `side` here can be `from`, `to` for {hoverxreftooltip}`user_manual/components:Branch`es, `1`, `2`, `3` for
  {hoverxreftooltip}`user_manual/components:Branch3`s.

### Power flow algorithms

Two types of power flow algorithms are implemented in power-grid-model; iterative algorithms (Newton-Raphson / Iterative
current) and linear algorithms (Linear / Linear current).
Iterative methods are more accurate and should thus be selected when an accurate solution is required.
Linear approximation methods are many times faster than the iterative methods, but are generally less accurate.
They can be used where approximate solutions are acceptable.
Their accuracy is not explicitly calculated and may vary a lot.
The user should have an intuition of their applicability based on the input grid configuration.
The table below can be used to pick the right algorithm.
Below the table a more in depth explanation is given for each algorithm.

At the moment, the following power flow algorithms are implemented.

| Algorithm                                          | Default  | Speed    | Accuracy | Algorithm call                                                                                              |
| -------------------------------------------------- | -------- | -------- | -------- | ----------------------------------------------------------------------------------------------------------- |
| [Newton-Raphson](#newton-raphson-power-flow)       | &#10004; |          | &#10004; | {py:class}`CalculationMethod.newton_raphson <power_grid_model.enum.CalculationMethod.newton_raphson>`       |
| [Iterative current](#iterative-current-power-flow) |          |          | &#10004; | {py:class}`CalculationMethod.iterative_current <power_grid_model.enum.CalculationMethod.iterative_current>` |
| [Linear](#linear-power-flow)                       |          | &#10004; |          | {py:class}`CalculationMethod.linear <power_grid_model.enum.CalculationMethod.linear>`                       |
| [Linear current](#linear-current-power-flow)       |          | &#10004; |          | {py:class}`CalculationMethod.linear_current <power_grid_model.enum.CalculationMethod.linear_current>`       |

```{note}
By default, the [Newton-Raphson](#newton-raphson-power-flow) method is used.
```

```{note}
When all the load/generation types are of constant impedance, the [Linear](#linear-power-flow) method will be the
fastest without loss of accuracy.
Therefore power-grid-model will use this method regardless of the input provided by the user in this case.
```

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
  [source](./components.md#source).
- Load bus: a bus with known $P$ and $Q$.
- Voltage controlled bus: a bus with known $P$ and $U$.
  Note: this bus is not supported by power-grid-model yet.

#### Newton-Raphson power flow

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
    \begin{eqnarray}
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
    \end{eqnarray}
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

#### Iterative current power flow

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

#### Linear power flow

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

#### Linear current power flow

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

### State estimation algorithms

Weighted least squares (WLS) state estimation can be performed with power-grid-model.
Given a grid with $N_b$ buses the state variable column vector is defined as below.

$$
    \begin{eqnarray}
            \underline{U}     =     \begin{bmatrix}
                            \underline{U}_1 \\
                            \underline{U}_2 \\
                            \vdots \\
                            \underline{U}_{N_{b}}
                        \end{bmatrix}
    \end{eqnarray}
$$

Where $\underline{U}_i$ is the complex voltage phasor of the i-th bus.

The goal of WLS state estimation is to evaluate the state variable with the highest likelihood given (pseudo)
measurement input, by solving:

$$
  \begin{eqnarray}
    \min r(\underline{U}) = \dfrac{1}{2} (f(\underline{U}) - \underline{z})^H W (f(\underline{U}) - \underline{z})
  \end{eqnarray}
$$

Where:

$$
    \begin{eqnarray}
        \underline{x}     =  \begin{bmatrix}
                \underline{x}_1 \\
                \underline{x}_2 \\
                \vdots \\
                \underline{x}_{N_{m}}
                \end{bmatrix} =
                f(\underline{U})
        \quad\text{and}\quad
        \underline{z}     =  \begin{bmatrix}
                \underline{z}_1 \\
                \underline{z}_2 \\
                \vdots \\
                \underline{z}_{N_{m}}
                \end{bmatrix}
        \quad\text{and}\quad
        W  = \Sigma^{-1} =  \begin{bmatrix}
                \sigma_1^2 & 0 & \cdots & 0 \\
                0 & \sigma_2^2 & \cdots & 0 \\
                \vdots & \vdots & \ddots & \vdots \\
                0 & 0 & \cdots & \sigma_{N_{m}}^2
                \end{bmatrix} ^{-1} =
                \begin{bmatrix}
                w_1 & 0 & \cdots & 0 \\
                0 & w_2 & \cdots & 0 \\
                \vdots & \vdots & \ddots & \vdots \\
                0 & 0 & \cdots & w_{N_{m}}
                \end{bmatrix}
    \end{eqnarray}
$$

Where $\underline{x}_i$ is the real value of the i-th measured quantity in complex form, $\underline{z}_i$ is the i-th
measured value in complex form, $\sigma_i$ is the normalized standard deviation of the measurement error of the i-th
measurement, $\Sigma$ is the normalized covariance matrix and $W$ is the weighting factor matrix.

At the moment, the following state estimation algorithms are implemented.

| Algorithm                                              | Default  | Speed    | Accuracy | Algorithm call                                                                                            |
| ------------------------------------------------------ | -------- | -------- | -------- | --------------------------------------------------------------------------------------------------------- |
| [Iterative linear](#iterative-linear-state-estimation) | &#10004; | &#10004; |          | {py:class}`CalculationMethod.iterative_linear <power_grid_model.enum.CalculationMethod.iterative_linear>` |
| [Newton-Raphson](#newton-raphson-state-estimation)     |          |          | &#10004; | {py:class}`CalculationMethod.newton_raphson <power_grid_model.enum.CalculationMethod.newton_raphson>`     |

```{note}
By default, the [iterative linear](#iterative-linear-state-estimation) method is used.
```

#### State estimation measurement aggregation

There can be multiple sensors measuring the same physical quantity.
For example, there can be multiple voltage sensors on the same bus.
The measurement data can be merged into one virtual measurement using a Kalman filter:

$$
    \begin{eqnarray}
            z = \dfrac{\sum_{k=1}^{N_{sensor}} z_k \sigma_k^{-2}}{\sum_{k=1}^{N_{sensor}} \sigma_k^{-2}}
    \end{eqnarray}
$$

Where $z_k$ and $\sigma_k$ are the measured value and standard deviation of individual measurements.

Multiple appliance measurements (power measurements) on one bus are aggregated as the total injection at the bus:

$$
    \begin{eqnarray}
            \underline{S} = \sum_{k=1}^{N_{appliance}} \underline{S}_k
            \quad\text{and}\quad
            \sigma_P^2 = \sum_{k=1}^{N_{appliance}} \sigma_{P,k}^2
            \quad\text{and}\quad
            \sigma_Q^2 = \sum_{k=1}^{N_{appliance}} \sigma_{Q,k}^2
    \end{eqnarray}
$$

Where $S_k$ and $\sigma_{P,k}$ and $\sigma_{Q,k}$ are the measured value and the standard deviation of the individual
appliances.

```{note}
It is not possible to mix [power sensors](./components.md#generic-power-sensor) with
[current sensors](./components.md#generic-current-sensor) on the same terminal of the same component.
It is also not possible to mix
[current sensors with global angle measurement type](./components.md#global-angle-current-sensors) with
[current sensors with local angle measurement type](./components.md#local-angle-current-sensors) on the same terminal 
of the same component.
However, such mixing of sensor types is allowed as long as they are on different terminals.
```

#### State estimate sensor transformations

Sometimes, measurements need to be transformed between coordinate spaces.
For example, current measurements are in polar coordinates (magnitude and angle), but it is beneficial to transform them
to separate real and imaginary components per phase, with each their own variances.
Variances of the results of such transformations are estimated using the common linearization approximation.
E.g., for a real random variable $X$, the random variable $Y = F\left(X\right)$, with $F$ sufficiently well-behaved,
exists.
The variances of $Y$ can then be estimated from the variance of $X$ using
$\text{Var}\left(Y\right) \approx \text{Var}\left(X\right) \left\|\frac{\delta f}{\delta X}\right\|^2$

This approach generalizes to extension fields and to more dimensions.
Both generalizations are required for current sensors.
The current phasor is described by a complex random variable, which can be decomposed into a real and an imaginary
component, or into a magnitude and a phasor.
Asymmetric sensors and asymmetric calculations, on the other hand, require multidimensional approaches.
The generalization is done as follows.

Let $A$ and $B$ be fields (or rings) that may be multidimensional and that may be extension fields.
Let $\boldsymbol{X}$ be a random variable with codomain $A$ with components $X_i$.
Let $\boldsymbol{Y} = \boldsymbol{F}\left(\boldsymbol{X}\right)$ be a random variable with codomain $B$, with
$\boldsymbol{F}$ sufficiently well-behaved,
so that its components $Y_j = F_j\left(\boldsymbol{X}\right)$ are well-behaved.
Then the variances $\text{Var}\left(Y_i\right)$ of the components of $\boldsymbol{Y}$ can be estimated in terms of the
variances $\text{Var}\left(X_j\right)$ of the components of $\boldsymbol{X}$ using the following function.

$$
\text{Var}\left(Y_j\right)
    = \text{Var}\left(F_j\left(\boldsymbol{X}\right)\right)
    \approx \sum_i \text{Var}\left(X_i\right) \left\|\frac{\delta F_j}{\delta x_i}\left(\boldsymbol{X}\right)\right\|^2
$$

The following illustrates how this works for `sym_current_sensor`s in symmetric calculations.
See also [the full mathematical workout](https://github.com/PowerGridModel/power-grid-model/issues/547).

$$
   \begin{eqnarray}
        & \mathrm{Re}\left\{I\right\} = I \cos\theta \\
        & \mathrm{Im}\left\{I\right\} = I \sin\theta \\
        & \text{Var}\left(\mathrm{Re}\left\{I\right\}\right) =
            \sigma_i^2 \cos^2\theta + I^2 \sigma_{\theta}^2\sin^2\theta \\
        & \text{Var}\left(\mathrm{Im}\left\{I\right\}\right) =
            \sigma_i^2 \sin^2\theta + I^2 \sigma_{\theta}^2\cos^2\theta
   \end{eqnarray}
$$

#### Iterative linear state estimation

Algorithm call:
{py:class}`CalculationMethod.iterative_linear <power_grid_model.enum.CalculationMethod.iterative_linear>`

Linear WLS requires all measurements to be linear and only handles voltage phasor measurements including a phase angle,
as well as complex current phasor measurements.
This is only possible when all measurements are phasor unit measurements, which is not realistic in distribution grids.
Therefore, traditional measurements are linearized prior to running the algorithm.

- Bus voltage: Given that the phase shift in the distribution grid is very small, it is assumed that the angle shift is
  zero plus the intrinsic phase shift of transformers.
  For a certain bus `i`, the voltage magnitude measured at that bus is translated into a voltage phasor, where
  $\theta_i$ is the intrinsic transformer phase shift:

$$
    \begin{eqnarray}
            \underline{U}_i = U_i \cdot e^{j \theta_i}
    \end{eqnarray}
$$

- Branch current with global angle: The global angle current measurement captures the phase offset relative to the same
  predetermined reference phase against which the voltage angle is measured.
  It is not sufficient to use the default zero-phase reference offset, because the reference point is not uniquely
  determined when there are no voltage angle measurements, resulting in ambiguities.
  As a result, using any global angle current sensor in the grid requires at least one voltage phasor measurement (with
  angle).

$$
    \begin{eqnarray}
            \underline{I} = I_i \cdot e^{j \theta_i}
    \end{eqnarray}
$$

- Branch current with local angle: Sometimes, (accurate) voltage measurements are not available for a branch, which
  means no power measurement is possible.
  While it is not straightforward to use magnitude-only measurements of currents in calculations, current phasor
  measurements can in fact be used in cases where it is possible to obtain a reasonably accurate measurement of the
  current amplitude and the relative phase angle to the voltage.
  In this way, we still have enough information for the state estimation.
  The resulting local current phasor $\underline{I}_{\text{local}}$ can be translated to the global current phasor (see
  above) for iterative linear state estimation.
  Given the measured (linearized) voltage phasor, the current phasor is calculated as follows:

$$
    \begin{equation}
        \underline{I} = \underline{I}_{\text{local}}^{*} \frac{\underline{U}}{|\underline{U}|}
        = \underline{I}_{\text{local}}^{*} \cdot e^{j \theta}
    \end{equation}
$$

where $\underline{U}$ is either measured voltage magnitude at the bus or assumed unity magnitude, with the intrinsic
phase shift.
$\theta$ is the phase angle of the voltage.

- Branch/shunt power flow: To translate the power flow to a complex current phasor, the voltage at the terminal should
  also be measured, otherwise the nominal voltage with zero angle is used as an estimation.
  Given the measured (linearized) voltage phasor, the current phasor is calculated as follows:

$$
    \begin{eqnarray}
            \underline{I} = (\underline{S}/\underline{U})^*
    \end{eqnarray}
$$

- Bus power injection: Similar as above, to translate the power flow to a complex current phasor, if the bus voltage is
  not measured, the nominal voltage with zero angle will be used as an estimation.
  The current phasor is calculated as follows:

$$
    \begin{eqnarray}
            \underline{I} = (\underline{S}/\underline{U})^*
    \end{eqnarray}
$$

The aggregated apparent power flow is considered as a single measurement, with variance
$\sigma_S^2 = \sigma_P^2 + \sigma_Q^2$.

The assumption made in the linearization of measurements introduces a system error to the algorithm, because the phase
shifts of bus voltages are ignored in the input measurement data.
This error is corrected by applying an iterative approach to the linear WLS algorithm.
In each iteration, the resulted voltage phase angle will be applied as the phase shift of the measured voltage phasor
for the next iteration:

- Initialization: let $\underline{U}^{(k)}$ be the column vector of the estimated voltage phasor in the k-th iteration.
  Let Bus $s$ be the slack bus, which is connected to the external network (source).
  $\underline{U}^{(0)}$ is initialized as follows:
  - For bus $i$, if there is no voltage measurement, assign $\underline{U}^{(0)} = e^{j \theta_i}$, where $\theta_i$ is
    the intrinsic transformer phase shift between Bus $i$ and Bus $s$.
  - For bus $i$, if there is a voltage measurement, assign $\underline{U}^{(0)} = U_{meas,i}e^{j \theta_i}$, where
    $U_{meas,i}$ is the measured voltage magnitude.
- In iteration $k$, follow the steps below:
  - Linearize the voltage measurements by using the phase angle of the calculated voltages of iteration $k-1$.
  - Linearize the complex power flow measurements by using either the linearized voltage measurement if the bus is
    measured, or the voltage phasor result from iteration $k-1$.
  - Compute the temporary new voltage phasor $\underline{\tilde{U}}^{(k)}$ using the pre-factorized matrix.
    See also [Matrix-prefactorization](./performance-guide.md#matrix-prefactorization)
  - Normalize the voltage phasor angle by setting the angle of the slack bus to zero:
  - If the maximum deviation between $\underline{U}^{(k)}$ and $\underline{U}^{(k-1)}$ is smaller than the error
    tolerance $\epsilon$, stop the iteration.
    Otherwise, continue until the maximum number of iterations is reached.

In the iteration process, the phase angle of voltages at each bus is updated using the last iteration; the system error
of the phase shift converges to zero.
Because the matrix is pre-built and pre-factorized, the computation cost of each iteration is much smaller than for the
[Newton-Raphson](#newton-raphson-state-estimation) method, where the Jacobian matrix needs to be constructed and
factorized each time.

Because the matrix depends on the variances of the linearized current measurements, pre-factorization requires assuming
that the magnitudes of the voltages remain constant throughout the iteration process.
This assumption is, of course, an approximation, and potentially results an inaccurate relative weighing of the power
measurements.
However, the difference between the solution and the actual grid state is usually small, because the error introduced by
this is typically dominated by the following contributing factors (see also
[this thread](https://github.com/PowerGridModel/power-grid-model/pull/951#issuecomment-2805154436)).

- voltages in the distribution grid are usually close to 1 p.u..
- power sensor errors are usually best-effort estimates.
- the interpretation of power measurements as current measurements is an approximation.

```{warning}
In short, the pre-factorization may produce results that may deviate from the actual grid state.
If higher precision is desired, please consider using
[Newton-Raphson state estimation](#newton-raphson-state-estimation) instead.
```

```{warning}
The algorithm will assume angles to be zero by default (see the details about voltage sensors).
This produces more correct outputs when the system is observable, but will prevent the calculation from raising an
exception, even if it is unobservable, therefore giving faulty results.
```

#### Newton-Raphson state estimation

Algorithm call: {py:class}`CalculationMethod.newton_raphson <power_grid_model.enum.CalculationMethod.newton_raphson>`

The Newton-Raphson state estimation considers the problem as a system of real, non-linear equations.
It iteratively solves the first order Taylor expansion of the system.
It does not make any assumptions on linearization.
It could be more accurate and converge in less iterations than the
[iterative linear](#iterative-linear-state-estimation) approach.
However, the Jacobian matrix needs to be calculated every iteration and cannot be prefactorized, which makes it slower
on average.

The Newton-Raphson method considers all measurements to be independent real measurements.
I.e., $\sigma_P$, $\sigma_Q$ and $\sigma_U$ are all independent values.
The rationale behind to calculation is similar to that of the
[Newton-Raphson method for power flow](#newton-raphson-power-flow).
Consequently, the iteration process differs slightly from that of
[iterative linear state estimation](#iterative-linear-state-estimation), as shown below.

- Initialization: let $\boldsymbol{U}^{(k)}$ be the column vector of the estimated voltage magnitude and
  $\boldsymbol{\theta}^{(k)}$ the column vector of the estimated voltage angle in k-th iteration.
  Let Bus $s$ be the slack bus which is connected to external network (source).
  We initialize $\boldsymbol{U}^{(0)}$ and $\boldsymbol{\theta}^{(k)}$ as follows:
  - For Bus $i$, if there is no voltage measurement, assign $U_i^{(0)} = 1$ and $\theta_i^{(0)} = 0$, where $\theta_i$
    is the intrinsic transformer phase shift between Bus $i$ and Bus $s$.
  - For Bus $i$, if there is a voltage measurement, $U_i^{(0)} = U_{\text{mea},i}$ and $\theta_i^{(0)} = \theta_i$,
    where $U_{\text{mea},i}$ is the measured voltage magnitude.
- In iteration $k$, follow the steps below:
  - Construct and (LU-)factorize the Jacobian matrix for $\boldsymbol{x}^{(k)}$, using the network parameters.
  - Compute the temporary new voltage magnitude and angle result,
    $\widetilde{\boldsymbol{U}}^{(k)}$ and $\widetilde{\boldsymbol{\theta}}^{(k)}$, using the prefactorized matrix.
    See also [Matrix-prefactorization](./performance-guide.md#matrix-prefactorization)
  - Normalize the result voltage phasor angle by setting angle of slack bus to zero:
    $\underline{U}_i^{(k)} = \widetilde{\underline{U}}_i^{(k)} \times |\widetilde{\underline{U}}_s^{(k)}| / \widetilde{\underline{U}}_s^{(k)}$. <!-- markdownlint-disable-line line-length -->
  - If the maximum deviation between $\underline{\boldsymbol{U}}^{(k)}$ and $\underline{\boldsymbol{U}}^{(k-1)}$ is
    smaller than the tolerance $\epsilon$, stop the iteration, otherwise continue until the maximum number of iterations
    is reached.
    Note: we're using the phasor here: $\underline{\boldsymbol{U}} = \boldsymbol{U}e^{j\boldsymbol{\theta}}$.

As for the [iterative linear](#iterative-linear-state-estimation) approach, during iterations, phase angles of voltage
at each bus are updated using ones from the previous iteration.
The system error of the phase shift converges to zero.

```{warning}
The algorithm will assume angles to be zero by default (see the details about voltage sensors).
In observable systems this helps better outputting correct results.
On the other hand with unobservable systems, exceptions raised from calculations due to faulty results will be 
prevented.
```

### Short circuit calculation algorithms

In the short circuit calculation, the following equations are solved with border conditions of faults added as
constraints.

$$ \begin{eqnarray} I_N & = Y_{bus}U_N \end{eqnarray} $$

This gives the initial symmetrical short circuit current ($I_k^{\prime\prime}$) for a fault.
This quantity is then used to derive almost all further calculations of short circuit studies applications.

At the moment, the following short circuit algorithms are implemented.

| Algorithm                                         | Default  | Speed    | Accuracy | Algorithm call                                                                            |
| ------------------------------------------------- | -------- | -------- | -------- | ----------------------------------------------------------------------------------------- |
| [IEC 60909](#iec-60909-short-circuit-calculation) | &#10004; | &#10004; |          | {py:class}`CalculationMethod.iec60909 <power_grid_model.enum.CalculationMethod.iec60909>` |

#### IEC 60909 short circuit calculation

Algorithm call: {py:class}`CalculationMethod.iec60909 <power_grid_model.enum.CalculationMethod.iec60909>`

The assumptions used for calculations in power-grid-model are aligned to the ones mentioned in
[IEC 60909](https://webstore.iec.ch/publication/24100).

- The state of the grid with respect to loads and generations are ignored for the short circuit calculation.
  (Note: Shunt admittances are included in calculation.)
- The pre-fault voltage is considered in the calculation and is calculated based on the grid parameters and topology.
  (Excl. loads and generation)
- The calculations are assumed to be time-independent.
  (Voltages are sine throughout with the fault occurring at a zero crossing of the voltage, the complexity of rotating
  machines and harmonics are neglected, etc.)
- To account for the different operational conditions, a voltage scaling factor of `c` is applied to the voltage source
  while running short circuit calculation function.
  The factor `c` is determined by the nominal voltage of the node that the source is connected to and the API option to
  calculate the `minimum` or `maximum` short circuit currents.
  The table to derive `c` according to IEC 60909 is shown below.

| Algorithm      | c_max | c_min |
| -------------- | ----- | ----- |
| `U_nom` <= 1kV | 1.10  | 0.95  |
| `U_nom` > 1kV  | 1.10  | 1.00  |

```{note}
In the IEC 609090 standard, there is a difference in `c` (for `U_nom` <= 1kV) for systems with a voltage tolerance of 6%
and 10%.
In power-grid-model we only use the value for a 10% voltage tolerance.
```

There are {py:class}`4 types <power_grid_model.enum.FaultType>` of fault situations that can occur in the grid, along
with the following possible combinations of the {py:class}`associated phases <power_grid_model.enum.FaultType>`:

- Three-phase to ground: `abc`
- Single phase to ground: `a`, `b`, `c`
- Two phase: `ab`, `bc`, `ac`
- Two phase to ground: `ab`, `bc`, `ac`

### Regulated power flow calculations

Regulated power flow calculations are disabled by default.

At the time of writing, the following regulated power flow calculation types are implemented.
Please refer to their respective sections for detailed documentation.

| Regulation type                                                   | Setting                                                                                 | Enum values                                                                 |
| ----------------------------------------------------------------- | --------------------------------------------------------------------------------------- | --------------------------------------------------------------------------- |
| [Automatic tap changing](#power-flow-with-automatic-tap-changing) | {py:meth}`tap_changing_strategy <power_grid_model.PowerGridModel.calculate_power_flow>` | {py:class}`TapChangingStrategy <power_grid_model.enum.TapChangingStrategy>` |

#### Power flow with automatic tap changing

Some of the most important regulators in the grid affect the tap position of transformers.
These {hoverxreftooltip}`user_manual/components:Transformer Tap Regulator`s try to regulate a control voltage
$U_{\text{control}}$ such that it is within a specified voltage band.
The $U_{\text{control}}$ may be compensated for the voltage drop during transport.
Power flow calculations that take the behavior of these regulators into account may be toggled by providing one of the
following strategies to the {py:meth}`tap_changing_strategy <power_grid_model.PowerGridModel.calculate_power_flow>`
option.

| Algorithm                                                                   | Default  | Speed    | Algorithm call                                                                                              |
| --------------------------------------------------------------------------- | -------- | -------- | ----------------------------------------------------------------------------------------------------------- |
| No automatic tap changing (regular power flow)                              | &#10004; | &#10004; | {py:class}`TapChangingStrategy.disabled <power_grid_model.enum.TapChangingStrategy.disabled>`               |
| Optimize tap positions for any value in the voltage band                    |          |          | {py:class}`TapChangingStrategy.any_valid_tap <power_grid_model.enum.TapChangingStrategy.any_valid_tap>`     |
| Optimize tap positions for lowest possible voltage in the voltage band      |          |          | {py:class}`TapChangingStrategy.min_voltage_tap <power_grid_model.enum.TapChangingStrategy.min_voltage_tap>` |
| Optimize tap positions for lowest possible voltage in the voltage band      |          |          | {py:class}`TapChangingStrategy.max_voltage_tap <power_grid_model.enum.TapChangingStrategy.max_voltage_tap>` |
| Optimize tap positions for any value in the voltage band with binary search |          | &#10004; | {py:class}`TapChangingStrategy.fast_any_tap <power_grid_model.enum.TapChangingStrategy.fast_any_tap>`       |

##### Control logic for power flow with automatic tap changing

We provide the control logic used for tap changing.
For simplicity, we demonstrate the case where the regulator control side and the transformer tap side are at different
sides.

- Regulated transformers are ranked according to how close they are to
  {hoverxreftooltip}`sources <user_manual/components:source>` in terms of the amount of regulated transformers
  inbetween.
  In the presence of meshed grids, transformers with conflicting ranks will be ranked the last.
  - Transformers are regulated in order according to their ranks.
- Initialize all transformers to their starting tap position (see
  {hoverxreftooltip}`user_manual/calculations:Initialization and exploitation of regulated transformers`)
- Find the optimal state using the following procedure
  - While some transformers can still be further regulated, iterate as follows:
    - Run a power flow calculation with the current tap positions with the specified
      [calculation method](#power-flow-algorithms).
    - Start with the transformers ranked closest to a {hoverxreftooltip}`user_manual/components:source` (because the
      source provides a relatively stable voltage level and these transformers will have a high impact on the rest of
      the grid).
    - Loop over all ranks:
      - Loop over all transformers within this rank; transformers with the same rank are independently regulated:
        - If the $U_{\text{control}} < U_{\text{set}} - \frac{U_{\text{band}}}{2}$:
          - The ratio between the voltage on the tap side and the voltage on the control side is too high.
          - To decrease this ratio, the tap ratio must be decreased.
          - Therefore, the tap position of the regulated transformer is decreased if it satisfies the bounds set by
            `tap_min` and `tap_max`.
        - If, however, the $U_{\text{control}} > U_{\text{set}} + \frac{U_{\text{band}}}{2}$:
          - The ratio between the voltage on the tap side and the voltage on the control side is too low.
          - To increase this ratio, the tap ratio must be increase.
          - Therefore, the tap position of the regulated transformer is increased if it satisfies the bounds set by
            `tap_min` and `tap_max`.
        - If the tap position of this transformer did not change, the transformer is considered regulated.
      - If not all transformers within this rank are regulated:
        - A better combination of tap positions may have been found.
        - Step out of the loop and go to the next iteration step.
  - Exploit the neighbourhood of all transformers (see
    {hoverxreftooltip}`user_manual/calculations:Initialization and exploitation of regulated transformers`)
    - Re-run the iteration in the above if any of the tap positions changed by the exploitation.

In the case where the control side of the regulator and the tap side of the transformer are at the same side, the
control logic of taps will be reverted (see
{hoverxreftooltip}`user_manual/calculations:Initialization and exploitation of regulated transformers`).
The exploitation of the neighbourhood ensures that the actual optimum is not accidentally missed due to feedback
mechanisms in the grid.

```{note}
For iterative [power flow calculation methods](#power-flow-algorithms), the initial state may not converge.
If the iterative process failed to converge, the optimization procedure is executed twice.
First, the {py:class}`linear <power_grid_model.enum.CalculationMethod.linear>` calculation method is used to find an
approximate solution.
The optimization procedure is then run a second time to find the actual optimum with the user-specified calculation
method.
```

```{note}
The control logic assumes that changes in the voltage level at a transformer are dominated by changes in the tap
position of the transformer itself, rather than by adjacent transformers.
This assumption is reflected in the requirements mentioned in
{hoverxreftooltip}`user_manual/components:Transformer Tap Regulator`.
```

```{note}
If the line drop compensation impedance is high, and the control side has generator-like behavior, then this assumption
does not hold, and the calculation may diverge.
Hence, this assumption is reflected in the requirements mentioned in
{hoverxreftooltip}`user_manual/components:Line drop compensation`.
```

##### Initialization and exploitation of regulated transformers

Internally, to achieve an optimal regulated tap position, the control algorithm sets initial tap positions and exploits
neighborhoods around local optima, depending on the strategy as follows.

| strategy                                                                                                    | initial tap position | exploitation direction | search method | description                                                                           |
| ----------------------------------------------------------------------------------------------------------- | -------------------- | ---------------------- | ------------- | ------------------------------------------------------------------------------------- |
| {py:class}`TapChangingStrategy.any_valid_tap <power_grid_model.enum.TapChangingStrategy.any_valid_tap>`     | current tap position | no exploitation        | linear search | Find any tap position that gives a control side voltage within the `u_band`           |
| {py:class}`TapChangingStrategy.min_voltage_tap <power_grid_model.enum.TapChangingStrategy.min_voltage_tap>` | voltage min tap      | voltage down           | binary search | Find the tap position that gives the lowest control side voltage within the `u_band`  |
| {py:class}`TapChangingStrategy.max_voltage_tap <power_grid_model.enum.TapChangingStrategy.max_voltage_tap>` | voltage min tap      | voltage up             | binary search | Find the tap position that gives the highest control side voltage within the `u_band` |
| {py:class}`TapChangingStrategy.fast_any_tap <power_grid_model.enum.TapChangingStrategy.fast_any_tap>`       | current tap position | no exploitation        | binary search | Find any tap position that gives a control side voltage within the `u_band`           |

| transformer configuration                      | voltage min tap | voltage min tap | voltage down | voltage up |
| ---------------------------------------------- | --------------- | --------------- | ------------ | ---------- |
| regulator control side != transformer tap side | `tap_max`       | `tap_min`       | step up      | step down  |
| regulator control side == transformer tap side | `tap_min`       | `tap_max`       | step down    | step up    |

##### Search methods used for tap changing optimization

Given the discrete nature of the finite tap ranges, we use the following search methods to find the next tap position
along the exploitation direction.

| Search method | Description                                                                                     |
| ------------- | ----------------------------------------------------------------------------------------------- |
| linear search | Start with an initial guess and do a local search with step size 1 for each iteration step.     |
| binary search | Start with a large search region and reduce the search region by half for every iteration step. |

## Batch Calculations

Usually, a single power-flow or state estimation calculation would not be enough to get insights in the grid.
Any form of multiple calculations can be carried out in power-grid-model using batch calculations.
Batches are not restricted to any particular type of calculations, like timeseries or contingency analysis or their
combination.
They can be used for determining hosting/loading capacity, determining optimal tap positions, estimating system losses,
monte-carlo simulations or any other form of multiple calculations required in a power-flow study.
The framework for creating the batches is the same for all types of calculations.
For every component, the attributes that can be updated in a batch scenario are mentioned in
[Components](components.md).
Examples of batch calculations for timeseries and contingency analysis are given in
[Power Flow Example](../examples/Power%20Flow%20Example.ipynb)

The same method as for single calculations, {py:class}`power_grid_model.PowerGridModel.calculate_power_flow`, can be
used to calculate a number of scenarios in one go.
To do this, you need to supply an `update_data` keyword argument.
This keyword argument contains a dictionary of 2D update arrays (one array per component type).

The performance for different batches vary.
power-grid-model automatically makes efficient calculations whenever possible.
See the [Performance Guide](performance-guide.md#topology-caching) for ways to optimally use the performance
optimizations.

### Batch data set

The parameters of the individual scenarios within a batch can be done by providing deltas compared to the existing state
of the model.
The values of unchanged attributes and components parameters within a scenario may be implicit (like a delta update) or
explicit (similarly to how one would provide a full state).
In the context of the power-grid-model, these are called **dependent** (implicit) and **independent** (explicit) batch
updates, respectively.
In both cases, all scenario updates are relative to the state of the model before the call of the calculation.
See the examples below for usage.

- Dependent batches are useful for a sparse sampling for many different components, e.g. for N-1 checks.
- Independent batches are useful for a dense sampling of a small subset of components, e.g. time series power flow
  calculation.

See the [Performance Guide](performance-guide.md#using-independent-batches) for more suggestions.

#### Example: dependent batch update

```py
# 3 scenarios, 3 objects (lines)
# for each scenario, only one line is specified
line_update = initialize_array('update', 'line', (3, 1))

# set the mutations for each scenario: disable one of the three lines
for component_update, component_id in zip(line_update, (3, 5, 8)):
    component_update['id'] = component_id
    component_update['from_status'] = 0
    component_update['to_status'] = 0

non_independent_update_data = {'line': line_update}
```

#### Example: full batch update data

```py
# 3 scenarios, 3 objects (lines)
# for each scenario, all lines are specified
line_update = initialize_array('update', 'line', (3, 3))

# use broadcasting to specify the default state
line_update['id'] = [[3, 5, 8]]
line_update['from_status'] = 1
line_update['to_status'] = 1

# set the mutations for each scenario: disable one of the three lines
for component_idx, scenario in enumerate(line_update):
    component = scenario[component_idx]
    component['from_status'] = 0
    component['to_status'] = 0

independent_update_data = {'line': line_update}
```

### Parallel Computing

The batch calculation supports shared memory multi-threading parallel computing.
The common internal states and variables are shared as much as possible to save memory usage and avoid copy.

You can set the `threading` keyword argument in the `calculate_*` functions (like
{py:class}`calculate_power_flow() <power_grid_model.PowerGridModel.calculate_power_flow>`) to enable/disable parallel
computing.

- `threading=-1`, use sequential computing (default)
- `threading=0`, use number of threads available from the machine hardware (recommended)
- `threading>0`, set the number of threads you want to use
