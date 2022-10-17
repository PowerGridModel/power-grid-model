<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Guidelines for performance enhancement

```{warning}
[Issue 79](https://github.com/alliander-opensource/power-grid-model/issues/79)
```
% TODO Explain grid types interpreted by power-grid-model and its relation with performance or any other tips/tricks//hacks (Check examples)

## Independent / cache topology

## Parallel Computing

## Matrix prefactorization

Every iteration of power-flow or state estimation has a step of solving large number of sparse linear equations
i.e. `AX=b` in matrix form. Computation wise this is a very expensive step. One major component of this step is
factorization of the `A` matrix. In certain calculation methods, this `A` matrix and its factorization remains unchanged
over iterations and batches (only specific cases). This makes it possible to reuse the factorization, skip this step and
improve performance.

**Note:** Prefactorization over batches is possible when switching status or specified power values of load/generation
or source reference voltage is modified. It is not possible when topology or grid parameters are modified, i.e. in
switching of branches, shunt, sources or change in transformer tap positions.

