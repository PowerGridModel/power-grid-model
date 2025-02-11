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

#### Matrix properties of power system equations

[Power flow equations](../../user_manual/calculations.md#power-flow-algorithms) are not Hermitian
and also not positive (semi-)definite. As a result, methods that depend on these properties cannot
be used.

[State estimation equations](../../user_manual/calculations.md#state-estimation-algorithms) are
intrinsically positive definite and Hermitian, but for
[performance reasons](#performance-considerations), the matrix equation is augmented to achieve
a consistent structure across the entire topology using Lagrange multipliers.

#### Element size properties of power system equations

Power system equations may contain elements of several orders of magnitude. This may lead to
instabilities when solving the (matrix) equations due to non-linearly propagating (rounding) errors.
Stability checks that limit those rounding errors need to function under these extreme conditions.

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

* A block-sparse LU solver that:
  * handles factorization using the topological structure up to block-level
  * solves the sparse matrix equation given the factorization
* A dense LU factor that handles individual blocks within the matrix equation

### Dense LU factorization

The power grid model uses a modified version of the
[`LUFullPiv` defined in Eigen](https://gitlab.com/libeigen/eigen/-/blob/3.4/Eigen/src/LU/FullPivLU.h)
(credits go to the original author). The modification adds opt-in support for
[pivot perturbation](#pivot-perturbation).

Let $M\equiv M\left[0:N, 0:N\right]$ be the $N\times N$-matrix and $M\left[i,j\right]$ its element
at (0-based) indices $(i,j)$, where $i,j = 0..(N-1)$.

1. Loop over all rows: $p = 0..(N-1)$:
   1. Set the remaining matrix: $M_p \gets M\left[p:N,p:N\right]$ with size $N_p\times N_p$.
   2. Find largest element $M_p\left[i_p,j_p\right]$ in $M_p$ by magnitude. This is the pivot
      element.
   3. If the magnitude of the pivot element is too small:
      1. If pivot perturbation is enabled, then:
         1. Apply [pivot perturbation](#pivot-perturbation).
         2. Proceed.
      2. Else, if the matrix is singular (pivot element is identically $0$), then:
         1. Raise a `SparseMatrixError`.
      3. Else:
         1. Proceed.
   4. Else:
      1. Proceed.
   5. Swap the first and pivot row and column of $M_p$, so that the pivot element is in the
      top-left corner of the remaining matrix:
      1. $M_p\left[0,0:N_p\right] \leftrightarrow M\left[i_p,0:N_p\right]$
      2. $M_p\left[0:N_p,0\right] \leftrightarrow M\left[0:N_p,j_p\right]$
   6. Apply Gaussian elimination for the current pivot element:
      1. $M_p\left[0,0:N_p\right] \gets \frac{1}{M_p[0,0]}M_p\left[0,0:N_p\right]$
      2. $M_p\left[1:N_p,0:N_p\right] \gets M_p\left[1:N_p,0:N_p\right] - M_p\left[1:N_p,0\right] \otimes M_p\left[0,0:N_p\right]$
   7. Continue with the next $p$ to factorize the the bottom-right block.

```{note}
In the actual implementation, we use a slightly different order of operations: instead of raising
the `SparseMatrixError` immediately, we break from the loop and throw after that. This does not
change the functional behavior.
```

### Block-sparse LU decomposition

TODO

### Pivot perturbation

The LU solver implemented in the power grid model has support for pivot perturbation. The methods
are described in
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
and
[Schenk06](https://etna.math.kent.edu/volumes/2001-2010/vol23/abstract.php?vol=23&pages=158-179).
Pivot perturbation consists of selecting a pivot element. If its magnitude is too small compared
to that of the other elements in the matrix, then it cannot be used in its current form. Selecting
another pivot element is not desirable, as described in the section on
[pivot operations](#pivot-operations), so the matrix is ill-conditioned.

Instead, a small perturbation is done on the pivot element. This makes the matrix equation solvable
without selecting a different pivot element, at the cost of propating rounding errors.
slightly different matrix that is then iteratively refined.

#### Pivot perturbation algorithm

The following pivot perturbation algorithm works for both real and complex matrix equations. Let $M$
be the matrix, $\left\||A\right\||_{\infty ,\text{bwod}}$ the
[block-wise off-diagonal infinite norm](#block-wise-off-diagonal-infinite-matrix-norm) of the matrix.

1. Set $\epsilon \gets \text{perturbation_threshold} * \left\||A\right\||_{\text{bwod}}$
2. If $|\text{pivot_element}| < \epsilon$, then:
   1. Set $\text{phase}\left(\text{pivot_element}\right) \gets \text{pivot_element} / |\text{pivot_element}|$.
   2. Set $\text{pivot_element} \gets \text{perturbation_threshold} * \text{phase}\left(\text{pivot_element}\right)$.

<!-- In this equation, $\left\||A\right\||$ denotes the absolute value of the maximum element, as defined in
[Arioli89](https://epubs.siam.org/doi/10.1137/0610013). -->

#### Iterative refinement

TODO

#### Differences with literature

There are several differences between our implementation and the ones described in
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
and
[Schenk06](https://etna.math.kent.edu/volumes/2001-2010/vol23/abstract.php?vol=23&pages=158-179).
They are summarized below.

##### No check for diminishing returns

[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
contains an early-out criterion for the iterative refinement that checks for deminishing returns in
consecutive iterations. It amounts to (in reverse order):

1. If $$\text{backward_error} \gt \frac{1}{2}\text{last_backward_error}$$, then:
   1. Stop iterative refinement.
2. Else:
   1. Go to next refinement iteration.

In power systems, however, the fact that the matrix may contain elements
[spanning several orders of magnitude](#element-size-properties-of-power-system-equations) may cause
slow convergence far away from the optimum. The diminishing return criterion would cause the
algorithm to exit before the actual solution is found. Multiple refinement iterations may still
yield better results. The power grid model therefore does not stop on deminishing returns. Instead,
a maximum amount of iterations is used in combination with the error tolerance.

##### Block-wise off-diagonal infinite matrix norm

For the pivot perturbation algorithm, a matrix norm is used to determine the relative size of the
current pivot element compared to the rest of the matrix as a measure for the ill-conditioning. The
norm is a variant to the $L_{\infty}$ norm of a matrix, which we call the block-wise off-diagonal
infinite matrix norm ($L_{\infty ,\text{bwod}}$).

Since the power grid model solves the matrix equations using a multi-scale matrix solver (dense
intra-block, block-sparse for the full topological structure of the grid), the norm is also taken on
two levels.

In addition, because the diagonal blocks may have much larger elements than the off-diagonal
elements, while the relevant information is contained mostly in in the off-diagonal blocks. As a
result, the block-diagonal elements would undesirably dominate the norm. The power grid model
therefore skips the diagonal blocks when calculating the norm.

In short, the $L_{\infty ,\text{bwod}}$ norm it is the $L_{\infty}$ norm of the block-sparse matrix
with the $L_{\infty}$ norm of the individual blocks as elements, where the block-diagonal elements
are skipped at the block-level. The algorithm is as follows:

Let $M\equiv M\left[0:N, 0:N\right]$ be the $N\times N$-matrix with a block-sparse structure and
$M\left[i,j\right]$ its block element at (0-based) indices $(i,j)$, where $i,j = 0..(N-1)$. In turn,
let $M[i,j] \equiv M_{i,j}\left[0:N_{i,j},0:N_{i,j}\right]$ be the dense block with dimensions

1. Set $\text{norm} \gets 0$.
2. Loop over all block-rows: $i = 0..(N-1)$:
   1. Set $\text{row_norm} \gets 0$.
   2. Loop over all block-columns: $j = 0..(N-1)$ (beware of sparse structure):
      1. If $i = j$, then:
         1. Skip this block: continue with the next block-column.
      2. Else, calculate the $L_{\infty}$ norm of the current block and add to the current row norm:
         1. Set the current block: $M_{i,j} \gets M[i,j]$.
         2. Set $\text{block_norm} \gets 0$.
         3. Loop over all rows of the current block: $k = 0..(N_{i,j} - 1)$:
            1. Set $\text{block_row_norm} \gets 0$.
            2. Loop over all columns of the current block: $l = 0..(N_{i,j} - 1)$:
               1. Set $\text{block_row_norm} \gets \text{block_row_norm} + \left\|M_{i,j}\left[k,l\right]\right\|$.
            3. Calculate the new block norm: set
               $\text{block_norm} \gets \max\left\{\text{block_norm}, \text{block_row_norm}\right\}$.
         4. Set $\text{row_norm} \gets \text{row_norm} + \text{block_norm}$.
   3. Calculate the new norm: set $\text{norm} \gets \max\left\{\text{norm}, \text{row_norm}\right\}$.
