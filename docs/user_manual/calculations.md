<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Calculations

## Calculation types
With power-grid-model it is possible to perform two different types of calculation:
- [Power flow](###Power flow algorithms): a "what-if" scenario calculation. This calculation can be performed by using the {py:class}`calculate_power_flow <power_grid_model.PowerGridModel.calculate_power_flow>` method. An example of usage of the power-flow calculation function is given in [Power flow Example](ex_power_flow)
- [State estimation](###State estimation algorithms): a statistical method that calculates the most probabilistic state of the grid, given sensor values with an uncertainty. This calculation can be performed by using the {py:class}`calculate_state_estimation <power_grid_model.PowerGridModel.calculate_state_estimation>` method. An example of usage of the power-flow calculation function is given in [State Estimation Example](ex_state_est)

### Calculation types explained
TODO: 
- What is the difference between power flow and state estimation
- When should you use which? Maybe small example (physical, not code)

### Power flow algorithms
Two types of power flow algorithms are implemented in power-grid-model; iterative algorithms (Newton-Raphson / Iterative current) and linear algorithms (Linear / Linear current).
Iterative methods are more accurate and should thus be selected when an accurate solution is required. Linear approximation methods are many times faster than the iterative methods, in tradeoff to accuracy. 
They can be used where approximate solutions are acceptable. The table below can be used to pick the right algorithm. Below the table a more in depth explanation is given for each algorithm.

| Algorithm                                  | Speed    | Accuracy | Algorithm call                        |
|--------------------------------------------|----------|----------|---------------------------------------|
| [Newton-Raphson](####Newton-Raphson)       |          | &#10004; | `CalculationMethod.newton_raphson`    |
| [Iterative current](####Iterative current) |          | &#10004; | `CalculationMethod.iterative_current` | 
| [Linear](####Linear)                       | &#10004; |          | `CalculationMethod.linear`            | 
| [Linear current](####Linear current)       | &#10004; |          | `CalculationMethod.linear_current`    |

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

Power flow equations are based on solving the nodal equations above to obtain the voltage and voltage angle at each node
and then obtaining the real and reactive power flow through the branches. The following bus types can be present in the system:

- Slack bus: the reference bus with known voltage and angle; in power-grid-model referred to as the [source](./components.md#source).
- Load bus: a bus with known $P$ and $Q$.
- Voltage controlled bus: a bus with known $P$ and $V$.

#### Newton-Raphson
This is the traditional method for power flow calculations. This method uses a Taylor series, ignoring the higher order
terms, to solve the nonlinear set of equations:

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
               \delta_N
               V_2
               \vdots \\
               V_N
               \end{bmatrix}
   \end{eqnarray}
$$

#### Iterative Current
Newton-Raphson would be more robust in achieving convergence and require fewer iterations. However, Iterative current can be faster most times because it uses .

#### Linear
It will be more accurate when most of the load/generation types are of constant impedance.

#### Linear current
It will be more accurate when most of the load/generation types are constant power or constant current. Batch calculations here will be faster because matrix prefactorization is possible.

## Power-flow calculation

Power flow calculation is done using the {py:class}`calculate_power_flow <power_grid_model.PowerGridModel.calculate_power_flow>` method. An example of usage of the power-flow calculation function is given in [Power flow Example](ex_power_flow)


### State estimation algorithms
At the moment one state estimation algorithm is implemented: [iterative linear](####Iterative linear).

#### Iterative linear

TODO: extend the explanation of the algorithm.

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
