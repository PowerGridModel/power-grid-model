<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# LU Solver

Power system equations can be modeled as matrix equations. A matrix equation solver is therefore
key to the power grid model.

This section documents the need for a custom sparse LU solver and its implementation.

## Background

The choice for the matrix equation solver type heavily leans on the need for
[performance](#performance-considerations), the
[topological structure](#topological-structure) of the grid and the
[properties of the equations](#power-system-equation-properties). They are documented here.

### Performance considerations

There is a large variety of usages of the power grid model that require good performance. This
imposes some constraints on the algorithms that can be used.

* Highly accurate and fast calculations are needed for very large grids. This means that direct
  methods are strongly preferred, and approximate methods can only be used when there is no other
  alternative, and only if can be iteratively refined with a fast convergence rate.
* Sometimes, very many repetitive calculations are required, e.g., for time series. In those cases,
  separating the decomposition of a matrix and solving two systems of equations separately can give
  major performance boosts.

### Topological structure

Distribution grids consist of substations that distribute power in a region. This can be represented
in a topological way as vertices and edges. As a consequence of the locality of Kirchoff's laws,
power system equations also take on the same topological structure in block-matrix equation form.

#### Sparsity

It is common that a substation is fed by a single upstream substation, i.e., most grids are operated
in a tree-like structure. Meshed grid operations are rare, and even when they do happen, it is
usually only for a small section of the grid. All this gives rise to extremely sparse topologies
and, as a result, extremely sparse matrix equations with a block structure.

Sparse matrix equations can be solved efficiently: they can be solved in linear time complexity, as
opposed to the cubic complexity of naive Gaussian elimination. As a result, a sparse matrix solver
is key to the performance of the power grid model. QR decomposition therefore is not a good
candidate.

#### Pivot operations

Pivoting of blocks is expensive, both computationally and memory-wise, as it interferes with the
sparse block structure of the matrix equations. To this end, a pre-fixed permutation can be chosen
to avoid bock pivoting at a later stage.

The [topological structure](#topological-structure) of the grid does not change during the
solving phase, so the permutation can be obtained by the minimum degree algorithm from just the
topology alone, at the cost of potential [ill-conditioned pivot elements](#pivot-perturbation).

### Power system equation properties

[Power flow equations](../../user_manual/calculations.md#power-flow-algorithms) are not Hermitian
and also not positive (semi-)definite. As a result, methods that depend on these properties cannot
be used.

[State estimation equations](../../user_manual/calculations.md#state-estimation-algorithms) are
intrinsically positive definite and Hermitian, but for
[performance reasons](#performance-considerations), the matrix equation is augmented to achieve
a consistent structure across the entire topology using Lagrange multipliers.

### Block-sparsity considerations

The power flow and state estimation equations involve block-sparse matrices: dense blocks, with a
dimensionality varying between different calculation types, methods and symmetries, are distributed
across the matrix in an extremely sparse way. The sparse structure can be pre-calculated, but the
dense blocks need to be inverted separately. To make matters worse, the dense blocks may differ
heavily in structure and contents between different nodes, and are often not solvable without
pivoting.

### Custom sparse LU solver

The choice for a custom LU solver implementation comes from a number of considerations:

* LU-decomposition is the best choice, because QR-decomposition and Cholesky decomposition cannot
  solve the power system equations efficiently as a consequence of the properties
* Alternative LU solver implementations are optimized for a variety of use cases that are less
  sparse than the ones encountered in power systems.
* Alternative LU solver implementations do not have good block-sparse matrix equation solvers.

## Implementation

The LU solver implemented in the power grid model consists of 3 components:

* A sparse LU solver that:
  * handles factorization using the topological structure up to block-level
  * solves the sparse matrix equation given the factorization
* A dense LU factor that handles individual blocks within the matrix equation

### Pivot perturbation

The LU solver implemented in the power grid model has support for pivot perturbation. The methods
are described in [Li99](https://portal.nersc.gov/project/sparse/superlu/siam_pp99.pdf) and
[Schenk04](http://ftp.gwdg.de/pub/misc/EMIS/journals/ETNA/vol.23.2006/pp158-179.dir/pp158-179.pdf).
Pivot perturbation consists of selecting a pivot element. If its magnitude is too small compared
to that of the other elements in the matrix, then it cannot be used in its current form. Selecting
another pivot element is not desirable, as described in the section on
[pivot operations](#pivot-operations), so the matrix is ill-conditioned.

Instead, a small perturbation is done on the pivot element. This makes the matrix equation solvable
without selecting a different pivot element, at the cost of propating rounding errors.
slightly different matrix that is then iteratively refined.

#### Pivot perturbation algorithm

\begin{align*}
A = B && C = D \\
&& E = F
\end{align*}
<!-- if (|pivot_element| < perturbation_threshold) then
    pivot_element = perturbation_threshold
endif -->

### Dense LU factorization

The power grid model uses a modified version of the
[`LUFullPiv` defined in Eigen](https://gitlab.com/libeigen/eigen/-/blob/3.4/Eigen/src/LU/FullPivLU.h)
(credits go to the original author). The modification adds opt-in support for
[pivot perturbation](#pivot-perturbation).

1. Set the remaining square matrix
2. Find largest element in the remaining matrix by magnitude. This is the pivot element.
3. If the magnitude of the pivot element is too small:
   1. If pivot perturbation is enabled, apply [pivot perturbation](#pivot-perturbation).
   2. Otherwise, if the matrix is exactly singular (pivot element is identically $0$) is disabled,
      raise a `SparseMatrixError`.
