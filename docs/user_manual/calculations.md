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
They can be used where approximate solutions are acceptable. The table below can be used to pick the right algorithm. Below the table a more in depth explanation is given for each algorithm.

| Algorithm                               | Speed    | Accuracy | Algorithm call                        |
|-----------------------------------------|----------|----------|---------------------------------------|
| [Newton-Raphson](#newton-raphson)       |          | &#10004; | `CalculationMethod.newton_raphson`    |
| [Iterative current](#iterative-current) |          | &#10004; | `CalculationMethod.iterative_current` | 
| [Linear](#linear)                       | &#10004; |          | `CalculationMethod.linear`            | 
| [Linear current](#linear-current)       | &#10004; |          | `CalculationMethod.linear_current`    |

TODO: for each of the algorithms give a brief explanation of the algorithm and in what cases this algorithm would be the prefered method. The old explanations are given, but they should be extended/improved.
Also include the mathematics/algorithms.

#### Newton-Raphson
Traditional Newton-Raphson method.

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
It will be more accurate when most of the load/generation types are of constant impedance.

#### Linear current

**This algorithm is essentially a single iteration of [Iterative Current](calculations.md#iterative-current).** It will be a better approximation when most of the load/generation types resemble constant current. Similar to [Iterative Current](calculations.md#iterative-current), batch calculations like timeseries, here will also be faster. The reason is the same that the Y bus matrix does not change across batches and the same factorization would be used.


In practical grids most loads and generations correspond to the constant power type. Linear current would give a better approximation than [Linear](calculations.md#linear) in such case. This is because we approximate the load as current instead of impedance.
There is a correlation in voltage error of approximation with respect to the actual voltage for all approximations. They are most accurate when the actual voltages are close to 1 p.u. and the error increases as we deviate from this level. 
When we approximate the load as impedance at 1 p.u., the voltage error has quadratic relation to the actual voltage. When it is approximated as a current at 1 p.u., the voltage error is only linearly dependent in comparison.


### State estimation algorithms
At the moment one state estimation algorithm is implemented: [iterative linear](#iterative-linear).

#### Iterative linear

TODO: extend the explanation of the algorithm. Mention that the algorithm will assume angles to be zero if not given. This might result in not having a 
crash due to an unobservable system, but succeeding with the calculations and giving faulty results.

Algorithm call: `CalculationMethod.iterative_linear`. It is an iterative method which converges to a true
  solution. [Matrix-prefactorization](./performance-guide.md#matrix-prefactorization) is possible.

## Batch Calculations

TODO, add explanation on batch calculations:
- when to use batch calculations
- what are the batch options
- how to use it
- explain independent batches and caching topology
- something else?


```{warning}
[Issue 79](https://github.com/alliander-opensource/power-grid-model/issues/79)
```
