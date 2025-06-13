<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# LU Solver

Power system equations can be modeled as matrix equations.
A matrix equation solver is therefore key to the power grid model.

This section documents the need for a custom sparse LU solver and its implementation.

```{contents}
```

## Background

The choice for the matrix equation solver type heavily leans on the need for [performance](#performance-considerations),
the [topological structure](#topological-structure) of the grid, and the
[properties of the equations](#power-system-equations-properties).
They are documented here.

### Performance considerations

There is a large variety of applications of the power grid model that requires high performance.
This imposes some limitations on the algorithms that can be used.

* Highly accurate and fast calculations are needed for very large grids.
  This means that direct methods are strongly preferred, and approximate methods can only be used when there is no other
  alternative, and only if they can be iteratively refined with a fast convergence rate.
* In applications like time series, repetitive calculations are required.
  In those cases, separating the decomposition (also known as factorization) step of a matrix and the solving step can
  bring major performance benefits, as the decomposition remains the same across the entire set of calculations.

### Topological structure

Distribution grids consist of substations that distribute power in a region.
This can be represented in a topological way as graphs containing vertices and edges.
As a consequence of the locality of Kirchoff's laws, power system equations also take on the same topological structure
in block-matrix equation form.

#### Sparsity

It is common that a substation is fed by a single upstream substation, i.e., most grids are operated in a tree-like
structure.
Meshed grid operations are relatively rare and are generally restricted to small sub-sections of the grid, although
exceptions exist.
All this gives rise to extremely sparse topologies and, as a result, extremely
[sparse matrix equations](https://en.wikipedia.org/wiki/Sparse_matrix) with a
[block structure](https://en.wikipedia.org/wiki/Block_matrix).

Sparse matrix equations can be solved efficiently: they can be solved in linear time complexity, as opposed to the cubic
complexity of naive [Gaussian elimination](https://en.wikipedia.org/wiki/Gaussian_elimination).
As a result, a sparse matrix solver is key to the performance of the power grid model.
QR decomposition therefore is not a good candidate.

#### Pivot operations

In matrix solvers, it is very common practice to use the top-left diagonal element as the pivot point of all
consequitive operations.
This allows in-place operations and enhances computational performance.
This, however, does not guarantee numerical stability.
Instead, a different pivot element (often the matrix element with the largest norm) may be selected and moved to the
top-left diagonal element by permuting the rows and columns of the matrix.
This so-called _pivoting_ enhances the numerical stability of consequtive operations.

Pivoting of blocks in sparse matrices is expensive, both time-wise and memory-wise.
During the process of [Gaussian elimination](https://en.wikipedia.org/wiki/Gaussian_elimination), the pivoted sparse
block structure of the matrix equations contains newly-introduced, potentially non-zero elements, called _fill-ins_.
To this end, a pre-fixed permutation can be chosen to avoid block pivoting at a later stage, at the cost of some
numerical stability.

The [topological structure](#topological-structure) of the grid does not change during the solving phase.
The permutation can be obtained from just topology alone.
This is typically done with the [minimum degree algorithm](https://en.wikipedia.org/wiki/Minimum_degree_algorithm),
which seeks to minimize the amount of fill-ins.
Note that matrix blocks that contribute topologically to the matrix equation can still contain zeros.
It is possible that such zeros result in [ill-conditioned pivot elements](#pivot-perturbation).
Handling of such ill-conditioned cases is discussed in the [section on pivot perturbation](#pivot-perturbation).

### Power system equations properties

#### Matrix properties of power system equations

The matrices involved in [power flow equations](../user_manual/calculations.md#power-flow-algorithms) are not Hermitian,
nor positive (semi-)definite.
As a result, methods that depend on that property cannot be used.

The matrices involved in [state estimation equations](../user_manual/calculations.md#state-estimation-algorithms),
instead, are, in fact, intrinsically both positive definite and Hermitian.
However, for [performance reasons](#performance-considerations), the matrix equation is augmented to achieve a
consistent structure across the entire topology using Lagrange multipliers.
This augmented structure causes the matrix to no longer be positive definite (but still Hermitian).

It is in principle possible to use different solvers for power flow equations and state estimation equations.
The power grid model, however, opts for one single implementation that can handle both power flow equations and state
estimation equations.
This reduces the overall complexity of the code base.

#### Element size properties of power system equations

Power system equations may contain elements of several orders of magnitude.
However, many matrix solvers are inherently unstable under such extreme conditions, as a reslt of non-linearly
propagated numerical errors under such ill-conditioned matrices.
It is therefore essential that the solvers function under extreme conditions by limiting numerical errors and checking
for stability.

### Block-sparsity considerations

The power flow and state estimation equations involve block-sparse matrices containing typically dense blocks.
The dimensionality of such blocks varies between different calculation types, methods and symmetries.
The blocks are typically arranged in extremely sparse fashion.
The sparse structure can be pre-calculated, but the dense blocks need to be inverted separately.
To make matters worse, the dense blocks may differ heavily in structure and contents between different nodes, and are
often not solvable without pivoting.

### Custom sparse LU solver

The choice for a custom LU solver implementation comes from a number of considerations:

* [LU decomposition](https://en.wikipedia.org/wiki/LU_decomposition) is the best choice, because
  [QR decomposition](https://en.wikipedia.org/wiki/QR_decomposition) and
  [Cholesky decomposition](https://en.wikipedia.org/wiki/Cholesky_decomposition) cannot solve the power system equations
  efficiently as a consequence of the [power system equations' properties](#power-system-equations-properties), as the
  QR decomposition is not as efficient when dealing with sparse matrices, while the latter requires Hermitian and/or
  semi-definite (in generalized form).
* Alternative LU solver implementations are optimized for a variety of use cases that are less sparse than the ones
  encountered in power systems.
* The power grid model requires a faster and more dedicated block-sparse matrix equation solver than what alternative LU
  solver implementations provide.

## Implementation

The LU solver implemented in the power grid model consists of 3 components:

* A block-sparse LU solver that:
  * handles factorization using the topological structure down to block-level
  * solves the sparse matrix equation given the factorization
* A dense LU factor that handles individual blocks within the matrix equation

### Dense LU factorization

The power grid model uses a modified version of the
[`LUFullPiv` defined in Eigen](https://gitlab.com/libeigen/eigen/-/blob/3.4/Eigen/src/LU/FullPivLU.h)
(credits go to the original author).
The modification adds opt-in support for [pivot perturbation](#pivot-perturbation).

#### Dense LU factorization process

The [Gaussian elimination](https://en.wikipedia.org/wiki/Gaussian_elimination) process is identical and iterating over
all pivot elements $p$.
Let the full matrix be as follows.

$$
\mathbf{M} = \begin{bmatrix}
m_{0,0} && m_{0,1} && \cdots && m_{0,N-1} \\
m_{1,0} && m_{1,1} && \cdots && m_{1,N-1} \\
\vdots && \vdots && \ddots && \vdots \\
m_{N-1,0} && m_{N-1,1} && \cdots && m_{N-1,N-1}
\end{bmatrix}
$$

While processing current pivot element $p$, the partially processed matrix has the following arrangement.

$$
\mathbf{M}_{\underline{p}}\equiv\begin{bmatrix}
&& \mathbf{U}_{\underline{p}} && \\
\mathbf{L}_{\underline{p}} && \begin{array}{|c}\hline
  \mathbf{M}_p \end{array}
\end{bmatrix}\equiv\begin{bmatrix}
&& \mathbf{U}_{\underline{p}} && \\
\mathbf{L}_{\underline{p}} && \begin{array}{|cc}\hline
  m_p && \boldsymbol{r}_p^T \\
 \boldsymbol{q}_p && \hat{\mathbf{M}}_p \end{array}
\end{bmatrix}
$$

Here, $\underline{p}$ denotes the fact that it is partially processed.
Furthermore, $\mathbf{M}_{\underline{p}}$ is the partially processed matrix up to pivot $p$;
$\mathbf{U}_{\underline{p}}$ and $\mathbf{L}_{\underline{p}}$ are the upper and lower part of the matrix up to pivot
$p$;
$\mathbf{M}_p\equiv\begin{bmatrix} m_p && \boldsymbol{r}_p^T \\ \boldsymbol{q}_p && \hat{\mathbf{M}}_p\end{bmatrix}$
is the remaining $\left(N-p\right)\times\left(N-p\right)$ part of the matrix that is yet to be LU factorized.
In addition, $m_p$ is the pivot element value, $\boldsymbol{q}$ and $\boldsymbol{r}_p^T$ are the associated column and
row vectors containing the rest of the pivot column and row; and $\hat{\mathbf{M}}_p$ is the remaining bottom-right
block of the matrix.
Plugging all definitions into the full matrix yields the following.

$$
\begin{bmatrix}
    m_0 && \boldsymbol{u}_0^T && && \\
    \boldsymbol{l}_0 && \ddots && \ddots && \\
    && \ddots && \ddots && \boldsymbol{u}_{p-1}^T \\
    && && \boldsymbol{l}_{p-1} && \mathbf{M}_p
\end{bmatrix}\equiv\begin{bmatrix}
    m_0 && \boldsymbol{u}_0^T && && && \\
    \boldsymbol{l}_0 && \ddots && \ddots && && \\
    && \ddots && m_{p-1} && \boldsymbol{u}_{p-1}^T && \\
    && && \boldsymbol{l}_{p-1} && m_p && \boldsymbol{r}_p^T \\
    && && && \boldsymbol{q}_p && \hat{\mathbf{M}}_p\end{bmatrix}
$$

In this equation, $\boldsymbol{l}_k$ and $\boldsymbol{u}_k$ are column and row vectors, respectively; $m_k$,
$\boldsymbol{l}_k$ and $\boldsymbol{u}_k$ ($k < p$) denote sections of the matrix that have already been processed.

[Gaussian elimination](https://en.wikipedia.org/wiki/Gaussian_elimination) using
[LU decomposition](https://en.wikipedia.org/wiki/LU_decomposition) constructs the matrices

$$
\begin{align*}
\mathbf{L}_p &= \begin{bmatrix} 1 && \boldsymbol{0}^T \\ m_p^{-1} \boldsymbol{q}_p && \mathbf{1}_p\end{bmatrix} \\
\mathbf{U}_p &= \begin{bmatrix} m_p && \boldsymbol{r}_p^T \\\boldsymbol{0} && \mathbf{1}_p\end{bmatrix} \\
\mathbf{M}_{p+1} &= \hat{\mathbf{M}}_p - m_p^{-1} \boldsymbol{q}_p \boldsymbol{r}_p^T
\end{align*}
$$

where $\mathbf{1}$ is the matrix with ones on the diagonal and zeros off-diagonal, and $\mathbf{M}_{p+1}$ is the start
of the next iteration.
$\mathbf{L}_p$, $\mathbf{U}_p$, $\mathbf{M}_{p+1}$ and $\mathbf{M}_{p}$ are related as follows.

$$
\mathbf{M}_p \equiv
\begin{bmatrix} 1 && \boldsymbol{0}^T \\ m_p^{-1} \boldsymbol{q}_p && \mathbf{1}_p\end{bmatrix}\begin{bmatrix}
    1 && \boldsymbol{0}^T \\
    \boldsymbol{0} && \hat{\mathbf{M}}_p - m_p^{-1} \boldsymbol{q}_p \boldsymbol{r}_p^T
\end{bmatrix}\begin{bmatrix} m_p && \boldsymbol{r}_p^T \\\boldsymbol{0} && \mathbf{1}_p\end{bmatrix}
\equiv \mathbf{L}_p \begin{bmatrix}
    1 && \boldsymbol{0}^T \\
    \boldsymbol{0} && \mathbf{M}_{p+1}
\end{bmatrix} \mathbf{U}_p
$$

Expanding one more step, this yields the following.

$$
\mathbf{M}_{p-1} \equiv \mathbf{L}_{p-1} \begin{bmatrix}
    1 && \boldsymbol{0}^T \\
    \boldsymbol{0} && \mathbf{L}_p
\end{bmatrix}\begin{bmatrix}
    1 && 0 && \boldsymbol{0}^T \\
    0 && 1 && \boldsymbol{0}^T \\
    \boldsymbol{0} && \boldsymbol{0} && \mathbf{M}_{p+1}
\end{bmatrix} \begin{bmatrix}
    1 && \boldsymbol{0}^T \\
    \boldsymbol{0} && \mathbf{U}_p
\end{bmatrix}\mathbf{U}_{p-1}
$$

Here, $\mathbf{L}_{p-1}$ and $\mathbf{U}_{p-1}$ have dimensions $\left(N-p+1\right)\times \left(N-p+1\right)$,
$\mathbf{L}_p$ and $\mathbf{U}_p$ have dimensions $\left(N-p\right) \times \left(N-p\right)$, and $\mathbf{M}_{p+1}$ has
dimensions $\left(N-p-1\right) \times \left(N-p-1\right)$.

Iterating this process yields the matrices

$$
\begin{align*}
\mathbf{L} = \begin{bmatrix}
1 && 0 && \cdots && 0 \\
\left(\boldsymbol{l}_0\right)_0 && \ddots && \ddots && \vdots \\
\vdots && \ddots && 1 && 0 \\
\left(\boldsymbol{l}_0\right)_{N-p-2} && \cdots && \left(\boldsymbol{l}_{N-2}\right)_0 && 1
\end{bmatrix} &= \begin{bmatrix}
1 && 0 && \cdots && 0 \\
m_0^{-1} \left(\boldsymbol{q}_0\right)_0 && \ddots && \ddots && \vdots \\
\vdots && \ddots && 1 && 0 \\
m_0^{-1} \left(\boldsymbol{q}_0\right)_{N-p-2} && \cdots && m_{N-2}^{-1} \left(\boldsymbol{q}_{N-2}\right)_0 && 1
\end{bmatrix} \\
\mathbf{U} = \begin{bmatrix}
m_0 && \left(\boldsymbol{u}_0^T\right)_0 && \cdots && \left(\boldsymbol{u}_0^T\right)_{N-p-2} \\
0 && \ddots && \ddots && \vdots \\
\vdots && \ddots && m_{N-2} && \left(\boldsymbol{u}_{N-2}^T\right)_0 \\
0 && \cdots && 0 && m_{N-1}
\end{bmatrix} &= \begin{bmatrix}
m_0 && \left(\boldsymbol{r}_0^T\right)_0 && \cdots && \left(\boldsymbol{r}_0^T\right)_{N-p-2} \\
0 && \ddots && \ddots && \vdots \\
\vdots && \ddots && m_{N-2} && \left(\boldsymbol{r}_{N-2}^T\right)_0 \\
0 && \cdots && 0 && m_{N-1}
\end{bmatrix}
\end{align*}
$$

in which $\boldsymbol{l}_p$ is the first column of the lower triangle of $\mathbf{L}_p$ and $\boldsymbol{u}_p^T$ is the
first row of the upper triangle of $\mathbf{U}_p$.

The process described in the above assumes no pivot permutations were necessary.
If permutations are required, they are kept track of in separate row-permution and column-permutation matrices
$\mathbf{P}$ and $\mathbf{Q}$, such that $\mathbf{P}\mathbf{M}\mathbf{Q} = \mathbf{L}\mathbf{U}$, which can be rewritten
as

$$
\mathbf{M} = \mathbf{P}^{-1}\mathbf{L}\mathbf{U}\mathbf{Q}^{-1}
$$

#### Dense LU factorization algorithm

The power grid model uses an in-place LU decomposition approach.
Permutations are separately stored.
Below is the algorithm of LU decomposition with pivot perturbation ([see below](#pivot-perturbation)) used in the power
grid model.

Let $\mathbf{M}$ be the $N\times N$-matrix and $\mathbf{M}\left[i,j\right]$ its element at (0-based) indices $(i,j)$,
where $i,j = 0..(N-1)$.
For readbility, we use $:$ to denote a range slicing operation to along a dimension of matrix $\mathbf{M}$, e.g.
$\mathbf{M}\left[0:3, j\right]$.

1. Initialize the permutations $\mathbf{P}$ and $\mathbf{Q}$ to the identity permutation.
2. Initialize fill-in elements to $0$.
3. Loop over all rows: $p = 0..(N-1)$:
   1. Set the remaining matrix: $\mathbf{M}_p \gets \mathbf{M}\left[p:N,p:N\right]$ with size $N_p\times N_p$, where
      $N_p := N - p$.
   2. Find largest element $\mathbf{M}_p\left[i_p,j_p\right]$ in $\mathbf{M}_p$ by magnitude.
      This is the pivot element.
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
   5. Swap the first and pivot row and column of $\mathbf{M}_p$, so that the pivot element is in the top-left corner of
      the remaining matrix:
      1. $\mathbf{M}_p\left[0,0:N_p\right] \leftrightarrow \mathbf{M}\left[i_p,0:N_p\right]$
      2. $\mathbf{M}_p\left[0:N_p,0\right] \leftrightarrow \mathbf{M}\left[0:N_p,j_p\right]$
   6. Apply Gaussian elimination for the current pivot element:
      1. $\mathbf{M}_p\left[0,0:N_p\right] \gets \frac{1}{\mathbf{M}_p[0,0]}\mathbf{M}_p\left[0,0:N_p\right]$
      2. $\mathbf{M}_p\left[1:N_p,0:N_p\right] \gets \mathbf{M}_p\left[1:N_p,0:N_p\right] - \mathbf{M}_p\left[1:N_p,0\right] \otimes \mathbf{M}_p\left[0,0:N_p\right]$  <!-- markdownlint-disable-line line-length -->
   7. Accumulate the permutation matrices:
      1. In $\mathbf{P}$: swap $p \leftrightarrow p + i_p$
      2. In $\mathbf{Q}$: swap $p \leftrightarrow p + j_p$
   8. Continue with the next $p$ to factorize the the bottom-right block.

$\mathbf{L}$ is now the matrix containing the lower triangle of $\mathbf{M}$, ones on the diagonal and zeros in the
upper triangle.
Similarly, $\mathbf{U}$ is the matrix containing the upper triangle of $\mathbf{M}$ including the diagonal elements and
zeros in the lower triangle.

```{note}
In the (equivalent) actual implementation, we break from the loop and throw afterwards, instead of raising the
`SparseMatrixError` immediately.
```

```{note}
Permutations are only allowed within each dense block, for the reasons described [earlier](#pivot-operations).
```

### Block-sparse LU factorization

The LU factorization process for block-sparse matrices is similar to that for [dense matrices](#dense-lu-factorization).
Only in this case, $m_p$ refers to a block element.
$\boldsymbol{q}_p$, $\boldsymbol{r}_p^T$ and $\hat{\mathbf{M}}_p$ consist of block elements as well.
Notice that the block-wise inverse $m_p^{-1}$ can be calculated using its LU decomposition, which can be obtained from
the [dense LU factorization process](#dense-lu-factorization-process).

#### Block-sparse LU factorization process

The block-sparse LU factorization process differs slightly from the
[dense counterpart](#dense-lu-factorization-process).
Below, we first [describe the naive approach](#naive-block-sparse-lu-factorization-process), followed by the rationale
and proof of the alternative approach.

##### Naive block-sparse LU factorization process

A naïve approach towards block-sparse LU factorization is done as follows.
The [dense LU factorization process](#dense-lu-factorization-process) is followed but on blocks instead of single
elements.
Let
$\mathbf{M}_p\equiv\begin{bmatrix}\mathbf{m}_p && \mathbf{r}_p^T \\ \mathbf{q}_p && \hat{\mathbf{M}}_p\end{bmatrix}$ be
the block-sparse sub-matrix yet to be decomposed.
$\mathbf{m}_p$ is a dense block that can be [LU factorized](#dense-lu-factorization-process):
$\mathbf{m}_p = \mathbf{p}_p^{-1} \mathbf{l}_p \mathbf{u}_p \mathbf{q}_p^{-1}$, where the lower-case helps avoiding
confusion with the block-sparse matrix components.
The matrices constructed with [LU decomposition](https://en.wikipedia.org/wiki/LU_decomposition) for
[Gaussian elimination](https://en.wikipedia.org/wiki/Gaussian_elimination) are constructed accordingly.

$$
\begin{align*}
\mathbf{L}_p &= \begin{bmatrix}
    \mathbf{1} && \mathbf{0}^T \\
    \overrightarrow{\mathbf{m}_p^{-1}\mathbf{q}_p} && \mathbf{1}_p
\end{bmatrix} \\
\mathbf{U}_p &= \begin{bmatrix}
    \mathbf{m}_p && \mathbf{r}_p^T \\
    \mathbf{0} && \mathbf{1}_p
\end{bmatrix} \\
\mathbf{M}_{p+1} &= \hat{\mathbf{M}}_p - \widehat{\mathbf{m}_p^{-1}\mathbf{q}_p \mathbf{r}_p^T}
\end{align*}
$$

Here, $\overrightarrow{\mathbf{m}_p^{-1}\mathbf{q}_p}$ is symbolic notation for the block-vector of solutions to the
equation $\mathbf{m}_p x_{p;k} = \mathbf{q}_{p;k}$, where $k = 0..(p-1)$.
Similarly, $\widehat{\mathbf{m}_p^{-1}\mathbf{q}_p \mathbf{r}_p^T}$ is symbolic notation for the block-matrix of
solutions to the equation $\mathbf{m}_p x_{p;k,l} = \mathbf{q}_{p;k} \mathbf{r}_{p;l}^T$, where $k,l = 0..(p-1)$.
That is:

$$
\begin{align*}
\overrightarrow{\mathbf{m}_p^{-1}\mathbf{q}_p}
&= \begin{bmatrix}\mathbf{m}_p^{-1}\mathbf{q}_{p;0} \\
\vdots \\
\mathbf{m}_p^{-1} \mathbf{q}_{p;N-1} \end{bmatrix} \\
\widehat{\mathbf{m}_p^{-1}\mathbf{q}_p \mathbf{r}_p^T}
&= \begin{bmatrix}\mathbf{m}_p^{-1} \mathbf{q}_{p;0} \mathbf{r}_{p;0}^T && \cdots &&
\mathbf{m}_p^{-1} \mathbf{q}_{p;0} \mathbf{r}_{p;N-1}^T \\
\vdots && \ddots && \vdots \\
\mathbf{m}_p^{-1} \mathbf{q}_{p;0} \mathbf{r}_{p;0}^T && \cdots &&
\mathbf{m}_p^{-1} \mathbf{q}_{p;N-1} \mathbf{r}_{p;N-1}^T \end{bmatrix}
\end{align*}
$$

Iteratively applying above factorization process yields $\mathbf{L}$ and $\mathbf{U}$, as well as $\mathbf{P}$ and
$\mathbf{Q}$.

Unfortunately, the process of obtaining $\mathbf{m}_p^{-1} \boldsymbol{x}_p$ can be numerically unstable, even if it is
split into multiple solving steps.
Instead, the process below is used.

##### Partially solved block-sparse LU factorization process

A more numerically stable approach compared to the [naïve approach](#naive-block-sparse-lu-factorization-process) is
equivalent to single-element pivot.
Instead of applying the full pivot decomposition in the $L$-matrix and delaying the solve process until the final
solving phase, we only apply a partial pivoting step to both $L$ matrix and the $U$ matrix.
This is equivalent to single-element pivoting without block-pivoting.
In the following sections, we provide the [rationale](#rationale-of-the-block-sparse-lu-factorization-process) and
[proof](#proof-of-block-sparse-lu-factorization-process).

Following the same conventions as [above](#naive-block-sparse-lu-factorization-process), let
$\mathbf{M}_p\equiv\begin{bmatrix}\mathbf{m}_p && \mathbf{r}_p^T \\ \mathbf{q}_p && \hat{\mathbf{M}}_p\end{bmatrix}$ be
the block-sparse matrix to decompose.
$\mathbf{m}_p$ is a dense block that can be [LU factorized](#dense-lu-factorization-process):
$\mathbf{m}_p = \mathbf{p}_p^{-1} \mathbf{l}_p \mathbf{u}_p \mathbf{q}_p^{-1}$.
Partial Gaussian elimination constructs the following matrices.

$$
\begin{align*}
\mathbf{L}_p &= \begin{bmatrix}
    \mathbf{l}_p && \mathbf{0}^T \\
    \overrightarrow{\mathbf{q}_p\mathbf{q}_p\mathbf{u}_p^{-1}} && \mathbf{1}_p
\end{bmatrix} \\
\mathbf{U}_p &= \begin{bmatrix}
    \mathbf{u}_p && \overrightarrow{\mathbf{l}_p\mathbf{p}_p\mathbf{r}_p}^T \\
    \mathbf{0} && \mathbf{1}_p
\end{bmatrix} \\
\mathbf{M}_{p+1} &=
    \hat{\mathbf{M}}_p - \widehat{\mathbf{q}_p\mathbf{q}_p\mathbf{u}_p^{-1}\mathbf{l}_p^{-1}\mathbf{p}_p\mathbf{r}_p^T}
\end{align*}
$$

Note that the first column of $\mathbf{L}_p$ can be obtained by applying a right-solve procedure, instead of the regular
left-solve procedure, as is the case for $\mathbf{U}_p$.

##### Rationale of the block-sparse LU factorization process

To illustrate the rationale, let's fully solve a matrix equation (without using blocks):

$$
\begin{align*}
\begin{bmatrix}
\mathbf{a} && \mathbf{b} \\
\mathbf{c} && \mathbf{d}
\end{bmatrix}
&=
\begin{bmatrix}
a_{11} && a_{12} && b_{11} && b_{12} \\
a_{21} && a_{22} && b_{21} && b_{22} \\
c_{11} && c_{12} && d_{11} && d_{12} \\
c_{21} && c_{22} && d_{21} && d_{22}
\end{bmatrix} \\
& \mapsto
\begin{bmatrix}
a_{11} && a_{12} && b_{11} && b_{12} \\
\frac{a_{21}}{a_{11}} &&
    a_{22} - a_{12} \frac{a_{21}}{a_{11}} &&
    b_{21} - b_{11}\frac{a_{21}}{a_{11}} &&
    b_{22} - b_{12}\frac{a_{21}}{a_{11}} \\
\frac{c_{11}}{a_{11}} &&
    c_{12} - a_{12} \frac{c_{11}}{a_{11}} &&
    d_{11} - b_{11}\frac{c_{11}}{a_{11}} &&
    d_{12} - b_{12}\frac{c_{11}}{a_{11}} \\
\frac{c_{21}}{a_{11}} &&
    c_{22} - a_{12} \frac{c_{21}}{a_{11}} &&
    d_{21} - b_{11}\frac{c_{21}}{a_{11}} &&
    d_{22} - b_{12}\frac{c_{21}}{a_{11}}
\end{bmatrix} \\
& \mapsto
\begin{bmatrix}
a_{11} && a_{12} && b_{11} && b_{12} \\
\frac{a_{21}}{a_{11}} &&
    a_{22} - a_{12} \frac{a_{21}}{a_{11}} &&
    b_{21} - b_{11}\frac{a_{21}}{a_{11}} &&
    b_{22} - b_{12}\frac{a_{21}}{a_{11}} \\
\frac{c_{11}}{a_{11}} &&
    \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{11} - b_{11}\frac{c_{11}}{a_{11}}
        - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{12} - b_{12}\frac{c_{11}}{a_{11}}
        - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}  \\
\frac{c_{21}}{a_{11}} &&
    \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{21} - b_{11}\frac{c_{21}}{a_{11}}
        - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{22} - b_{12}\frac{c_{21}}{a_{11}}
        - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{bmatrix}
\end{align*}
$$

Using the following denotations, we can simplify the above as
$\begin{bmatrix} \mathbf{l}_a \mathbf{u}_a && \mathbf{u}_b \\ \mathbf{l}_c && \mathbf{l}_d \mathbf{u}_d \end{bmatrix}$.

$$
\begin{align*}
\mathbf{l}_a &= \begin{bmatrix}
    1                     && 0 \\
    \frac{a_{21}}{a_{11}} && 1
\end{bmatrix} \\
\mathbf{u}_a &= \begin{bmatrix}
    a_{11} && a_{12} \\
    0 && a_{22} - a_{12} \frac{a_{21}}{a_{11}}
\end{bmatrix} \\
\mathbf{l}_c &= \begin{bmatrix}
    \frac{c_{11}}{a_{11}} && \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} \\
    \frac{c_{21}}{a_{11}} && \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{bmatrix} \\
\mathbf{u}_b &= \begin{bmatrix}
    b_{11}                               && b_{12} \\
    b_{21} - b_{11}\frac{a_{21}}{a_{11}} && b_{22} - b_{12}\frac{a_{21}}{a_{11}}
\end{bmatrix} \\
\mathbf{l}_d\mathbf{u}_d &= \begin{bmatrix}
    d_{11} - b_{11}\frac{c_{11}}{a_{11}}
        - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{12} - b_{12}\frac{c_{11}}{a_{11}}
        - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}  \\
    d_{21} - b_{11}\frac{c_{21}}{a_{11}}
        - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
     d_{22} - b_{12}\frac{c_{21}}{a_{11}}
         - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right)
             \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{bmatrix}
\end{align*}
$$

Interestingly, the matrices $\mathbf{l}_c$, $\mathbf{u}_b$ and $\mathbf{l}_d\mathbf{u}_d$ can be obtained without doing
full pivoting on the sub-block level:

* $\mathbf{l}_c$ is the solution to the right-multiplication matrix equation $\mathbf{l}_c \mathbf{u}_a = \mathbf{c}$
* $\mathbf{u}_b$ is the solution to the left-multiplication matrix equation $\mathbf{l}_a \mathbf{u}_b = \mathbf{b}$.
* $\mathbf{l}_d\mathbf{u}_d$ denotes the start matrix of the decomposition of the next iteration and is equal to
  $\mathbf{l}_d\mathbf{u}_d = \mathbf{d} - \mathbf{l}_c \mathbf{u}_b$.

This process generalizes to block-invertible matrices of any size: $\mathbf{c}$ and $\mathbf{b}$ become a column- and
row-vector of block-matrices for which the individual block-elements of the vectorized decompositions $\mathbf{l}_c$ and
$\mathbf{u}_b$ can be obtained by solving the equations above.

If, during the LU decomposition of the pivot block, a row- and column-permutation was used,
$\mathbf{a} = \mathbf{p}_a^{-1} \mathbf{l}_a \mathbf{u}_p \mathbf{q}_a^{-1}$, and the columns of $\mathbf{c}$ and the
rows of $\mathbf{b}$ are permuted: $\mathbf{c}\mapsto\mathbf{c}\mathbf{q}_a$ and
$\mathbf{b}\mapsto\mathbf{p}_a\mathbf{b}$.
In this case, the equations generalize as follows.

* $\mathbf{l}_c$ is the solution to the right-multiplication matrix equation
  $\mathbf{l}_c \mathbf{u}_a = \mathbf{c} \mathbf{q}_a$.
* $\mathbf{u}_b$ is the solution to the left-multiplication matrix equation
  $\mathbf{l}_a \mathbf{u}_b = \mathbf{p}_a \mathbf{b}$.
* $\mathbf{l}_d\mathbf{u}_d$ denotes the start matrix of the decomposition of the next iteration and is equal to
  $\mathbf{l}_d\mathbf{u}_d = \mathbf{d} - \mathbf{l}_c \mathbf{u}_b$.

##### Proof of block-sparse LU factorization process

The following proves the equations $\mathbf{l}_c \mathbf{u}_a = \mathbf{c}$, $\mathbf{l}_a \mathbf{u}_b = \mathbf{b}$
and $\mathbf{l}_d\mathbf{u}_d = \mathbf{d} - \mathbf{l}_c \mathbf{u}_b$ mentioned in the
[previous section](#rationale-of-the-block-sparse-lu-factorization-process).

$$
\begin{align*}
\mathbf{l}_c \mathbf{u}_a
&=
\begin{bmatrix}
    \frac{c_{11}}{a_{11}} && \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} \\
    \frac{c_{21}}{a_{11}} && \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{bmatrix} \begin{bmatrix}
    a_{11} && a_{12} \\
    0 && a_{22} - a_{12} \frac{a_{21}}{a_{11}}
\end{bmatrix} \\
&= \begin{bmatrix}
    a_{11} \frac{c_{11}}{a_{11}} &&
    a_{12} \frac{c_{11}}{a_{11}}
        + \left(a_{22} - a_{12} \frac{a_{21}}{a_{11}}\right)
            \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} \\
    a_{11} \frac{c_{21}}{a_{11}} &&
    a_{12} \frac{c_{21}}{a_{11}}
        + \left(a_{22} - a_{12} \frac{a_{21}}{a_{11}}\right)
            \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{bmatrix} \\
&= \begin{bmatrix}
    c_{11} && c_{11} + \left(c_{12} - c_{11}\right) \\
    c_{21} && c_{21} + \left(c_{22} - c_{21}\right)
\end{bmatrix} \\
&= \begin{bmatrix}
    c_{11} && c_{12} \\
    c_{21} && c_{22}
\end{bmatrix} \\
&= \mathbf{c} \\
\mathbf{l}_a \mathbf{u}_b
&= \begin{bmatrix}
    1                     && 0 \\
    \frac{a_{21}}{a_{11}} && 1
\end{bmatrix} \begin{bmatrix}
    b_{11}                               && b_{12} \\
    b_{21} - b_{11}\frac{a_{21}}{a_{11}} && b_{22} - b_{12}\frac{a_{21}}{a_{11}}
\end{bmatrix} \\
&= \begin{bmatrix}
    b_{11} &&
    b_{12} \\
    b_{11} \frac{a_{21}}{a_{11}} + b_{21} - b_{11} \frac{a_{21}}{a_{11}} &&
    b_{12} \frac{a_{21}}{a_{11}} + b_{22} - b_{12}\frac{a_{21}}{a_{11}}
\end{bmatrix} \\
&= \begin{bmatrix}
    b_{11} && b_{12} \\
    b_{21} && b_{22}
\end{bmatrix} \\
&= \mathbf{b} \\
\mathbf{l}_d\mathbf{u}_d
&= \mathbf{d} - \mathbf{l}_c \mathbf{u}_b \\
&= \begin{bmatrix}d_{11} && d_{12} \\ d_{21} && d_{22} \end{bmatrix} - \begin{bmatrix}
    \frac{c_{11}}{a_{11}} && \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} \\
    \frac{c_{21}}{a_{11}} && \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{bmatrix} \begin{bmatrix}
    b_{11}                               && b_{12} \\
    b_{21} - b_{11}\frac{a_{21}}{a_{11}} && b_{22} - b_{12}\frac{a_{21}}{a_{11}}
\end{bmatrix} \\
&= \begin{bmatrix}
    d_{11} - b_{11}\frac{c_{11}}{a_{11}}
        - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{12} - b_{12}\frac{c_{11}}{a_{11}}
        - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} \\
    d{21} - b_{11}\frac{c_{21}}{a_{11}}
        - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{22} - b_{12}\frac{c_{21}}{a_{11}}
        - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right)
            \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{bmatrix} \\
&= \mathbf{l}_d\mathbf{u}_d
\end{align*}
$$

We can see that $\mathbf{l}_c$ and $\mathbf{u}_b$ are affected by the in-block LU decomposition of the pivot block
$\left(\mathbf{l}_a,\mathbf{u}_a\right)$, as well as the data in the respective blocks ($\mathbf{c}$ and $\mathbf{b}$)
in the original matrix.
Beyond these two sources, no other factors affect $\mathbf{l}_c$ and $\mathbf{u}_b$.
The generalization to largr block sizes and block-matrix sizes follows.

#### Block-sparse indexing

The structure of the block-sparse matrices is as follows.

* The $N\times N$ block matrix $\mathbf{M}$ is interpreted as the block-matrix, with $\mathbf{M}\left[i,j\right]$ its
  block element at (0-based) indices $(i,j)$, where $i,j = 0..(N-1)$.
* In turn, let $\mathbf{M}\left[i,j\right]\equiv\mathbf{M}_{i,j}$ be the dense block with dimensions $N_i\times N_j$.

This can be graphically represented as

$$
\begin{align*}
\mathbf{M} &\equiv \begin{bmatrix}
\mathbf{M}_{0,0}   && \cdots && \mathbf{M}_{0,N-1} \\
\vdots    && \ddots && \vdots \\
\mathbf{M}_{N-1,0} && \cdots && \mathbf{M}_{N-1,N-1}
\end{bmatrix} \\
&\equiv \begin{bmatrix}
\begin{bmatrix}
   \mathbf{M}_{0,0}\left[0,0\right]     && \cdots && \mathbf{M}_{0,0}\left[0,N_j-1\right] \\
   \vdots                      && \ddots && \vdots \\
   \mathbf{M}_{0,0}\left[N_i-1,0\right] && \cdots && \mathbf{M}_{0,0}\left[N_i-1,N_j-1\right]
\end{bmatrix} && \cdots && \begin{bmatrix}
   \mathbf{M}_{0,N-1}\left[0,0\right]     && \cdots && \mathbf{M}_{0,N-1}\left[0,N_j-1\right] \\
   \vdots                        && \ddots && \vdots \\
   \mathbf{M}_{0,N-1}\left[N_i-1,0\right] && \cdots && \mathbf{M}_{0,N-1}\left[N_i-1,N_j-1\right] \end{bmatrix} \\
\vdots && \ddots && \vdots \\
\begin{bmatrix}
   \mathbf{M}_{N-1,0}\left[0,0\right]     && \cdots && \mathbf{M}_{N-1,0}\left[0,N_j-1\right] \\
   \vdots                        && \ddots && \vdots \\
   \mathbf{M}_{N-1,0}\left[N_i-1,0\right] && \cdots && \mathbf{M}_{N-1,0}\left[N_i-1,N_j-1\right]
\end{bmatrix} && \cdots && \begin{bmatrix}
   \mathbf{M}_{N-1,N-1}\left[0,0\right]     && \cdots && \mathbf{M}_{N-1,N-1}\left[0,N_j-1\right] \\
   \vdots                          && \ddots && \vdots \\
   \mathbf{M}_{N-1,N-1}\left[N_i-1,0\right] && \cdots && \mathbf{M}_{N-1,N-1}\left[N_i-1,N_j-1\right] \end{bmatrix}
\end{bmatrix}
\end{align*}
$$

Because of the sparse structure and the fact that all $\mathbf{M}_{i,j}$ have the same shape, it is much more efficient
to store the blocks $\mathbf{M}_{i,j}$ in a vector $\mathbf{M}_{\tilde{k}}$ where $\tilde{k}$ is a reordered index from
$(i,j) \mapsto \tilde{k}$.
This mapping, in turn, is stored as an index pointer, i.e., a vector of vectors of indices, with the outer index given
by the row-index $i$, and the inner vector containing the values of $j$ for which $\mathbf{M}_{i,j}$ may be non-zero.
All topologically relevant matrix elements, as well as [fill-ins](#pivot-operations), are included in this mapping.
The following illustrates this mapping.

$$
\begin{align*}
\begin{bmatrix}
\mathbf{M}_{0,0} &&         &&         && \mathbf{M}_{0,3} \\
        && \mathbf{M}_{1,1} && \mathbf{M}_{1,2} &&         \\
        && \mathbf{M}_{2,1} && \mathbf{M}_{2,2} && \mathbf{M}_{2,3} \\
\mathbf{M}_{3,0} &&         && \mathbf{M}_{3,2} && \mathbf{M}_{3,3}
\end{bmatrix} &\equiv
\begin{bmatrix}
    \mathbf{M}_{0,0} &&
        \mathbf{M}_{0,3} &&
        \mathbf{M}_{1,1} &&
        \mathbf{M}_{1,2} &&
        \mathbf{M}_{2,1} &&
        \mathbf{M}_{2,2} &&
        \mathbf{M}_{2,3} &&
        \mathbf{M}_{3,0} &&
        \mathbf{M}_{3,2} &&
        \mathbf{M}_{3,3} \\
    [[0 && 3] && [1 && 2] && [1 && 2 && 3] && [0 && 2 && 3]] \\
\end{bmatrix} \\
&\equiv
\begin{bmatrix}
    \mathbf{M}_{0,0} &&
        \mathbf{M}_{0,3} &&
        \mathbf{M}_{1,1} &&
        \mathbf{M}_{1,2} &&
        \mathbf{M}_{2,1} &&
        \mathbf{M}_{2,2} &&
        \mathbf{M}_{2,3} &&
        \mathbf{M}_{3,0} &&
        \mathbf{M}_{3,2} &&
        \mathbf{M}_{3,3} &&
        \\
    [0 && 3 && 1 && 2 && 1 && 2 && 3 && 0 && 2 && 3] && \\
    [0 && && 2 && && 4 && && && 7 && && && && 10]
\end{bmatrix}
\end{align*}
$$

In the first equation, the upper row contains the present block entries and the bottom row their column indices per row
to obtain a flattened representation of the matrix.
In the last equivalence, the column indices, in turn, are also flattened into a separate flattened representation of the
column indices and the start indices of each row, i.e., the index pointer (`indptr`).
Note that the size of `indptr` is 1 greater than the amount of rows.
This enables describing the amount of elements in a given row $i$ as the difference between the respective element and
the next, i.e., $\text{indptr}[i + 1] - \text{indptr}[i]$.

Looping over the rows and columns becomes trivial.
Let $\text{indptr}$ be the start.
The double loop becomes:

1. Loop all rows: $i=0..(N-1)$:
   1. The column indices for this row start at $\text{indptr}\left[i\right]$.
   2. The column indices for this row stop at $\text{indptr}\left[i+1\right]$.
   3. Loop all columns:
      $j=\left(\text{indptr}\left[i\right]\right)..\left(\text{indptr}\left[i+1\right]\right)$:
      1. The current row index is $i$.
      2. The current column index is $j$.

### Block-sparse LU solving

An equation $\mathbf{M} \boldsymbol{x} = \boldsymbol{b}$ can be solved using
[LU factorization](#block-sparse-lu-factorization) by forward and backward substitutions.
Since the blocks of the matrix are already ordered in a way that minimizes the amount of [fill-ins](#pivot-operations),
it is recommended not to permute across blocks, as that would defeat the whole purpose of doing the reordering.
Permutations are therefore restricted to intra-block operations.
The first step of the block-sparse LU solving therefore permutes the rows within each block:
$\boldsymbol{b}^{\prime} = \mathbf{P}\boldsymbol{b}$.
After that, the forward substitution step essentially amounts to solving the matrix equation
$\mathbf{L}\boldsymbol{y} = \boldsymbol{b}^{\prime}$, followed by the backwards substitution by solving
$\mathbf{U}\boldsymbol{z} = \mathbf{y}$.
The final result is then obtained by applying the column permutation within each block:
$\boldsymbol{x} = \mathbf{Q}\boldsymbol{z}$.

#### Forward substitution

The row permutation is applied as follows.

1. Loop over all block-rows: $i=0..(N-1)$:
   1. If the matrix is a block matrix:
      1. Apply the current row's block permutation:
         $\boldsymbol{b}\left[i\right] \gets \mathbf{P}\left[i\right] \cdot \boldsymbol{b}\left[i\right]$.
      2. Proceed with the next iteration.
   2. Else:
      1. Proceed with the next iteration.

The equation $\mathbf{L}\boldsymbol{y} = \mathbf{P}\boldsymbol{b}$ is solved as follows.

1. Loop over all block-rows: $i=0..(N-1)$:
   1. Loop over all lower-triangle off-diagonal columns (beware of sparsity): $j=0..(i-1)$:
      1. $\boldsymbol{b}\left[i\right] \gets \boldsymbol{b}\left[i\right] - \mathbf{L}\left[i,j\right] \cdot \boldsymbol{b}\left[j\right]$.  <!-- markdownlint-disable-line line-length -->
      2. Continue with next block-column.
   2. If the matrix is a block matrix:
      1. Follow the same steps within the block.
      2. Proceed with the next iteration.
   3. Else:
      1. Proceed with the next iteration.

#### Backward substitution

The equation $Uz = y$ is solved as follows.

1. Loop over all block-rows in reverse order: $i=(N-1)..0$:
   1. Loop over all upper-triangle off-diagonal columns (beware of sparsity): $j=(i+1)..0$:
      1. $\boldsymbol{b}\left[i\right] \gets \boldsymbol{b}\left[i\right] - \mathbf{U}\left[i,j\right] \cdot \boldsymbol{b}\left[j\right]$.  <!-- markdownlint-disable-line line-length -->
      2. Continue with next block-column.
   2. Handle the diagonal element:
      1. If the matrix is a block matrix:
         1. Follow the same steps within the block.
         2. Proceed.
      2. Else:
         1. $\boldsymbol{b}\left[i\right] \gets \boldsymbol{b}\left[i\right] / \mathbf{U}\left[i,i\right]$.
         2. Proceed.

Apply the column permutation as follows.

1. Loop over all block-rows: $i=0..(N-1)$:
   1. If the matrix is a block matrix:
      1. Apply the current row's block permutation:
         $\boldsymbol{b}\left[i\right] \gets \mathbf{Q}\left[i\right] \cdot \boldsymbol{b}\left[i\right]$.
      2. Proceed.
   2. Else:
      1. Proceed.

```{note}
If [pivot perturbation](#pivot-perturbation) was used to obtain the LU decomposition, the solution obtained here is an
approximation to the exact solution.
The accuracy can be improved using [iterative refinement](#iterative-refinement-of-lu-solver-solutions).
```

### Pivot perturbation

The LU solver implemented in the power grid model features pivot perturbation.
We refer readers to
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
and
[Schenk06](https://etna.math.kent.edu/volumes/2001-2010/vol23/abstract.php?vol=23&pages=158-179)
for more details.
Here, we briefly present the method used in the power grid model LU solver.

Solving matrix equations with forward and backward substitution requires the selection of a pivot element.
If the magnitude of a pivot element is too small compared to that of the other elements in the matrix, numerical
instabilities may be encountered and the pivot element therefore cannot be used as is.
Another pivot element could potentially be selected by permutation with other matrix elements across blocks, but this is
not desirable, as described in the section on [pivot operations](#pivot-operations).
In such cases, the matrix is ill-conditioned.

Instead, a small perturbation can be added to the pivot element.
This renders the matrix equation solvable without selecting a different pivot element.
The cost of such operation is the introduction of numerical errors that are potentially propagating.
Iterative refinement can be used to mitigate the numerical errors.
This method is also used by
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
and
[Schenk06](https://etna.math.kent.edu/volumes/2001-2010/vol23/abstract.php?vol=23&pages=158-179),
as well as the well-known
[MKL Pardiso solver](https://www.intel.com/content/www/us/en/docs/onemkl/developer-reference-c/2023-0/onemkl-pardiso-parallel-direct-sparse-solver-iface.html)

#### Pivot perturbation algorithm

Let $\mathbf{M}$ be the matrix, $\left\|\mathbf{M}\right\|_{\infty ,\text{bwod}}$ the
[block-wise off-diagonal infinite norm](#block-wise-off-diagonal-infinite-matrix-norm) of the matrix.

1. $\epsilon \gets \text{perturbation_threshold} * \left\|\mathbf{M}\right\|_{\text{bwod}}$.
2. If $|\text{pivot_element}| \lt \epsilon$, then:
   1. If $|\text{pivot_element}| = 0$, then:
      1. $\text{phase_shift} \gets 1$.
      2. Proceed.
   2. Else:
      1. $\text{phase_shift} \gets \text{pivot_element} / |\text{pivot_element}|$.
      2. Proceed.
   3. $\text{pivot_element} \gets \epsilon * \text{phase_shift}$.

$\text{phase_shift}$ ensures that the sign (if $\mathbf{M}$ is a real matrix) or complex phase (if $\mathbf{M}$ is a
complex matrix) of the pivot element is preserved.
The positive real axis is used as a fallback when the pivot element is identically zero.

### Iterative refinement of LU solver solutions

The following refinement algorithm draws substantial inspiration from the GESP algorithm described in
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761).

[LU solving](#block-sparse-lu-solving) with an [LU decomposition](#block-sparse-lu-factorization) obtained using
[pivot perturbation](#pivot-perturbation) yields only approximate results, because the LU decomposition itself is only
approximate.
Because of that, an iterative refinement process is required to improve the solution to the matrix equation
$\mathbf{M} \cdot \boldsymbol{x} = \boldsymbol{b}$.

The iterative refinement process works as follows.
In step $i$, given an approximation $\boldsymbol{x}_i$ for $\boldsymbol{x}$, the difference between the current best and
the exact solution is defined as $\boldsymbol{\Delta x} := \boldsymbol{x} - \boldsymbol{x}_i$.
Substituting in the original equation yields
$\mathbf{M} \cdot (\boldsymbol{x}_i + \boldsymbol{\Delta x}) = \boldsymbol{b}$, so that
$\mathbf{M} \cdot \boldsymbol{\Delta x} = \boldsymbol{b} - \mathbf{M} \cdot \boldsymbol{x}_i =: \boldsymbol{r}$.
The residual $\boldsymbol{r}$ can be calculated.
An estimation for the left-hand side can be obtained by using the pivot-perturbed matrix $\tilde{\mathbf{M}}$ instead of
the original matrix $\mathbf{M}$.
Convergence is reached when $\boldsymbol{r} \to \boldsymbol{0}$, which implies
$\left\|\boldsymbol{\Delta x}\right\| \to 0$.
Solving for $\boldsymbol{\Delta x}$ and substituting back into
$\boldsymbol{x}_{i+1} = \boldsymbol{x}_i + \boldsymbol{\Delta x}$ provides the next best approximation
$\boldsymbol{x}_{i+1}$ for $\boldsymbol{x}$.

A measure for the quality of the approximation is given by the $\text{backward_error}$ (see also
[backward error formula](#backward-error-calculation)).

Since the matrix $\mathbf{M}$ remains static during this process, the LU decomposition is valid throughout the process.
The accuracy of the result can therefore be iteratively refined efficiently.

Given the original matrix equation $\mathbf{M} \cdot \boldsymbol{x} = \boldsymbol{b}$ to solve, the pivot perturbated
matrix $\tilde{\mathbf{M}}$ with a pre-calculated LU decomposition, and the convergence threshold $\epsilon$, the
algorithm is as follows:

1. Initialize:
   1. Set the initial estimate: $\boldsymbol{x}_{\text{est}} = \boldsymbol{0}$.
   2. Set the initial residual: $\boldsymbol{r} \gets \boldsymbol{b}$.
   3. Set the initial backward error: $\text{backward_error} = \infty$.
   4. Set the number of iterations to 0.
2. Iteratively refine; loop:
   1. Check stop criteria:
      1. If $\text{backward_error} \leq \epsilon$, then:
         1. Convergence reached: stop the refinement process.
      2. Else, if the number of iterations > maximum allowed amount of iterations, then:
         1. Convergence not reached; iterative refinement not possible: raise a sparse matrix error.
      3. Else:
         1. Increase the number of iterations.
         2. Proceed.
   2. Solve $\tilde{\mathbf{M}} \cdot \boldsymbol{\Delta x} = \boldsymbol{r}$ for $\boldsymbol{\Delta x}$.
   3. Calculate the backward error with the original $\boldsymbol{x}$ and $\boldsymbol{r}$ using the
      [backward error formula](#improved-backward-error-calculation).
   4. Set the next estimation of
      $\boldsymbol{x}$: $\boldsymbol{x}_{\text{est}} \gets \boldsymbol{x}_{\text{est}} + \boldsymbol{\Delta x}$.
   5. Set the residual: $\boldsymbol{r} \gets \boldsymbol{b} - \mathbf{M} \cdot \boldsymbol{x}$.

As a result, we effectively always do:

1. A solving step; and
2. At least one refinement step.

When the maximum allowed amount of iterations is exceeded, a sparse matrix error instead of an iteration diverge error
is raised.
The reason is, that it is specifically the iterative refinement of the matrix equation solution that cannot be solved in
the set amount of iterations - not the set of power system equations.
This will only happen when the matrix equation requires iterative refinement in the first place, which happens only when
pivot perturbation is needed, namely in the case of an ill-conditioned matrix equation.
A matrix equation that is both ill-conditioned and not iteratively refinable is underdetermined and cannot be solved.

#### Backward error calculation

The [iterative refinement algorithm](#iterative-refinement-of-lu-solver-solutions) uses a backward error as a
convergence criterion.
We use the following backward error calculation, inspired by
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761),
with a few modifications described [below](#improved-backward-error-calculation).

$$
\begin{align*}
D_{\text{max}} &= \max_i\left\{
    \left(\left|\mathbf{M}\right|\cdot\left|\boldsymbol{x}\right| + \left|\boldsymbol{b}\right|\right)_i
\right\} \\
\text{backward_error} &= \max_i \left\{
    \frac{\left|\boldsymbol{r}\right|_i}{
        \max\left\{
            \left(\left|\mathbf{M}\right|\cdot\left|\boldsymbol{x}\right| + \left|\boldsymbol{b}\right|\right)_i,
            \epsilon_{\text{backward_error}} D_{\text{max}}
        \right\}
    }
\right\}
\end{align*}
$$

$\epsilon \in \left[0, 1\right]$ is a value that introduces a
[cut-off value to improve stability](#improved-backward-error-calculation) of the algorithm and should ideally be small.

```{note}
$\epsilon = 10^{-4}$ was experimentally determined to be a reasonably good value on a number of real-world medium
voltage grids.
```

### Differences with literature

There are several differences between our implementation and the ones described in
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
and
[Schenk06](https://etna.math.kent.edu/volumes/2001-2010/vol23/abstract.php?vol=23&pages=158-179).
They are summarized below.

#### No diminishing backward error

[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
contains an early-out criterion for the [iterative refinement](#iterative-refinement-of-lu-solver-solutions) that checks
for diminishing backward error in consecutive iterations.
It amounts to (in reverse order):

1. If $\text{backward_error} \gt \frac{1}{2}\text{last_backward_error}$, then:
   1. Stop iterative refinement.
2. Else:
   1. Go to next refinement iteration.

In power systems, however, the fact that the matrix may contain elements
[spanning several orders of magnitude](#element-size-properties-of-power-system-equations) may cause slow convergence
far away from the optimum.
The diminishing criterion would cause the algorithm to terminate before the actual solution is found.
Multiple refinement iterations may still yield better results.
The power grid model therefore does not terminate early when encountering diminishing backward errors.
Instead, a maximum amount of iterations is used in combination with the error tolerance.

#### Improved backward error calculation

In power system equations, the matrix $\mathbf{M}$ in equation $\mathbf{M} \boldsymbol{x} = \boldsymbol{b}$ can contain
very discrepant entries: some may be very large while others are zero or very small (see also the
[documentation on calculations](../user_manual/calculations.md)).
The same may be true for the right-hand side of the equation $\boldsymbol{b}$, as well as its solution $\boldsymbol{x}$.
In fact, there may be certain rows $i$ for which both $\left|\boldsymbol{b}\left[i\right]\right|$ and
$\sum_j \left|\mathbf{M}\left[i,j\right]\right| \left|\boldsymbol{x}\left[j\right]\right|$ are small and, therefore,
their sum is prone to rounding errors, which may be several orders larger than machine precision.

[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
uses the following backward error in the
[iterative refinement algorithm](#iterative-refinement-of-lu-solver-solutions):

$$
\begin{align*}
\text{backward_error}_{\text{Li}}
   &= \max_i \frac{
         \left|\boldsymbol{r}_i\right|
      }{
         \sum_j \left|\mathbf{M}_{i,j}\right| \left|\boldsymbol{x}_j\right| + \left|\boldsymbol{b}_i\right|
      } \\
   &= \max_i \frac{
         \left|\boldsymbol{b}_i - \sum_j \mathbf{M}_{i,j} \boldsymbol{x}_j\right|
      }{
         \sum_j \left|\mathbf{M}_{i,j}\right| \left|\boldsymbol{x}_j\right| + \left|\boldsymbol{b}_i\right|
      } \\
   &= \max_i \frac{
         \left|\boldsymbol{r}_i\right|
      }{
         \left(\left|\mathbf{M}\right| \cdot \left|\boldsymbol{x}\right| + \left|\boldsymbol{b}\right|\right)_i
      }
\end{align*}
$$

In this equation, the symbolic notation $\left|\mathbf{M}\right|$ and $\left|\boldsymbol{x}\right|$ are the matrix and
vector with absolute values of the elements of $\mathbf{M}$ and $\boldsymbol{x}$ as elements, i.e.,
$\left|\mathbf{M}\right|_{i,j} := \left|\mathbf{M}_{i,j}\right|$ and
$\left|\boldsymbol{x}\right|_i := \left|\boldsymbol{x}_i\right|$, as defined in
[Arioli89](https://epubs.siam.org/doi/10.1137/0610013).

Due to the aforementioned, this is prone to rounding errors, and a single row with rounding errors may cause the entire
iterative refinement to fail.
The power grid model therefore uses a modified version, in which the denominator is capped to a minimum value,
determined by the maximum across all denominators:

$$
\begin{align*}
D_{\text{max}} &= \max_i\left\{
   \left(\left|\mathbf{M}\right|\cdot\left|\boldsymbol{x}\right| + \left|\boldsymbol{b}\right|\right)_i
\right\} \\
\text{backward_error} &= \max_i \left\{
   \frac{\left|\boldsymbol{r}\right|_i}{
      \max\left\{
         \left(\left|\mathbf{M}\right|\cdot\left|\boldsymbol{x}\right| + \left|\boldsymbol{b}\right|\right)_i,
         \epsilon_{\text{backward_error}} D_{\text{max}}
      \right\}
   }
\right\}
\end{align*}
$$

$\epsilon$ may be chosen.
$\epsilon = 0$ means no cut-off, while $\epsilon = 1$ means that only the absolute values of the residuals are
relevant - not the relative values.
The former is prone to rounding errors.
The latter may hide issues in rows with small coefficients by supressing them in the backward error.
This would favor rows with larger absolute, but smaller relative residuals.
This can be the case, even if that row's residual is relatively large compared to the other entries.
In conclusion, $\epsilon$ should be chosen small, but large enough to suppress numerical instabilities.
A choice for $\epsilon$ that is experimentally verified to be reasonable is mentioned
[above](#backward-error-calculation).

#### Block-wise off-diagonal infinite matrix norm

For the [pivot perturbation algorithm](#pivot-perturbation-algorithm), a matrix norm is used to determine the relative
size of the current pivot element compared to the rest of the matrix as a measure for the degree of ill-conditioning.
The norm is a variant of the $L_{\infty}$ norm of a matrix, which we call the block-wise off-diagonal infinite matrix
norm ($L_{\infty ,\text{bwod}}$).

Since the power grid model solves the matrix equations using a multi-scale matrix solver (dense intra-block,
block-sparse for the full topological structure of the grid), the norm is also taken on those same levels, so the
calculation of the norm is _block-wise_.

In addition, the diagonal blocks may have much larger elements than the off-diagonal ones, while the relevant
information is contained mostly in the off-diagonal blocks.
As a result, the block-diagonal blocks would undesirably dominate the norm.
The power grid model therefore restricts the calculation of the norm to _off-diagonal_ blocks.

In short, the $L_{\infty ,\text{bwod}}$-norm it is the $L_{\infty}$ norm of the block-sparse matrix with the
$L_{\infty}$ norm of the individual blocks as elements, where the diagonal blocks are skipped.

##### Block-wise off-diagonal infinite matrix norm algorithm

The algorithm is as follows:

Let $\mathbf{M}\equiv \mathbf{M}\left[0:N, 0:N\right]$ be the $N\times N$-matrix with a block-sparse structure and
$\mathbf{M}\left[i,j\right]$ its block element at (0-based) indices $(i,j)$, where $i,j = 0..(N-1)$.
In turn, let $\mathbf{M}\left[i,j\right] \equiv \mathbf{M}_{i,j}\left[0:N_{i,j},0:N_{i,j}\right]$ be the dense block
with dimensions $N_i\times N_j$.

1. $\text{norm} \gets 0$.
2. Loop over all block-rows: $i = 0..(N-1)$:
   1. $\text{row_norm} \gets 0$.
   2. Loop over all block-columns: $j = 0..(N-1)$ (beware of sparse structure):
      1. If $i = j$, then:
         1. Skip this block: continue with the next block-column.
      2. Else, calculate the $L_{\infty}$ norm of the current block and add to the current row norm:
         1. the current block: $\mathbf{M}_{i,j} \gets \mathbf{M}\left[i,j\right]$.
         2. $\text{block_norm} \gets 0$.
         3. Loop over all rows of the current block: $k = 0..(N_{i,j} - 1)$:
            1. $\text{block_row_norm} \gets 0$.
            2. Loop over all columns of the current block: $l = 0..(N_{i,j} - 1)$:
               1. $\text{block_row_norm} \gets \text{block_row_norm} + \left\|\mathbf{M}_{i,j}\left[k,l\right]\right\|$.
            3. Calculate the new block norm: set
               $\text{block_norm} \gets \max\left\{\text{block_norm}, \text{block_row_norm}\right\}$.
            4. Continue with the next row of the current block.
         4. $\text{row_norm} \gets \text{row_norm} + \text{block_norm}$.
         5. Continue with the next block-column.
   3. Calculate the new norm: set
      $\text{norm} \gets \max\left\{\text{norm}, \text{row_norm}\right\}$.
   4. Continue with the next block-row.

##### Illustration of the block-wise off-diagonal infinite matrix norm calculation

This section aims to illustrate how the $L_{\infty ,\text{bwod}}$-norm differs from a regular $L_{\infty}$-norm using
the following examples.

The first example shows how the taking the block-wise norm affects the calculation of the norm.

$$
\begin{bmatrix}
\begin{bmatrix}
0 && 0 \\
0 && 0
\end{bmatrix} && \begin{bmatrix}
1 && 0 \\
0 && 3
\end{bmatrix} && \begin{bmatrix}
3 && 0 \\
0 && 0
\end{bmatrix} \\
\begin{bmatrix}
5 && 0 \\
0 && 0
\end{bmatrix} &&
\begin{bmatrix}
0 && 0 \\
0 && 0
\end{bmatrix} && \begin{bmatrix}
0 && 0 \\
0 && \frac{1}{2}
\end{bmatrix} \\
\begin{bmatrix}
0 && 0 \\
0 && 0
\end{bmatrix} &&
\begin{bmatrix}
0 && 0 \\
0 && 0
\end{bmatrix} &&
\begin{bmatrix}
1 && 0 \\
0 && 1
\end{bmatrix}
\end{bmatrix}
$$

* The regular $L_{\infty}$-norm is $\max\left\{1+3, 3, 5, \frac{1}{2}, 1, 1\right\} = 5$.
* The block-wise off-diagonal infinity $L_{\infty ,\text{bwod}}$-norm is
  <!-- markdownlint-disable-next-line line-length -->
  $\max\left\{\max\left\{1, 3\right\}+\max\left\{3, 0\right\},\max\left\{5, 0\right\} + \max\left\{0, \frac{1}{2}\right\}, 1\right\} = \max\left\{3+3, 5+\frac{1}{2}, 1, 1\right\} = 6$.

The two norms clearly differ and even the elements that contribute most to the norm are different.

The next example shows how keeping only the off-diagonal blocks affects the norm.

$$
\begin{bmatrix}
\begin{bmatrix}
20 && 20 \\
30 && 0
\end{bmatrix} && \begin{bmatrix}
2 && 2 \\
3 && 0
\end{bmatrix} \\
\begin{bmatrix}
0 && 0 \\
0 && 3
\end{bmatrix} && \begin{bmatrix}
100 && 0 \\
0 && 1
\end{bmatrix}
\end{bmatrix}
$$

* The regular $L_{\infty}$-norm is $\max\left\{20+20+2+2,30+3,100,3+1\right\} = \max\left\{44,33,100,4\right\} = 100$.
* The block-wise infinity norm with diagonals would be
  <!-- markdownlint-disable-next-line line-length -->
  $\max\left\{\max\left\{20+20, 30\right\}+\max\left\{2+2, 3\right\},\max\left\{0,3\right\} + \max\left\{100, 1\right\}\right\} = \max\left\{40+4, 3+100\right\} = \max\left\{44, 103\right\} = 103$.
* The $L_{\infty ,\text{bwod}}$-norm is
  $\max\left\{\max\left\{2+2, 3\right\},\max\left\{0,3\right\}\right\} = \max\left\{4, 3\right\} = 4$.
