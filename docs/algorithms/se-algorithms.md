<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# State Estimation Algorithm Details

This page provides detailed mathematical descriptions of the state estimation algorithms implemented in
power-grid-model.
For a summary and guidance, see [Calculations](../user_manual/calculations.md#state-estimation-algorithms).

## Weighted Least Squares Formulation

Weighted least squares (WLS) state estimation can be performed with power-grid-model.
Given a grid with $N_b$ buses the state variable column vector is defined as below.

$$
\begin{aligned}
        \underline{U}     =     \begin{bmatrix}
                        \underline{U}_1 \\
                        \underline{U}_2 \\
                        \vdots \\
                        \underline{U}_{N_{b}}
                    \end{bmatrix}
\end{aligned}
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
\begin{aligned}
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
\end{aligned}
$$

Where $\underline{x}_i$ is the real value of the i-th measured quantity in complex form, $\underline{z}_i$ is the i-th
measured value in complex form, $\sigma_i$ is the normalized standard deviation of the measurement error of the i-th
measurement, $\Sigma$ is the normalized covariance matrix and $W$ is the weighting factor matrix.

## State Estimation Measurement Aggregation

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
It is not possible to mix [power sensors](../user_manual/components.md#generic-power-sensor) with
[current sensors](../user_manual/components.md#generic-current-sensor) on the same terminal of the same component.
It is also not possible to mix
[current sensors with global angle measurement type](../user_manual/components.md#global-angle-current-sensors) with
[current sensors with local angle measurement type](../user_manual/components.md#local-angle-current-sensors) on the same terminal
of the same component.
However, such mixing of sensor types is allowed as long as they are on different terminals.
```

## State Estimate Sensor Transformations

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
        & \mathrm{Re}\left\{I\right\} = I \cos\theta
   \end{eqnarray}
$$
$$
   \begin{eqnarray}
        & \mathrm{Im}\left\{I\right\} = I \sin\theta
   \end{eqnarray}
$$
$$
   \begin{eqnarray}
        & \text{Var}\left(\mathrm{Re}\left\{I\right\}\right) =
            \sigma_i^2 \cos^2\theta + I^2 \sigma_{\theta}^2\sin^2\theta
   \end{eqnarray}
$$
$$
   \begin{eqnarray}
        & \text{Var}\left(\mathrm{Im}\left\{I\right\}\right) =
            \sigma_i^2 \sin^2\theta + I^2 \sigma_{\theta}^2\cos^2\theta
   \end{eqnarray}
$$

## Iterative linear state estimation

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
    See also [Matrix-prefactorization](../user_manual/performance-guide.md#matrix-prefactorization)
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

## Newton-Raphson state estimation

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
[Newton-Raphson method for power flow](./pf-algorithms.md#newton-raphson-power-flow).
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
    See also [Matrix-prefactorization](../user_manual/performance-guide.md#matrix-prefactorization)
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
