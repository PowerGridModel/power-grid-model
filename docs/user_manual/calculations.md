<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Calculations

## Power-flow calculation

Power flow calculation is done using the {py:class}`calculate_power_flow <power_grid_model.PowerGridModel.calculate_power_flow>` method. An example of usage of the power-flow calculation function is given in [Power flow Example](ex_power_flow)


### Power-flow calculation algorithms

These should be selected when accurate solution is required within specified `error_tolerance`.

* `CalculationMethod.newton_raphson`: Traditional Newton-Raphson method.
* `CalculationMethod.iterative_current`: Newton-Raphson would be more robust in achieving convergence and require less
  iterations. However, Iterative current can be faster most times because it
  uses [](./performance-guide.md#matrix-prefactorization).

Linear approximation methods are many times faster than the iterative methods. Can be used where approximate solutions
are acceptable. Both methods have equal computation time for a single powerflow calculation.

* `CalculationMethod.linear`: It will be more accurate when most of the load/generation types are of constant impedance.
* `CalculationMethod.linear_current`: It will be more accurate when most of the load/generation types are constant power
  or constant current. Batch calculations here will be faster because matrix prefactorization is possible.

## State estimation calculation

State Estimation is done using the {py:class}`calculate_state_estimation <power_grid_model.PowerGridModel.calculate_state_estimation>`. An example of usage of the power-flow calculation function is given in [State Estimation Example](ex_state_est)

### State Estimation algorithms

* `CalculationMethod.iterative_linear`: It is an iterative method which converges to a true
  solution. [](./performance-guide.md#matrix-prefactorization) is possible.

## Batch Calculations

```{warning}
[Issue 79](https://github.com/alliander-opensource/power-grid-model/issues/79)
```
