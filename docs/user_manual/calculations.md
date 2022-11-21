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
- Link to pf / se workshop?

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

In order to perform a state estimation the system should be observable. Simply said, observability means that the number of measurements
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
      n_{unknowns}    & = & 2 & \cdot & n_{unknowns}
   \end{eqnarray}
$$

And

$$
   \begin{eqnarray}
      n_{measurements} = n_{nodes\_with\_voltage\_sensor\_without\_angle} + 2 n_{nodes_with_voltage_sensor_with_angle} + 2 n_{branches_with_power_sensor} + 2 n_{nodes_without_any_appliances_connected} + 2 n_{nodes_with_all_connected_appliances_measured_by_power_sensor}
   \end{eqnarray}
$$





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

#### Newton-Raphson
Traditional Newton-Raphson method.

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
