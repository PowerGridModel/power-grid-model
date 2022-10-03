<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Calculations

## Power-flow calculation

An example of usage of the power-flow calculation function is given in [Power flow Example](ex_power_flow)

```{eval-rst}
.. autofunction:: power_grid_model.PowerGridModel.calculate_power_flow
```

### Power-flow calculation algorithms

These should be selected when accurate solution is required within specified `error_tolerance`.

* `CalculationMethod.newton_raphson`: Traditional Newton-Raphson method.
* `CalculationMethod.iterative_current`: Newton-Raphson would be more robust in achieving convergence and require less
  iterations. However, Iterative current can be faster most times because it
  uses [Matrix Prefactorization](calculations.md#Matrix-Prefactorization).

Linear approximation methods are many times faster than the iterative methods. Can be used where approximate solutions
are acceptable. Both methods have equal computation time for a single powerflow calculation.

* `CalculationMethod.linear`: It will be more accurate when most of the load/generation types are of constant impedance.
* `CalculationMethod.linear_current`: It will be more accurate when most of the load/generation types are constant power
  or constant current. Batch calculations here will be faster because matrix prefacorization is possible.

## State estimation calculation

An example of usage of the power-flow calculation function is given in [State Estimation Example](ex_state_est)

```{eval-rst}
.. autofunction:: power_grid_model.PowerGridModel.calculate_state_estimation
```

### State Estimation algorithms

* `CalculationMethod.iterative_linear`: It is an iterative method which converges to a true
  solution. [Matrix Prefactorization](calculations.md#Matrix-Prefactorization) is possible.

# Batch Calculations

We can use the same method of `calculate_power_flow` to calculate a number of scenarios in one go. To do this, you need
to supply an `update_data` argument. This argument contains a dictionary of 2D update arrays (one array per component
type).

The model uses the current data as the base scenario. For each individual calculation, the model applies each mutation
to the base scenario and calculates the power flow.

```{note}
After the batch calculation, the original model will be kept unchanged. Internally the program copies the original model to multiple batch models for the calculation.
```

## Add more examples of batch calculation

[Issue 79](https://github.com/alliander-opensource/power-grid-model/issues/79)

## Independent Batch Dataset

# Guidelines for performance enhancement

% TODO add % Explain grid types interpreted by power-grid-model and its relation with performance

### Independent

% TODO add Check example

### cache topology

% TODO add Check example

### Parallel Computing

% TODO add Check example

## Matrix prefactorization

Every iteration of power-flow or state estimation has a step of solving large number of sparse linear equations
i.e. `AX=b` in matrix form. Computation wise this is a very expensive step. One major component of this step is
factorization of the `A` matrix. In certain calculation methods, this `A` matrix and its factorization remains unchanged
over iterations and batches (only specific cases). This makes it possible to reuse the factorization, skip this step and
improve performance.

**Note:** Prefactorization over batches is possible when switching status or specified power values of load/generation
or source reference voltage is modified. It is not possible when topology or grid parameters are modified, i.e. in
switching of branches, shunt, sources or change in transformer tap positions.

