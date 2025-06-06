<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Guidelines for performance enhancement

The `power-grid-model` is a library that shines in its ability to handle calculations at scale.
It remains performant, even when doing calculations with one or a combination of the following extremes
(non-exhaustive):

- Large grids
- Batch calculations with many scenarios
- Many changes in the grid in each scenario

To achieve that high performance, several optimizations are made.
To use those optimizations to the fullest, we recommend our users to follow the following guidelines.

## Data validity

Many of our optimizations assume input data validity and rely on the fact that the provided grid is reasonably close to
realistic.
Non-convergence, underdetermined equations or other unexpected behavior may therefore be encountered when the data is
not realistic.

To keep the PGM performant, checks on hard physical bounds are offloaded to a separate tool, i.e., the
[data validator](data-validator.md).
However, these checks can be prohibitively expensive and application at scale in production environments is therefore
not recommended when performance matters.
Instead, we recommend using the data validator specifically for debugging purposes.

```{note}
Some combinations of input data are not forbidden by physics, but still pose unrealistic conditions, e.g., a source with
a very low short-circuit power.
These cases may result in unexpected behavior of the calculation core.
Vagueness and case-dependence make it hard to check what can be considered "unrealistic", and the
[data validator](data-validator.md) will therefore not catch such cases.
We recommend our users to provide reasonably realistic scenarios to prevent these edge cases from happening.
```

## Data format

The data format of input, output and update data can have a big effect on memory and computational cost.

### Input/update data volume

Row-based data (created, e.g., using {py:class}`power_grid_model.initialize_array` in Python) constructs input/update
data with all attributes for a given dataset type.
However, many component attributes are optional.
If your use case does not depend on these attributes, a lot of data is needlessly created and initialized.
If you are running on a system where memory is the bottle-neck, using a columnar data format may reduce the memory
footprint.
This may or may not induce a slight computational overhead during calculations.

### Output data volume

For most use cases, only certain output values are relevant.
For example, if you are only interested in line loading, outputting all other components and attributes results in
unnecessary overhead.
The output data may be a significant, if not the dominant, contributor to memory load, particularly when running large
batch calculations.
We therefore recommend restricting the output data to only the components and attributes that are used by the user in
such production environments.
In Python, it is possible to do so by using the `output_component_types` keyword argument in the `calculate_*` functions
(like {py:class}`power_grid_model.PowerGridModel.calculate_power_flow`)

### Database integration

Most databases store their data in a columnar data format.
Copying, reserving unused memory, and cache misses can lead to unnecessary memory usage and computational overhead.
With the introduction of columnar data input to PGM, integrating with databases using this format becomes easier, more
natural, and more efficient.

## Batch calculations

Depending on the details of the batch, a number of performance optimizations are possible:

- [Topology constructions](#topology-caching), especially, may significantly impact the computation time of a scenario.
- The way the [batch data set](#batch-data-set) is provided to the model can also affect the performance.

### Topology caching

Topology is an expensive step in calculations.
Fortunately, the topology can be cached when there are no structural changes to the power grid itself.
For the power-grid-model, this is the case when there no changes to statuses (`from_status`, `to_status`, `status`,
etc.) of the following components:

1. Branches: Lines, Links, Transformers
2. Branch3: Three winding transformer
3. Appliances: Sources

In particular, the topology is cached in the following way:

- If none of the provided batch scenarios change the status of branches and sources, the model will re-use the pre-built
  internal graph/matrices for each calculation.
  Time-series load profile calculation is a typical use case.
- If some batch scenarios are changing the switching status of branches and sources, the topology changes and is thus
  reconstructed before and after each scenario that does so.
  N-1 check is a typical use case.

As such, the following rule-of-thumb holds:

```{note}
Scenarios that change the same status attributes the same way should be fed to the power-grid-model together as much as
possible.
```

In practice, this means:

- In use cases that require many different parameter calculations for only a small set of different topologies, it is
- recommended to split the calculation in separate batches - one for each topology - to optimize performance.
- Otherwise, it is recommended to sort the scenarios by topology to minimize the amount of reconstructions.

### Batch data set

In the [Calculations documentation](calculations.md#batch-data-set), the distinction is made between independent and
dependent batches.
Both types of batches allow for different performance optimizations.
To ensure that the right choice is always made, the following rule-of-thumb may be used:

```{note}
Sparsity of sampling should be reflected by sparsity in the batch update parameters and vice versa.
```

To elaborate:

- Dependent batches are useful for a sparse sampling for many different components, e.g. for N-1 checks.
- Independent batches are useful for a dense sampling of a small subset of components, e.g. time series power flow
- calculation.

## Parallel computing

If the host system supports it, parallel computation is an easy way to gain performance.
As mentioned in the [Calculations](calculations.md#parallel-computing), letting the power-grid-model determine the
amount of threads is recommended.

## Matrix prefactorization

Every iteration of power-flow or state estimation has a step of solving large number of sparse linear equations, i.e.
`AX=b` in matrix form.
Computation wise this is a very expensive step.
One major component of this step is factorization of the `A` matrix.
In certain calculation methods, this `A` matrix and its factorization remains unchanged over iterations and batches
(only specific cases).
This makes it possible to reuse the factorization, skip this step and improve performance.

```{note}
Prefactorization over batches is possible when switching status or specified power values of load/generation or source
reference voltage is modified.
It is not possible when topology or grid parameters are modified, i.e. in switching of branches, shunt, sources or
change in transformer tap positions.
```
