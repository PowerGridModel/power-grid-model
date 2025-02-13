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
sparse block structure of the matrix equations by introducing new potentially non-zero elements,
called _fill-ins_, during the process of Gaussian elimination. To this end, a pre-fixed permutation
can be chosen to avoid bock pivoting at a later stage.

The [topological structure](#topological-structure) of the grid does not change during the
solving phase, so the permutation can be obtained by the minimum degree algorithm from just the
topology alone, which seeks to minimize the amount of fill-ins. Note that even if a matrix block
is topologically relevant does not imply that it is always non-zero. This may result in potentially
[ill-conditioned pivot elements](#pivot-perturbation), which will be discussed in a different
section.

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

#### Dense LU factorization process

The Gaussian elimination process itself is as usual. Let
$M_p\equiv\begin{bmatrix}m_p && \vec{r}_p^T \\ \vec{q}_p && \hat{M}_p\end{bmatrix}$, where $p$ is
the current pivot element at the top left of the matrix, $\vec{q}$ and $\vec{r}_p^T$ are the
associated column and row vectors containing the rest of the pivot column and row and $\hat{M}_p$ is
the bottom-right block of the matrix. Gaussian elimination constructs the matrices

$$
\begin{align*}
L_p &= \begin{bmatrix} 1 && \vec{0}^T \\ m_p^{-1} \vec{q}_p && \hat{D}_p\end{bmatrix}
Q_p &= \begin{bmatrix} m_p && \vec{r}_p^T \\ \vec{0} && \hat{M}_p - m_p^{-1} \vec{q}_p \vec{r}_p^T\end{bmatrix} \equiv \begin{bmatrix} 1 && \tilde{\vec{r}}_p^T \\ \vec{0} && M_{p+1} \end{bmatrix}
\end{align*}
$$

where $\hat{D}$ is the matrix with ones on the diagonal and zeros as off-diagonal elements.

Iterating this process yields the matrices

$$
\begin{align*}
L = L_0 L_1 \cdots L_{N-1} &= \begin{bmatrix}
1 && 0 && \cdots \\
m_0^{-1} \vec{q}_0 && 1 && \ddots \\
&& m_1^{-1} \vec{q}_1 && \ddots \\
&& && \ddots
\end{bmatrix} \\
Q = Q_0 Q_1 \cdots Q_{N-1} &= \begin{bmatrix}
m_0 && \vec{r}_0^T && && \\
0 && m_1 && \vec{r}_1^T && \\
\vdots && \ddots && \ddots && \ddots
\end{bmatrix}
\end{align*}
$$

The process described in the above assumes no pivot permutations were necessary. If permutations
are required, they are kept track of in a separate permution matrix, so that $A = LUP$.

#### Dense LU factorization algorithm

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

### Block-sparse LU factorization

The LU factorization process for block-sparse matrices is similar to that for dense matrices.

#### Block-sparse indices

The structure of the block-sparse matrices is as follows.

* The $N\times N$ block matrix $M$ is interpreted as $M\equiv M\left[0:N, 0:N\right]$, with
  $M\left[i,j\right]$ its block element at (0-based) indices $(i,j)$, where $i,j = 0..(N-1)$.
* In turn, let $M[i,j] \equiv M_{i,j}\left[0:N_{i,j},0:N_{i,j}\right]$ be the dense block with
  dimensions $N_i\times N_j$.

This can be graphically represented as

$$
\begin{align*}
M &\equiv \begin{pmatrix}
M_{0,0}   && \cdots && M_{0,N-1} \\
\vdots    && \ddots && \vdots \\
M_{N-1,0} && \cdots && M_{N-1,N-1}
\end{pmatrix} \\
&\equiv \begin{pmatrix}
\begin{pmatrix}
   M_{0,0}\left[0,0\right]     && \cdots && M_{0,0}\left[0,N_j-1\right] \\
   \vdots                      && \ddots && \vdots \\
   M_{0,0}\left[N_i-1,0\right] && \cdots && M_{0,0}\left[N_i-1,N_j-1\right]
\end{pmatrix} && \cdots && \begin{pmatrix}
   M_{0,N-1}\left[0,0\right]     && \cdots && M_{0,N-1}\left[0,N_j-1\right] \\
   \vdots                        && \ddots && \vdots \\
   M_{0,N-1}\left[N_i-1,0\right] && \cdots && M_{0,N-1}\left[N_i-1,N_j-1\right] \end{pmatrix} \\
\vdots && \ddots && \vdots \\
\begin{pmatrix}
   M_{N-1,0}\left[0,0\right]     && \cdots && M_{N-1,0}\left[0,N_j-1\right] \\
   \vdots                        && \ddots && \vdots \\
   M_{N-1,0}\left[N_i-1,0\right] && \cdots && M_{N-1,0}\left[N_i-1,N_j-1\right]
\end{pmatrix} && \cdots && \begin{pmatrix}
   M_{N-1,N-1}\left[0,0\right]     && \cdots && M_{N-1,N-1}\left[0,N_j-1\right] \\
   \vdots                          && \ddots && \vdots \\
   M_{N-1,N-1}\left[N_i-1,0\right] && \cdots && M_{N-1,N-1}\left[N_i-1,N_j-1\right] \end{pmatrix}
\end{pmatrix}
\end{align*}
$$

Because of the sparse structure and the fact that all $M_{i,j}$ have the same shape, it is much more
efficient to store the blocks $M_{i,j}$ in a vector $M_{\tilde{k}}$ where $\tilde{k}$ is a reordered
index from $(i,j) \mapsto \tilde{k}$. This mapping, in turn, is stored as an index pointer, i.e., a
vector of vectors of indices, with the outer index given by the row-index $i$, and the inner vector
containing the values of $j$ for which $M_{i,j}$ may be non-zero. All topologically relevant matrix
elements, as well as [fill-ins](#pivot-operations), are included in this mapping. The following
illustrates this mapping.

$$
\begin{align*}
\begin{pmatrix}
M_{0,0} &&         &&         && M_{0,3} \\
        && M_{1,1} && M_{1,2} &&         \\
        && M_{2,1} && M_{2,2} && M_{2,3} \\
M_{3,0} &&         && M_{3,2} && M_{3,3}
\end{pmatrix} &\equiv
\begin{bmatrix}
M_{0,0} && M_{0,3} && M_{1,1} && M_{1,2} && M_{2,1} && M_{2,2} && M_{2,3} && M_{3,0} && M_{3,2} && M_{3,3} && \\
[[0 && 3] && [1 && 2] && [1 && 2 && 3] && [0 && 2 && 3]] && \\
[0 && && 2 && && 4 && && 7 && && 10]
\end{bmatrix}
\end{align*}
$$

In the right-hand side, the upper row contains the present block entries and the bottom row their
column indices. The row indices are implied by the index of the inner vector in the outer one.

Looping over the rows and columns becomes trivial. Let $\text{indptr}$ be the nested list of column
indices. The double loop becomes:

1. Loop all rows: $i=0..(N-1)$:
   1. The list of indices for this row is 

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
be the matrix, $\left\|M\right\|_{\infty ,\text{bwod}}$ the
[block-wise off-diagonal infinite norm](#block-wise-off-diagonal-infinite-matrix-norm) of the matrix.

1. Set $\epsilon \gets \text{perturbation_threshold} * \left\|M\right\|_{\text{bwod}}$
2. If $|\text{pivot_element}| \lt \epsilon$, then:
   1. If $|\text{pivot_element}| = 0$, then:
      1. Set $\text{direction} = 1$.
      2. Proceed.
   2. Else:
      1. Set $\text{direction}\left(\text{pivot_element}\right) \gets \text{pivot_element} / |\text{pivot_element}|$.
      2. Proceed.
   3. Set $\text{pivot_element} \gets \epsilon * \text{direction}\left(\text{pivot_element}\right)$.

$\text{direction}$ ensures that the complex phase of the pivot element is preserved, with a fallback
the positive real axis when the pivot element is identically zero.

#### Iterative refinement

This algorithm is heavily inspired by the GESP algorithm described in
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761).

The refinement process improves the solution to the matrix equation $A \cdot x = b$ as follows:
In iteration step $i$, it assumes an existing approximation $x_i$ for $x$. It then defines
the difference between the current best and the actual solution $\Delta x = x - x_i$.
Substiting in the original equation yields $A \cdot (x_i + \Delta x) = b$, so that
$A \cdot \Delta x = b - A \cdot x_i =: r$, where the residual $r$ can be calculated.
An estimation for the left-hand side can be obtained by using the pivot-perturbed matrix
$\tilde{A}$ instead of the original matrix A. Convergence can be reached if $r \to 0$, since then
also $\left\|\Delta x\right\| \to 0$. Solving for $\Delta x$ and substituting back into
$x_{i+1} = x_i + \Delta x$ provides the next best approximation $x_{i+1}$ for $x$.

A measure for the quality of the approximation is given by the $\text{backward_error}$ (see also
[backward error formula](#improved-backward-error-calculation)).

Since the matrix $A$ does not change during this process, the LU decomposition remains valid
throughout the process, so that this iterative refinement can be done at a reasonably low cost.

Given the original matrix equation $A \cdot x = b$ to solve, the pivot perturbated matrix
$\tilde{A}$ with a pre-calculated LU-decomposition, and the convergence threshold $\epsilon$,
the algorithm is as follows:

1. Initialize:
   1. Set the initial estimate: $x_{\text{est}} = 0$.
   2. Set the initial residual: $r \gets b$.
   3. Set the initial backward error: $\text{backward_error} = \infty$.
   4. Set the number of iterations to 0.
2. Iteratively refine; loop:
   1. Check stop criteria:
      1. If $\text{backward_error} \leq \epsilon$, then:
         1. Convergence reached: stop the refinement process.
      2. Else, if the number of iterations > maximum allowed amount of iterations, then:
         1. Convergence not reached; iterative refinement not possible: raise a sparse matrix
            error.
      3. Else:
         1. Increase the number of iterations.
         2. Proceed.
   2. Solve $\tilde{A} \cdot \Delta x = r$ for $\Delta x$.
   3. Calculate the backward error with the original $x$ and $r$ using the [backward error formula](#improved-backward-error-calculation).
   4. Set the next estimation of $x$: $x_{\text{est}} \gets x_{\text{est}} + \Delta x$.
   5. Set the residual: $r \gets b - A \cdot x$.

Because the backward error is calculated on the $x$ and $r$ from the previous iteration, the
iterative refinement loop will always be executed twice.

The reason a sparse matrix error is raised and not an iteration diverge error, is that it is the
iterative refinement of the matrix equation solution that cannot be solved in the set amount of
iterations - not the set of power system equations. This will only happen when the matrix
equation requires iterative refinement in the first place, which happens only when pivot
perturbation is needed, namely in the case of an ill-conditioned matrix equation.

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

1. If $\text{backward_error} \gt \frac{1}{2}\text{last_backward_error}$, then:
   1. Stop iterative refinement.
2. Else:
   1. Go to next refinement iteration.

In power systems, however, the fact that the matrix may contain elements
[spanning several orders of magnitude](#element-size-properties-of-power-system-equations) may cause
slow convergence far away from the optimum. The diminishing return criterion would cause the
algorithm to exit before the actual solution is found. Multiple refinement iterations may still
yield better results. The power grid model therefore does not stop on deminishing returns. Instead, a maximum amount of iterations is used in combination with the error tolerance.

##### Improved backward error calculation

In power system equations, the matrix equation $A x = b$ can be very unbalanced: some entries
in the matrix $A$ may be very large while others are zero or very small. The same may be true for
the right-hand side of the equation $b$, as well as its solution $x$. In fact, there may be
certain rows $i$ for which both $\left|b[i]\right|$ and
$\sum_j \left|A[i,j]\right| \left|x[j]\right|$ are small and, therefore, their sum is prone to
rounding errors, which may be several orders larger than machine precision.

[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
uses the following backward error in the [iterative refinement algorithm](#iterative-refinement):

$$
\begin{align*}
\text{backward_error}_{\text{Li}} &= \max_i \frac{\left|r_i\right|}{\sum_j \left|A_{i,j}\right| \left|x_j\right| + \left|b_i\right|} \\
&= \max_i \frac{\left|b_i - \sum_j A_{i,j} x_j\right|}{\sum_j \left|A_{i,j}\right| \left|x_j\right| + \left|b_i\right|} \\
&= \max_i \frac{\left|r_i\right|}{\left(\left|A\right| \cdot \left|x\right| + \left|b\right|\right)_i}
\end{align*}
$$

In this equation, the symbolic notation $\left|A\right|$ and $\left|x\right|$ are the matrix and
vector with absolute values of the elements of $A$ and $x$ as elements, i.e.,
$\left|A\right|_{i,j} := \left|a_{i,j}\right|$ and $\left|x\right|_i := \left|x_i\right|$, as
defined in [Arioli89](https://epubs.siam.org/doi/10.1137/0610013).

Due to aforementioned, this is prone to rounding errors, and a single row with rounding errors may
cause the entire iterative refinement to fail. The power grid model therefore use a modified
version, in which the denominator is capped to a minimum value, determined by the maximum across all
denominators:

$$
\begin{align*}
D_{\text{max}} &= \max_i\left\{\left(\left|A\right|\cdot\left|x\right| + \left|b\right|\right)_i\right\} \\
\text{backward_error} &= \max_i \left\{
   \frac{\left|r\right|_i}{
      \max\left\{
         \left(\left|A\right|\cdot\left|x\right| + \left|b\right|\right)_i,
         \epsilon_{\text{backward_error}} D_{\text{max}}
      \right\}
   }
\right\}
\end{align*}
$$

$\epsilon$ may be chosen. $\epsilon = 0$ means no cut-off, while $\epsilon = 1$ means that only the
absolute values of the residuals are relevant - not the relative values. The former is prone to
rounding errors, while the latter may hide issues in rows with small coefficients by supressing them
in the backward error, even if that row's residual is relatively large compared to the other
entries, in favor of other rows with larger absolute, but smaller relative, residuals. In
conclusion, $\epsilon$ should be chosen not too large and not too small.

```{note}
$\epsilon = 10^{-4}$ was experimentally determined to be a reasonably good value on a number of
real-world MV grids.
```

##### Block-wise off-diagonal infinite matrix norm

For the [pivot perturbation algorithm](#pivot-perturbation-algorithm), a matrix norm is used to
determine the relative size of the current pivot element compared to the rest of the matrix as a
measure for the degree of ill-conditioning. The norm is a variant of the $L_{\infty}$ norm of a
matrix, which we call the block-wise off-diagonal infinite matrix norm
($L_{\infty ,\text{bwod}}$).

Since the power grid model solves the matrix equations using a multi-scale matrix solver (dense
intra-block, block-sparse for the full topological structure of the grid), the norm is also taken on
those same levels, so the calculation of the norm is _block-wise_.

In addition, the diagonal blocks may have much larger elements than the off-diagonal elements, while
the relevant information is contained mostly in the off-diagonal blocks. As a result, the
block-diagonal elements would undesirably dominate the norm. The power grid model therefore
restricts the calculation of the norm to _off-diagonal_ blocks.

In short, the $L_{\infty ,\text{bwod}}$-norm it is the $L_{\infty}$ norm of the block-sparse matrix
with the $L_{\infty}$ norm of the individual blocks as elements, where the block-diagonal elements
are skipped at the block-level.

###### Block-wise off-diagonal infinite matrix norm algorithm

The algorithm is as follows:

Let $M\equiv M\left[0:N, 0:N\right]$ be the $N\times N$-matrix with a block-sparse structure and
$M\left[i,j\right]$ its block element at (0-based) indices $(i,j)$, where $i,j = 0..(N-1)$. In turn,
let $M[i,j] \equiv M_{i,j}\left[0:N_{i,j},0:N_{i,j}\right]$ be the dense block with dimensions
$N_i\times N_j$.

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
            4. Continue with the next row of the current block.
         4. Set $\text{row_norm} \gets \text{row_norm} + \text{block_norm}$.
         5. Continue with the next block-column.
   3. Calculate the new norm: set
      $\text{norm} \gets \max\left\{\text{norm}, \text{row_norm}\right\}$.
   4. Continue with the next block-row.

###### Illustration of the block-wise off-diagonal infinite matrix norm calculation

This section aims to illustrate how the $L_{\infty ,\text{bwod}}$-norm differs from a regular
$L_{\infty}$-norm using the following examples.

The first example shows how the taking the block-wise norm affects the calculation of the norm.

$$
\begin{pmatrix}
\begin{pmatrix}
0 && 0 \\
0 && 0
\end{pmatrix} && \begin{pmatrix}
1 && 0 \\
0 && 3
\end{pmatrix} && \begin{pmatrix}
3 && 0 \\
0 && 0
\end{pmatrix} \\
\begin{pmatrix}
5 && 0 \\
0 && 0
\end{pmatrix} &&
\begin{pmatrix}
0 && 0 \\
0 && 0
\end{pmatrix} && \begin{pmatrix}
0 && 0 \\
0 && \frac{1}{2}
\end{pmatrix} \\
\begin{pmatrix}
0 && 0 \\
0 && 0
\end{pmatrix} &&
\begin{pmatrix}
0 && 0 \\
0 && 0
\end{pmatrix} &&
\begin{pmatrix}
1 && 0 \\
0 && 1
\end{pmatrix}
\end{pmatrix}
$$

* The regular $L_{\infty}$-norm is $\max\left\{1+3, 3, 5, \frac{1}{2}, 1, 1\right\} = 5$.
* The block-wise off-diagonal infinity $L_{\infty ,\text{bwod}}$-norm is
  $\max\left\{0+\max\left\{1, 3\right\}+\max\left\{3, 0\right\},\max\left\{5, 0\right\} + \max\left\{0, \frac{1}{2}\right\}, 1\right\} = \max\left\{3+3, 5+\frac{1}{2}, 1, 1\right\} = 6$.

The two norms clearly differ and even the elements that contribute most to the norm are different.

The next example shows how keeping only the off-diagonal blocks affects the norm.

$$
\begin{pmatrix}
\begin{pmatrix}
20 && 20 \\
30 && 0
\end{pmatrix} && \begin{pmatrix}
2 && 2 \\
3 && 0
\end{pmatrix} \\
\begin{pmatrix}
0 && 0 \\
0 && 3
\end{pmatrix} && \begin{pmatrix}
100 && 0 \\
0 && 1
\end{pmatrix}
\end{pmatrix}
$$

* The regular $L_{\infty}$-norm is $\max\left\{20+20+2+2,30+3,100,3+1\right\} = \max\left\{44,33,100,4\right\} = 100$.
* The block-wise infinity norm with diagonals would be
  $\max\left\{\max\left\{20+20, 30\right\}+\max\left\{2+2, 3\right\},\max\left\{0,3\right\} + \max\left\{100, 1\right\}\right\} = \max\left\{40+4, 3+100\right\} = \max\left\{44, 103\right\} = 103$.
* The $L_{\infty ,\text{bwod}}$-norm is
  $\max\left\{\max\left\{2+2, 3\right\},\max\left\{0,3\right\}\right\} = \max\left\{4, 3\right\} = 4$.
