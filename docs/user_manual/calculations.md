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

TODO: fill in the table below. Maybe add some other column if that would make the decision to choose an algorithm easier.

| Algorithm                                  | Speed    | Accuracy | Radial grid | Meshed grid | Algorithm call                        |
|--------------------------------------------|----------|----------|-------------|-------------|---------------------------------------|
| [Newton-Raphson](####Newton-Raphson)       |          | &#10004; |             |             | `CalculationMethod.newton_raphson`    |
| [Iterative current](####Iterative current) |          | &#10004; |             |             | `CalculationMethod.iterative_current` | 
| [Linear](####Linear)                       | &#10004; |          |             |             | `CalculationMethod.linear`            | 
| [Linear current](####Linear current)       | &#10004; |          |             |             | `CalculationMethod.linear_current`    |

TODO: for each of the algorithms give a brief explanation of the algorithm and in what cases this algorithm would be the prefered method. The old explanations are given, but they should be extended/improved.

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

Usually, a single power-flow or state estimation calculation would not be enough to get insights in the grid. 
Any form of multiple number of calculations can be carried out in power-grid-model using batch calculations. 
Batches are not restricted to any particular types of calculations like timeseries or contingency analysis or their combination.
They can be used for determining hosting/loading capacity, determining optimal tap positions, estimating system losses, monte-carlo simulations or any other form of multiple calculations required in a power-flow study.
The framework of creating the batches remains the same.
The attributes of each component which can be updated over batches are mentioned in [Components](components.md).
An example of batch calculation of timeseries and contingency analysis is given in [Power Flow Example](../examples/Power%20Flow%20Example.ipynb)

The same method `calculate_power_flow` to calculate a number of scenarios in one go. 
To do this, you need to supply a `update_data` argument. 
This argument contains a dictionary of 2D update arrays (one array per component type).

The performance for different batches vary. power-grid-model automatically makes efficient calculations wherever possible in case of [independent batches](calculations.md#independent-batch-dataset) and [caching topology](calculations.md#caching-topology).

### Independent Batch dataset

There are two ways to specify batches.

- Only specify the objects and attributes that are changed in this batch.
Here original model is copied everytime for each batch.
- We specify all objects and attributes including the unchanged ones in one or more scenarios. i.e. The attributes to be updated have data for all batches.
This is an **independent** batch dataset (In a sense that each batch is independent of the original model input).
We do not need to keep a copy of the original model in such case.
The original model data is copied only once while we mutate over that data for all the batches. 
This brings performance benefits.

### Caching topology

To perform the calculations, a graph topology of the grid is to be constructed from the input data first. 

- If your batch scenarios are changing the switching status of branches and sources the base model is then kept as empty model without any internal cached graph/matrices. 
Thus, the topology is constructed afresh for each batch from the input data.
N-1 check is a typical use case.

- If all your batch scenarios do not change the switching status of branches and sources the model will re-use the pre-built internal graph/matrices for each calculation.
Time-series load profile calculation is a typical use case. This can bring performance benefits.

### Parallel Computing

The batch calculation supports shared memory multi-threading parallel computing. 
The common internal states and variables are shared as much as possible to save memory usage and avoid copy.

You can set `threading` parameter in `calculate_power_flow()` or `calculate_state_estimation()` to enable/disable parallel computing.
