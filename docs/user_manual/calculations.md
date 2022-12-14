<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Calculations

## Calculation types
With power-grid-model it is possible to perform two different types of calculation:
- [Power flow](#power-flow-algorithms): a "what-if" scenario calculation. This calculation can be performed by using the {py:class}`calculate_power_flow <power_grid_model.PowerGridModel.calculate_power_flow>` method. An example of usage of the power-flow calculation function is given in [Power flow Example](../examples/Power%20Flow%20Example.ipynb)
- [State estimation](#state-estimation-algorithms): a statistical method that calculates the most probabilistic state of the grid, given sensor values with an uncertainty. This calculation can be performed by using the {py:class}`calculate_state_estimation <power_grid_model.PowerGridModel.calculate_state_estimation>` method. An example of usage of the power-flow calculation function is given in [State Estimation Example](../examples/State%20Estimation%20Example.ipynb)

### Calculation types explained

#### Power flow
Power flow is a "what-if" based grid calculation that will calculate the node voltages and the power flow through the branches, based on assumed load/generation profiles.
Some typical use-cases are network planning and contingency analysis.

Input:
- Network data: topology + component attributes
- Assumed load/generation profile

Output:
- Node voltage magnitude and angle
- Power flow through branches

#### State estimation
State estimation is a statistical calculation method that determines the most probable state of the grid, based on
network data and measurements. Measurements meaning power flow or voltage values with some kind of uncertainty, which were 
either measured, estimated or forecasted.

Input:
- Network data: topology + component attributes
- Power flow / voltage measurements with uncertainty

Output:
- Node voltage magnitude and angle
- Power flow through branches
- Deviation between measurement values and estimated state

In order to perform a state estimation the system should be observable. If the system is not observable the calculation will 
raise a singular matrix error. Simply said, observability means that the number of measurements
should be larger than or equal to the number of unknowns. For each node there are two unknowns, `u` and `u_angle`, so the following
equations should be met:

$$
   \begin{eqnarray}
      n_{measurements}    & >= & n_{unknowns}
   \end{eqnarray}
$$

Where

$$
   \begin{eqnarray}
      n_{unknowns}    & = & 2 & \cdot & n_{nodes}
   \end{eqnarray}
$$

The number of measurements can be found by the sum of the following:
- number of nodes with a voltage sensor with magnitude only
- two times the number of nodes with a voltage sensor with magnitude and angle
- two times the number of nodes without appliances connected
- two times the number of nodes where all connected appliances are measured by a power sensor
- two times the number of branches with a power sensor

Note: enough measurements doesn't necessarily mean that the system is observable. The location of the measurements is also
of importance. Also, there should be at least one voltage measurement. The [iterative linear](#iterative-linear) 
state estimation algorithm assumes voltage angles to be zero when not given. This might result in the calculation succeeding, but giving 
a faulty outcome instead of raising a singular matrix error. 





### Power flow algorithms
Two types of power flow algorithms are implemented in power-grid-model; iterative algorithms (Newton-Raphson / Iterative current) and linear algorithms (Linear / Linear current).
Iterative methods are more accurate and should thus be selected when an accurate solution is required. Linear approximation methods are many times faster than the iterative methods, in tradeoff to accuracy. 
They can be used where approximate solutions are acceptable. 
Their accuracy is not explicitly calculated and may vary a lot. The user should have an intuition of their applicability based on the input grid configuration.
The table below can be used to pick the right algorithm. Below the table a more in depth explanation is given for each algorithm.

| Algorithm                               | Speed    | Accuracy | Algorithm call                        |
|-----------------------------------------|----------|----------|---------------------------------------|
| [Newton-Raphson](#newton-raphson)       |          | &#10004; | `CalculationMethod.newton_raphson`    |
| [Iterative current](#iterative-current) |          | &#10004; | `CalculationMethod.iterative_current` | 
| [Linear](#linear)                       | &#10004; |          | `CalculationMethod.linear`            | 
| [Linear current](#linear-current)       | &#10004; |          | `CalculationMethod.linear_current`    |

TODO: for each of the algorithms give a brief explanation of the algorithm and in what cases this algorithm would be the prefered method. The old explanations are given, but they should be extended/improved.
Also include the mathematics/algorithms.

The nodal equations of a power system network can be written as:

$$
   \begin{eqnarray}
      I    & = Y_{bus}V
   \end{eqnarray}
$$

Where $I$ is the $N$ vector of source currents injected into each bus and $V$ is the $N$ vector of bus voltages. The complex power
delivered to bus $k$ is:

$$
   \begin{eqnarray}
      S_{k}    & =  P_k + jQ_k & = V_{k} I_{k}^{*}
   \end{eqnarray}
$$

Power flow equations are based on solving the nodal equations above to obtain the voltage magnitude and voltage angle at each node
and then obtaining the real and reactive power flow through the branches. The following bus types can be present in the system:

- Slack bus: the reference bus with known voltage and angle; in power-grid-model referred to as the [source](./components.md#source).
- Load bus: a bus with known $P$ and $Q$.
- Voltage controlled bus: a bus with known $P$ and $V$. NOTE: this bus is not supported by power-grid-model yet.

#### Newton-Raphson
This is the traditional method for power flow calculations. This method uses a Taylor series, ignoring the higher order
terms, to solve the nonlinear set of equations iteratively:

$$
   \begin{eqnarray}
      f(x)    & =  y
   \end{eqnarray}
$$

Where:

$$
   \begin{eqnarray}
      x     =  \begin{bmatrix}
               \delta \\
               V
               \end{bmatrix} = 
               \begin{bmatrix}
               \delta_2 \\
               \vdots \\
               \delta_N \\
               V_2 \\
               \vdots \\
               V_N
               \end{bmatrix}
      \quad\text{and}\quad
      y     =  \begin{bmatrix}
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
      f(x)  =  \begin{bmatrix}
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
derivatives of $\dfrac{\partial P}{\partial \delta}$, $\dfrac{\partial P}{\partial V}$, $\dfrac{\partial Q}{\partial \delta}$
and $\dfrac{\partial Q}{\partial V}$.

For each iteration the following steps are executed:
- Compute $\Delta y(i)$
- Compute the Jacobian $J(i)$
- Using LU decomposition, solve $J(i) \Delta x(i)  =  \Delta y(i)$ for $\Delta x(i)$
- Compute $x(i+1)$ from $\Delta x(i) =  x(i+1) - x(i)$

#### Iterative Current

This algorithm is a jacobi like method for powerflow analysis.
It has linear convergence as opposed to quadratic convergence in newton-raphson method. This means that the number of iterations will be greater. Newton-Raphson would also be more robust in achieving convergence in case of greater meshed configurations. However, Iterative current algorithm will be faster most of the time.

The algorithm is as follows:
1. Build Y bus matrix 
2. Initialization $U_N^0$ 
3. Calculate injected currents: $I_N^i$ for $i^{th}$ iteration. The injected currents are calculated as per ZIP model of loads and generation using $U_N$. 
$$  I_N = \overline{S_{Z}} \cdot U_{N} + \overline{(\frac{S_{I}}{U_{N}})} \cdot |U_{N}| + \overline{(\frac{S_{P}}{U_N})}
$$
4. Solve linear equation: $YU_N^i = I_N^i$ 
5. Check convergence: If maximum voltage deviation from previous iteration is greater than the tolerance setting (ie. $u^{(i-1)}_\sigma > u_\epsilon$), then go back to step 3. 

Compared to newton-raphson, it only needs to calculate injected currents before solving linear equations. This is more straightforward than calculating the jacobian.

Factorizing the matrix of linear equation is the most computationally heavy task. The Y bus matrix here does not change across iterations which means it only needs to be factorized once to solve the linear equations in all iterations. The Y bus matrix is also unchanged in certain batch calculations like timeseries calculations. Thus the same factorization is used for all batches as well.


#### Linear

This is an approximation method where we assume that all loads and generations are of constant impedance type regardless of their actual `LoadGenType`.
By doing so, we obtain huge performance benefits as the computation required is equivalent to a single iteration of the iterative methods.
It will be more accurate when most of the load/generation types are of constant impedance or the actual node voltages are close to 1 p.u.
When all the load/generation types are of constant impedance, power-grid-model uses Linear method regardless of the input provided by the user. This is because this method will then be accurate and fastest. 

The algorithm is as follows:
1. Assume injected currents by loads $I_N=0$ since we model loads/generation as impedance. 
Admittance of each load/generation is calculated from rated power as $y=-\overline{S}$ 
2. Build Y bus matrix. Add the admittances of loads/generation to the diagonal elements.
3. Solve linear equation: $YU_N^i = I_N^i$ for $U_N$

#### Linear current

**This algorithm is essentially a single iteration of [Iterative Current](calculations.md#iterative-current).** It will be a better approximation when most of the load/generation types resemble constant current. Similar to [Iterative Current](calculations.md#iterative-current), batch calculations like timeseries, here will also be faster. The reason is the same that the Y bus matrix does not change across batches and the same factorization would be used.


In practical grids most loads and generations correspond to the constant power type. Linear current would give a better approximation than [Linear](calculations.md#linear) in such case. This is because we approximate the load as current instead of impedance.
There is a correlation in voltage error of approximation with respect to the actual voltage for all approximations. They are most accurate when the actual voltages are close to 1 p.u. and the error increases as we deviate from this level. 
When we approximate the load as impedance at 1 p.u., the voltage error has quadratic relation to the actual voltage. When it is approximated as a current at 1 p.u., the voltage error is only linearly dependent in comparison.


### State estimation algorithms
Weighted least squares (WLS) state estimation can be performed with power-grid-model.
Given a grid with $N_b$ buses the state variable column vector is defined as below.


$$
   \begin{eqnarray}
            U     =     \begin{bmatrix}
                            U_1 \\
                            U_2 \\ 
                            \vdots \\
                            U_{N_{b}}
                        \end{bmatrix} 
   \end{eqnarray}
$$

Where $U_i$ is the complex voltage phasor of the i-th bus. 

The goal of WLS state estimation is to evaluate the state variable with the highest likelihood given (pseudo) measurement input,
by solving:

$$
   \begin{eqnarray}
            min r(U) = \dfrac{1}{2} (f(U) - z)^H W (f(U) - z)
   \end{eqnarray}
$$

Where:

$$
   \begin{eqnarray}
      x     =  \begin{bmatrix}
               x_1 \\
               x_2 \\
               \vdots \\
               x_{N_{m}}
               \end{bmatrix} = 
               f(U)
      \quad\text{and}\quad
      z     =  \begin{bmatrix}
               z_1 \\
               z_2 \\
               \vdots \\
               z_{N_{m}}
               \end{bmatrix} 
      \quad\text{and}\quad
      W  = \Sigma^{-1} =  \begin{bmatrix}
               \sigma_1^2 & 0 & \cdots & 0 \\
               0 & \sigma_2^2 & \cdots & 0 \\
               \vdots & \vdots & \ddots &vdots \\
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

Where $x_i$ is the real value of the i-th measured quantity in complex form, $z_i$ is the i-th measured value in complex form,
$\sigma_i$ is the normalized standard deviation of the measurement error of the i-th measurement, $\Sigma$ is the normalized covariance matrix
and $W$ is the weighting factor matrix.

At the moment one state estimation algorithm is implemented: [iterative linear](#iterative-linear).

There can be multiple sensors measuring the same physical quantity. For example, there can be multiple
voltage snesors on the same bus. The measurement data can be merged into one virtual measurement using a Kalman filter:

$$
   \begin{eqnarray}
            z = \dfrac{\sum_{k=1}^{N_{sensor}} z_k \sigma_k^{-2}}{\sum_{k=1}^{N_{sensor}} \sigma_k^{-2}} 
   \end{eqnarray}
$$

Where $z_k$ and $\sigma_k$ are the measured value and standard deviation of individual measurements.

Multiple appliance measurements (power measurements) on one bus are aggregated as the total injection at the bus:

$$
   \begin{eqnarray}
            S = \sum_{k=1}^{N_{appliance}} S_k 
            \quad\text{and}\quad
            \sigma^2 = \sum_{k=1}^{N_{appliance}} \sigma_k^2
   \end{eqnarray}
$$

Where $S_k$ and $\sigma_k$ are the measured value and the standard deviation of the individual appliances.

#### Iterative linear

TODO: extend the explanation of the algorithm. Mention that the algorithm will assume angles to be zero if not given. This might result in not having a 
crash due to an unobservable system, but succeeding with the calculations and giving faulty results.

Linear WLS requires all measurements to be linear. This is only possible is all measurements are phasor unit measurements, 
which is not realistic in a distribution grid. Therefore, traditional measurements are linearized before the algorithm is performed:

- Linear WLS requires a voltage phasor including a phase angle. Given that the phase shift in the distribution grid is very small, 
it is assumed that the angle shift is zero plus the intrinsic phase shift of transformers. For a certain bus `i`, the voltage
magnitude measured at that bus is translated into voltage phasor:

$$
   \begin{eqnarray}
            \underline{U}_i = U_i \cdot e^{j \theta_i}
   \end{eqnarray}
$$

Where $\theta_i$ is the intrinsic transformer phase shift.

- Test layout




Algorithm call: `CalculationMethod.iterative_linear`. It is an iterative method which converges to a true
  solution. [Matrix-prefactorization](./performance-guide.md#matrix-prefactorization) is possible.

## Batch Calculations

Usually, a single power-flow or state estimation calculation would not be enough to get insights in the grid. 
Any form of multiple number of calculations can be carried out in power-grid-model using batch calculations. 
Batches are not restricted to any particular types of calculations like timeseries or contingency analysis or their combination.
They can be used for determining hosting/loading capacity, determining optimal tap positions, estimating system losses, monte-carlo simulations or any other form of multiple calculations required in a power-flow study.
The framework of creating the batches remains the same.
The attributes of each component which can be updated over batches are mentioned in [Components](components.md).
An example of batch calculation of timeseries and contingency analysis is given in [Power Flow Example](../examples/Power%20Flow%20Example.ipynb)

The same method as for single calculations, `calculate_power_flow`, can be used to calculate a number of scenarios in one go.
To do this, you need to supply a `update_data` argument. 
This argument contains a dictionary of 2D update arrays (one array per component type).

The performance for different batches vary. power-grid-model automatically makes efficient calculations wherever possible in case of [independent batches](calculations.md#independent-batch-dataset) and [caching topology](calculations.md#caching-topology).

### Independent Batch dataset

There are two ways to specify batches.

- Only specify the objects and attributes that are changed in this batch.
Here original model is copied everytime for each batch.

Example:
```
line_update = initialize_array('update', 'line', (3, 1)) 
# for each mutation, only one object is specified
line_update['id'] = [[3], [5], [8]]
# specify only the changed status (switch off) of one line
line_update['from_status'] = [[0], [0], [0]]
line_update['to_status'] = [[0], [0], [0]]

non_independent_update_data = {'line': line_update}
```

- We specify all objects and attributes including the unchanged ones in one or more scenarios. i.e. The attributes to be updated have data for all batches.
This is an **independent** batch dataset (In a sense that each batch is independent of the original model input).
We do not need to keep a copy of the original model in such case.
The original model data is copied only once while we mutate over that data for all the batches. 
This brings performance benefits.

Example:
```
line_update = initialize_array('update', 'line', (3, 3))  # 3 scenarios, 3 objects (lines)
# below the same broadcasting trick
line_update['id'] = [[3, 5, 8]]
# fully specify the status of all lines, even it is the same as the base scenario
line_update['from_status'] = [[0, 1, 1], [1, 0, 1], [1, 1, 0]]
line_update['to_status'] = [[0, 1, 1], [1, 0, 1], [1, 1, 0]]

independent_update_data = {'line': line_update}
```

### Caching topology

To perform the calculations, a graph topology of the grid is to be constructed from the input data first. 

- If your batch scenarios are changing the switching status of branches and sources the base model is then kept as empty model without any internal cached graph/matrices. 
Thus, the topology is constructed for each batch from the input data.
N-1 check is a typical use case.

- If all your batch scenarios do not change the switching status of branches and sources the model will re-use the pre-built internal graph/matrices for each calculation.
Time-series load profile calculation is a typical use case. This can bring performance benefits.

The topology is not cached when any of the switching statuses (`from_status`, `to_status` or `status`) of the following components are updated:
1. Branches: Lines, Links, Transformers
2. Branch3: Three winding transformer
3. Appliances: Sources

### Parallel Computing

The batch calculation supports shared memory multi-threading parallel computing. 
The common internal states and variables are shared as much as possible to save memory usage and avoid copy.

You can set `threading` parameter in `calculate_power_flow()` or `calculate_state_estimation()` to enable/disable parallel computing.

- `threading=-1`, use sequential computing (default)
- `threading=0`, use number of threads available from the machine hardware (recommended)
- `threading>0`, set the number of threads you want to use