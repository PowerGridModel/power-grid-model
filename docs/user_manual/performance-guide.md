<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Guidelines for performance enhancement

```{warning}
[Issue 79](https://github.com/PowerGridModel/power-grid-model/issues/79)
```
% TODO Explain grid types interpreted by power-grid-model and its relation with performance or any other tips/tricks//hacks (Check examples)

## Batch calculations

Depending on the details of the batch, a number of performance optimizations are possible:

* [Topology constructions](#topology-caching), especially, may significantly impact the computation time of a scenario.
* The way the [batch data set](#using-independent-batches) is provided to the model can also affect the performance.

### Topology caching

Topology is an expensive step in calculations.
Fortunately, the topology can be cached when there are no structural changes to the power grid itself.
For the power-grid-model, this is the case when there no changes to statuses (`from_status`, `to_status`, `status`, etc.) of the following components:

1. Branches: Lines, Links, Transformers
2. Branch3: Three winding transformer
3. Appliances: Sources

In particular, the topology is cached in the following way:

- If none of the provided batch scenarios change the status of branches and sources, the model will re-use the pre-built internal graph/matrices for each calculation. Time-series load profile calculation is a typical use case.
- If some batch scenarios are changing the switching status of branches and sources, the topology changes and is thus reconstructed before and after each scenario that does so. N-1 check is a typical use case.

As such, the following rule-of-thumb holds:

```{note}
Scenarios that change the same status attributes the same way should be fed to the power-grid-model together as much as possible. 
```

In practice, this means:

- In use cases that require many different parameter calculations for only a small set of different topologies, it is recommended to split the calculation in separate batches - one for each topology - to optimize performance.
- Otherwise, it is recommended to sort the scenarios by topology to minimize the amount of reconstructions.

### Batch data set

In the [Calculations documentation](calculations.md#batch-data-set), the distinction is made between independent and dependent batches.
Both types of batches allow for different performance optimizations.
To ensure that the right choice is always made, the following rule-of-thumb may be used:

```{note}
Sparsity of sampling should be reflected by sparsity in the batch update parameters and vice versa.
```

To elaborate:

- Dependent batches are useful for a sparse sampling for many different components, e.g. for N-1 checks.
- Independent batches are useful for a dense sampling of a small subset of components, e.g. time seris power flow calculation.

## Parallel computing

If the host system supports it, parallel computation is an easy way to gain performance.
As mentioned in the [Calculations](calculations.md#parallel-computing), letting the power-grid-model determine the amount of threads is recommended.

## Matrix prefactorization

Every iteration of power-flow or state estimation has a step of solving large number of sparse linear equations, i.e. `AX=b` in matrix form.
Computation wise this is a very expensive step.
One major component of this step is factorization of the `A` matrix.
In certain calculation methods, this `A` matrix and its factorization remains unchanged
over iterations and batches (only specific cases).
This makes it possible to reuse the factorization, skip this step and improve performance.

```{note}
Prefactorization over batches is possible when switching status or specified power values of load/generation or source reference voltage is modified.
It is not possible when topology or grid parameters are modified, i.e. in switching of branches, shunt, sources or change in transformer tap positions.
```
