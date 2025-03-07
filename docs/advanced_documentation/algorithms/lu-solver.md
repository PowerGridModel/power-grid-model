<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# LU Solver

Power system equations can be modeled as matrix equations. A matrix equation solver is therefore
key to the power grid model.

This section documents the need for a custom sparse LU-solver and its implementation.

```{contents}
```

## Background

The choice for the matrix equation solver type heavily leans on the need for
[performance](#performance-considerations), the
[topological structure](#topological-structure) of the grid, and the
[properties of the equations](#power-system-equations-properties). They are documented here.

### Performance considerations

There is a large variety of applications of the power grid model that requires high performance.
This imposes some limitations on the algorithms that can be used.

* Highly accurate and fast calculations are needed for very large grids. This means that direct
  methods are strongly preferred, and approximate methods can only be used when there is no other
  alternative, and only if they can be iteratively refined with a fast convergence rate.
* In applications like time series, repetitive calculations are required. In those cases, separating
  the decomposition (also known as factorization) step of a matrix and the solving step can bring
  major performance benefits, as the decomposition remains the same across the entire set of
  calculations.

### Topological structure

Distribution grids consist of substations that distribute power in a region. This can be represented
in a topological way as vertices and edges. As a consequence of the locality of Kirchoff's laws,
power system equations also take on the same topological structure in block-matrix equation form.

#### Sparsity

It is common that a substation is fed by a single upstream substation, i.e., most grids are operated
in a tree-like structure. Meshed grid operations are rare, and even when they do happen, it is
usually only for a small section of the grid. All this gives rise to extremely sparse topologies
and, as a result, extremely [sparse matrix equations](https://en.wikipedia.org/wiki/Sparse_matrix)
with a [block structure](https://en.wikipedia.org/wiki/Block_matrix).

Sparse matrix equations can be solved efficiently: they can be solved in linear time complexity, as
opposed to the cubic complexity of naive
[Gaussian elimination](https://en.wikipedia.org/wiki/Gaussian_elimination). As a result, a sparse
matrix solver is key to the performance of the power grid model. QR-decomposition therefore is not a
good candidate.

#### Pivot operations

Pivoting of blocks is expensive, both computationally and memory-wise, as it interferes with the
sparse block structure of the matrix equations by introducing new potentially non-zero elements,
called _fill-ins_, during the process of
[Gaussian elimination](https://en.wikipedia.org/wiki/Gaussian_elimination). To this end, a pre-fixed
permutation can be chosen to avoid bock pivoting at a later stage.

The [topological structure](#topological-structure) of the grid does not change during the
solving phase, so the permutation can be obtained by the
[minimum degree algorithm](https://en.wikipedia.org/wiki/Minimum_degree_algorithm) from just the
topology alone, which seeks to minimize the amount of fill-ins. Note that matrix blocks that
contribute topologically to the matrix equation can still contain zeros. It is possible that such
zeros result in [ill-conditioned pivot elements](#pivot-perturbation). Handling of such
ill-conditioned cases is discussed in the [section on pivot perturbation](#pivot-perturbation).

### Power system equations properties

#### Matrix properties of power system equations

[Power flow equations](../../user_manual/calculations.md#power-flow-algorithms) are not Hermitian
and also not positive (semi-)definite. As a result, methods that depend on that property cannot be
used.

[State estimation equations](../../user_manual/calculations.md#state-estimation-algorithms) are
intrinsically positive definite and Hermitian, but for
[performance reasons](#performance-considerations), the matrix equation is augmented to achieve
a consistent structure across the entire topology using Lagrange multipliers.

#### Element size properties of power system equations

Power system equations may contain elements of several orders of magnitude. This may lead to
instabilities when solving the equations due to non-linearly propagated numerical errors. It is
therefore essential that the solvers function under extreme conditions by limiting numerical errors
and checking for stability.

### Block-sparsity considerations

The power flow and state estimation equations involve block-sparse matrices containing typically
dense blocks. The dimensionality of such blocks varies between different calculation types, methods
and symmetries. The blocks are typically arranged in extremely sparse fashion. The sparse structure
can be pre-calculated, but the dense blocks need to be inverted separately. To make matters worse,
the dense blocks may differ heavily in structure and contents between different nodes, and are often
not solvable without pivoting.

### Custom sparse LU-solver

The choice for a custom LU-solver implementation comes from a number of considerations:

* [LU-decomposition](https://en.wikipedia.org/wiki/LU_decomposition) is the best choice, because
  [QR-decomposition](https://en.wikipedia.org/wiki/QR_decomposition) and
  [Cholesky-decomposition](https://en.wikipedia.org/wiki/Cholesky_decomposition) cannot solve the
  power system equations efficiently as a consequence of the
  [power system equations' properties](#power-system-equations-properties),
  as the QR-decomposition is not as efficient when dealing with sparse matrices, while the latter
  requires Hermitian and/or semi-definite (in generalized form).
* Alternative LU-solver implementations are optimized for a variety of use cases that are less
  sparse than the ones encountered in power systems.
* Alternative LU-solver implementations do not have good block-sparse matrix equation solvers.

## Implementation

The LU-solver implemented in the power grid model consists of 3 components:

* A block-sparse LU-solver that:
  * handles factorization using the topological structure down to block-level
  * solves the sparse matrix equation given the factorization
* A dense LU-factor that handles individual blocks within the matrix equation

### Dense LU-factorization

The power grid model uses a modified version of the
[`LUFullPiv` defined in Eigen](https://gitlab.com/libeigen/eigen/-/blob/3.4/Eigen/src/LU/FullPivLU.h)
(credits go to the original author). The modification adds opt-in support for
[pivot perturbation](#pivot-perturbation).

#### Dense LU-factorization process

The [Gaussian elimination](https://en.wikipedia.org/wiki/Gaussian_elimination) process itself is, as
usual, by iterating over all pivot elements $p$. Let
$\mathbb{M}_p\equiv\begin{bmatrix} m_p && \boldsymbol{r}_p^T \\ \boldsymbol{q}_p && \hat{\mathbb{M}}_p\end{bmatrix}$,
be the matrix block at some iteration step where $p$ is the current pivot element at the top
left of the matrix, $m_p$ its matrix element value, $\boldsymbol{q}$ and $\boldsymbol{r}_p^T$ are
the associated column and row vectors containing the rest of the pivot column and row and
$\hat{\mathbb{M}}_p$ is the bottom-right block of the matrix. Note that in the bigger picture of the
full matrix, this looks as follows, where the $m_k$, $\boldsymbol{l}_k$ and $\boldsymbol{u}_k$
($k < p$) denote sections of the matrix that already have been decomposed in previous iteration
steps (see below).

$$
\begin{bmatrix}
    m_0 && \boldsymbol{u}_0^T && && \\
    \boldsymbol{l}_0 && \ddots && \ddots && \\
    \vdots && \ddots && \ddots && \\
    && && && \mathbb{M}_p
\end{bmatrix}\equiv\begin{bmatrix}
    m_0 && \boldsymbol{u}_0^T && && && \\
    \boldsymbol{l}_0 && \ddots && \ddots && && \\
    \vdots && \ddots && m_{p-1} && \boldsymbol{u}_{p-1}^T && \\
    && && \boldsymbol{l}_{p-1} && m_p && \boldsymbol{r}_p^T \\
    && && && \boldsymbol{q}_p && \hat{\mathbb{M}}_p\end{bmatrix}
$$

[Gaussian elimination](https://en.wikipedia.org/wiki/Gaussian_elimination) using
[LU-decomposition](https://en.wikipedia.org/wiki/LU_decomposition) constructs the matrices

$$
\begin{align*}
\mathbb{L}_p &= \begin{bmatrix} 1 && \boldsymbol{0}^T \\ m_p^{-1} \boldsymbol{q}_p && \mathbb{1}_p\end{bmatrix} \\
\mathbb{U}_p &= \begin{bmatrix} m_p && \boldsymbol{r}_p^T \\\boldsymbol{0} && \mathbb{1}_p\end{bmatrix} \\
\mathbb{M}_{p+1} &= \hat{\mathbb{M}}_p - m_p^{-1} \boldsymbol{q}_p \boldsymbol{r}_p^T
\end{align*}
$$

where $\mathbb{1}$ is the matrix with ones on the diagonal and zeros as off-diagonal elements and
$\mathbb{M}_{p+1}$ is the start of the next iteration. They together form the LU-decomposition as

$$
\mathbb{M}_p \equiv
\begin{bmatrix} 1 && \boldsymbol{0}^T \\ m_p^{-1} \boldsymbol{q}_p && \mathbb{1}_p\end{bmatrix}\begin{bmatrix}
    1 && \boldsymbol{0}^T \\
    \boldsymbol{0} && \hat{\mathbb{M}}_p - m_p^{-1} \boldsymbol{q}_p \boldsymbol{r}_p^T
\end{bmatrix}\begin{bmatrix} m_p && \boldsymbol{r}_p^T \\\boldsymbol{0} && \mathbb{1}_p\end{bmatrix}
\equiv \mathbb{L}_p \begin{bmatrix}
    1 && \boldsymbol{0}^T \\
    \boldsymbol{0} && \mathbb{M}_{p+1}
\end{bmatrix} \mathbb{U}_p
$$

or, with the previous iteration also shown,

$$
\mathbb{M}_{p-1} \equiv \mathbb{L}_{p-1} \begin{bmatrix}
    1 && \boldsymbol{0}^T \\
    \boldsymbol{0} && \mathbb{L}_p
\end{bmatrix}\begin{bmatrix}
    1 && 0 && \boldsymbol{0}^T \\
    0 && 1 && \boldsymbol{0}^T \\
    \boldsymbol{0} && \boldsymbol{0} && \mathbb{M}_{p+1}
\end{bmatrix} \begin{bmatrix}
    1 && \boldsymbol{0}^T \\
    \boldsymbol{0} && \mathbb{U}_p
\end{bmatrix}\mathbb{U}_{p-1}
$$

in which $\mathbb{L}_{p-1}$ and $\mathbb{U}_{p-1}$ have dimensions
$\left(N-p+1\right)\times \left(N-p+1\right)$, $\mathbb{L}_p$ and $\mathbb{U}_p$ have dimensions
$\left(N-p\right) \times \left(N-p\right)$, and $\mathbb{M}_{p+1}$ has dimensions
$\left(N-p-1\right) \times \left(N-p-1\right)$.

Iterating this process yields the matrices

$$
\begin{align*}
\mathbb{L} = \begin{bmatrix}
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
\mathbb{U} = \begin{bmatrix}
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

in which $\boldsymbol{l}_p$ is the first column of the lower triangle of $\mathbb{L}_p$ and
$\boldsymbol{u}_p^T$ is the first row of the upper triangle of $\mathbb{U}_p$.

The process described in the above assumes no pivot permutations were necessary. If permutations
are required, they are kept track of in separate row-permution and column-permutation matrices
$\mathbb{P}$ and $\mathbb{Q}$, such that $\mathbb{P}\mathbb{M}\mathbb{Q} = \mathbb{L}\mathbb{U}$,
which can be rewritten as

$$
\mathbb{M} = \mathbb{P}^{-1}\mathbb{L}\mathbb{U}\mathbb{Q}^{-1}
$$

#### Dense LU-factorization algorithm

The power grid model uses an in-place approach. Permutations are separately stored.

Let $\mathbb{M}$ be the $N\times N$-matrix and
$\mathbb{M}\left[i,j\right]$ its element at (0-based) indices $(i,j)$, where $i,j = 0..(N-1)$.

1. Initialize the permutations $\mathbb{P}$ and $\mathbb{Q}$ to the identity permutation.
2. Initialize fill-in elements to $0$.
3. Loop over all rows: $p = 0..(N-1)$:
   1. Set the remaining matrix: $\mathbb{M}_p \gets \mathbb{M}\left[p:N,p:N\right]$ with size
      $N_p\times N_p$.
   2. Find largest element $\mathbb{M}_p\left[i_p,j_p\right]$ in $\mathbb{M}_p$ by magnitude. This
      is the pivot element.
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
   5. Swap the first and pivot row and column of $\mathbb{M}_p$, so that the pivot element is in the
      top-left corner of the remaining matrix:
      1. $\mathbb{M}_p\left[0,0:N_p\right] \leftrightarrow \mathbb{M}\left[i_p,0:N_p\right]$
      2. $\mathbb{M}_p\left[0:N_p,0\right] \leftrightarrow \mathbb{M}\left[0:N_p,j_p\right]$
   6. Apply Gaussian elimination for the current pivot element:
      1. $\mathbb{M}_p\left[0,0:N_p\right] \gets \frac{1}{\mathbb{M}_p[0,0]}\mathbb{M}_p\left[0,0:N_p\right]$
      2. $\mathbb{M}_p\left[1:N_p,0:N_p\right] \gets \mathbb{M}_p\left[1:N_p,0:N_p\right] - \mathbb{M}_p\left[1:N_p,0\right] \otimes \mathbb{M}_p\left[0,0:N_p\right]$
   7. Accumulate the permutation matrices:
      1. In $\mathbb{P}$: swap $p \leftrightarrow p + i_p$
      2. In $\mathbb{Q}$: swap $p \leftrightarrow p + j_p$
   8. Continue with the next $p$ to factorize the the bottom-right block.

$\mathbb{L}$ is now the matrix composed of ones on the diagonal and the bottom-left off-diagonal
triangle of $\mathbb{M}$ and zeros in the upper-right off-diagonal triangle. $\mathbb{U}$ is the
matrix composed of the upper-right triangle of $\mathbb{M}$ including the diagonal elements and
zeros in the lower-left off-diagonal triangle.

```{note}
In the actual implementation, we use a slightly different order of operations: instead of raising
the `SparseMatrixError` immediately, we break from the loop and throw after that. This does not
change the functional behavior.
```

```{note}
Permutations are only allowed within each dense block, for the reasons described
[earlier](#pivot-operations).
```

### Block-sparse LU-factorization

The LU-factorization process for block-sparse matrices is similar to that for
[dense matrices](#dense-lu-factorization), but in this case, $m_p$ is a block element, and
$\boldsymbol{q}_p$, $\boldsymbol{r}_p^T$ and $\hat{\mathbb{M}}_p$ consist of block elements as well.
Notice that the inverse $m_p^{-1}$ can be calculated from its LU-decomposition, which can be
obtained from the [dense LU-factorization process](#dense-lu-factorization-process).

#### Block-sparse LU-factorization process

The block-sparse LU-factorization process is a little bit different from
[before](#dense-lu-factorization-process). Below, we first
[describe the naive approach](#naive-block-sparse-lu-factorization-process), followed by the
rationale and proof of the alternative approach.

##### Naive block-sparse LU-factorization process

A naive first attempt at block-sparse LU-factorization is done as follows. The
[dense LU-factorization process](#dense-lu-factorization-process) is followed but on blocks instead
of single elements. Let
$\mathbb{M}_p\equiv\begin{bmatrix}\mathbb{m}_p && \pmb{\mathbb{r}}_p^T \\ \pmb{\mathbb{q}}_p && \hat{\pmb{\mathbb{M}}}_p\end{bmatrix}$
be the block-sparse matrix to decompose. $\mathbb{m}_p$ is a dense block that can be
[LU-factorized](#dense-lu-factorization-process):
$\mathbb{m}_p = \mathbb{p}_p^{-1} \mathbb{l}_p \mathbb{u}_p \mathbb{q}_p^{-1}$, where
the lower-case helps avoiding confusion with the block-sparse matrix components. [Gaussian
elimination](https://en.wikipedia.org/wiki/Gaussian_elimination) using
[LU-decomposition](https://en.wikipedia.org/wiki/LU_decomposition) constructs the matrices

$$
\begin{align*}
\mathbb{L}_p &= \begin{bmatrix} 1 && \pmb{\mathbb{0}}^T \\ \overrightarrow{\mathbb{m}_p^{-1}\pmb{\mathbb{q}}_p} && \pmb{\mathbb{1}}_p\end{bmatrix} \\
\mathbb{U}_p &= \begin{bmatrix} \mathbb{m}_p && \pmb{\mathbb{r}}_p^T \\ \pmb{\mathbb{0}} && \pmb{\mathbb{1}}_p \end{bmatrix} \\
\pmb{\mathbb{M}}_{p+1} &= \hat{\pmb{\mathbb{{M}}}}_p - \widehat{\mathbb{m}_p^{-1}\pmb{\mathbb{q}}_p \pmb{\mathbb{r}}_p^T}
\end{align*}
$$

Here, $\overrightarrow{\mathbb{m}_p^{-1}\pmb{\mathbb{q}}_p}$ is symbolic notation for the
block-vector of solutions of the equation $\mathbb{m}_p x_{p;k} = \mathbb{q}_{p;k}$, where
$k = 0..(p-1)$. Similarly, $\widehat{\mathbb{m}_p^{-1}\pmb{\mathbb{q}}_p \pmb{\mathbb{r}}_p^T}$ is
symbolic notation for the block-matrix of solutions of the equation
$\mathbb{m}_p x_{p;k,l} = \mathbb{q}_{p;k} \mathbb{r}_{p;l}^T$, where
$k,l = 0..(p-1)$. That is:

$$
\begin{align*}
\overrightarrow{\mathbb{m}_p^{-1}\pmb{\mathbb{q}}_p}
&= \begin{bmatrix}\mathbb{m}_p^{-1}\pmb{\mathbb{q}}_{p;0} \\
\vdots \\
\mathbb{m}_p^{-1} \pmb{\mathbb{q}}_{p;N-1} \end{bmatrix} \\
\widehat{\mathbb{m}_p^{-1}\pmb{\mathbb{q}}_p \pmb{\mathbb{r}}_p^T}
&= \begin{bmatrix}\mathbb{m}_p^{-1} \pmb{\mathbb{q}}_{p;0} \pmb{\mathbb{r}}_{p;0}^T && \cdots &&
\mathbb{m}_p^{-1} \pmb{\mathbb{q}}_{p;0} \pmb{\mathbb{r}}_{p;N-1}^T \\
\vdots && \ddots && \vdots \\
\mathbb{m}_p^{-1} \pmb{\mathbb{q}}_{p;0} \pmb{\mathbb{r}}_{p;0}^T && \cdots &&
\mathbb{m}_p^{-1} \pmb{\mathbb{q}}_{p;N-1} \pmb{\mathbb{r}}_{p;N-1}^T \end{bmatrix}
\end{align*}
$$

Iteratively applying above factorization process yields $\mathbb{L}$ and $\mathbb{U}$, as well as
$\mathbb{P}$ and $\mathbb{Q}$.

Unfortunately, obtaining $\mathbb{m}_p^{-1} \boldsymbol{x}_p$ is numerically unstable, even if it
is split into multiple solving steps. Instead, the process below is used.

##### Partially solved block-sparse LU-factorization process

A more numerically stable approach compared to the
[naive approach](#naive-block-sparse-lu-factorization-process) is equivalent to single-element
pivot. Here, we describe the results. Instead of applying the full pivot decomposition in the
$L$-matrix and delaying the solve process all the way to after the solving phase, we only apply a
partial pivot to the $L$ matrix, and the other half in the $U$ matrix. This is equivalent to single-
element pivoting instead of block-pivoting. For the
[rationale](#rationale-of-the-block-sparse-lu-factorization-process) and
[proof](#proof-of-block-sparse-lu-factorization-process), see below. Here, only the results are
presented.

Using the same conventions as [before](#naive-block-sparse-lu-factorization-process), Let
$\mathbb{M}_p\equiv\begin{bmatrix}\mathbb{m}_p && \pmb{\mathbb{r}}_p^T \\ \pmb{\mathbb{q}}_p && \hat{\pmb{\mathbb{M}}}_p\end{bmatrix}$
be the block-sparse matrix to decompose. $\mathbb{m}_p$ is a dense block that can be
[LU-factorized](#dense-lu-factorization-process):
$\mathbb{m}_p = \mathbb{p}_p^{-1} \mathbb{l}_p \mathbb{u}_p \mathbb{q}_p^{-1}$. Partial Gaussian
elimination constructs the matrices

$$
\begin{align*}
\mathbb{L}_p &= \begin{bmatrix} \mathbb{l}_p && \pmb{\mathbb{0}}^T \\ \overrightarrow{\pmb{\mathbb{q}}_p\mathbb{q}_p\mathbb{u}_p^{-1}} && \pmb{\mathbb{1}}_p\end{bmatrix} \\
\mathbb{U}_p &= \begin{bmatrix} \mathbb{u}_p && \overrightarrow{\mathbb{l}_p\mathbb{p}_p\pmb{\mathbb{r}}_p}^T \\ \pmb{\mathbb{0}} && \pmb{\mathbb{1}}_p \end{bmatrix} \\
\pmb{\mathbb{M}}_{p+1} &= \hat{\pmb{\mathbb{{M}}}}_p - \widehat{\pmb{\mathbb{q}}_p\mathbb{q}_p\mathbb{u}_p^{-1}\mathbb{l}_p^{-1}\mathbb{p}_p\pmb{\mathbb{r}}_p^T}
\end{align*}
$$

Note that the first column of $\mathbb{L}_p$ can be obtained by applying a right-solve procedure,
instead of the regular left-solve procedure, as is the case for the $\mathbb{U}_p$.

##### Rationale of the block-sparse LU-factorization process

To illustrate the rationale, let's perform the full matrix solving (without using blocks):

$$
\begin{align*}
\begin{pmatrix}
\mathbb{a} && \mathbb{b} \\
\mathbb{c} && \mathbb{d}
\end{pmatrix}
&=
\begin{pmatrix}
a_{11} && a_{12} && b_{11} && b_{12} \\
a_{21} && a_{22} && b_{21} && b_{22} \\
c_{11} && c_{12} && d_{11} && d_{12} \\
c_{21} && c_{22} && d_{21} && d_{22}
\end{pmatrix} \\
& \mapsto
\begin{pmatrix}
a_{11} && a_{12} && b_{11} && b_{12} \\
\frac{a_{21}}{a_{11}} && a_{22} - a_{12} \frac{a_{21}}{a_{11}} && b_{21} - b_{11}\frac{a_{21}}{a_{11}} && b_{22} - b_{12}\frac{a_{21}}{a_{11}} \\
\frac{c_{11}}{a_{11}} && c_{12} - a_{12} \frac{c_{11}}{a_{11}} && d_{11} - b_{11}\frac{c_{11}}{a_{11}} && d_{12} - b_{12}\frac{c_{11}}{a_{11}} \\
\frac{c_{21}}{a_{11}} && c_{22} - a_{12} \frac{c_{21}}{a_{11}} && d_{21} - b_{11}\frac{c_{21}}{a_{11}} && d_{22} - b_{12}\frac{c_{21}}{a_{11}}
\end{pmatrix} \\
& \mapsto
\begin{pmatrix}
a_{11} && a_{12} && b_{11} && b_{12} \\
\frac{a_{21}}{a_{11}} &&
    a_{22} - a_{12} \frac{a_{21}}{a_{11}} &&
    b_{21} - b_{11}\frac{a_{21}}{a_{11}} &&
    b_{22} - b_{12}\frac{a_{21}}{a_{11}} \\
\frac{c_{11}}{a_{11}} &&
    \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{11} - b_{11}\frac{c_{11}}{a_{11}} - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right) \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{12} - b_{12}\frac{c_{11}}{a_{11}} - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right) \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}  \\
\frac{c_{21}}{a_{11}} &&
    \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{21} - b_{11}\frac{c_{21}}{a_{11}} - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right) \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
    d_{22} - b_{12}\frac{c_{21}}{a_{11}} - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right) \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{pmatrix} \\
&\equiv
\begin{pmatrix}
\mathbb{l}_a \mathbb{u}_a && \mathbb{u}_b \\
\mathbb{l}_c              && \mathbb{l}_d \mathbb{u}_d
\end{pmatrix}
\end{align*}
$$

so that

$$
\begin{align*}
\mathbb{l}_a &= \begin{pmatrix}
    1                     && 0 \\
    \frac{a_{21}}{a_{11}} && 1
\end{pmatrix} \\
\mathbb{u}_a &= \begin{pmatrix}
    a_{11} && a_{12} \\
    0 && a_{22} - a_{12} \frac{a_{21}}{a_{11}}
\end{pmatrix} \\
\mathbb{l}_c &= \begin{pmatrix}
    \frac{c_{11}}{a_{11}} && \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} \\
    \frac{c_{21}}{a_{11}} && \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{pmatrix} \\
\mathbb{u}_b &= \begin{pmatrix}
    b_{11}                               && b_{12} \\
    b_{21} - b_{11}\frac{a_{21}}{a_{11}} && b_{22} - b_{12}\frac{a_{21}}{a_{11}}
\end{pmatrix} \\
\mathbb{l}_d\mathbb{u}_d &= \begin{pmatrix}
    d_{11} - b_{11}\frac{c_{11}}{a_{11}} - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right) \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
        d_{12} - b_{12}\frac{c_{11}}{a_{11}} - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right) \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}  \\
    d_{21} - b_{11}\frac{c_{21}}{a_{11}} - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right) \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
        d_{22} - b_{12}\frac{c_{21}}{a_{11}} - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right) \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{pmatrix}
\end{align*}
$$

Interestingly, the matrices $\mathbb{l}_c$, $\mathbb{u}_b$ and $\mathbb{l}_d\mathbb{u}_d$ can be
obtained without doing full pivoting on the sub-block level:

* $\mathbb{l}_c$ is the solution to the right-multiplication matrix equation
  $\mathbb{l}_c \mathbb{u}_a = \mathbb{c}$
* $\mathbb{u}_b$ is the solution to the left-multiplication matrix equation
  $\mathbb{l}_a \mathbb{u}_b = \mathbb{b}$.
* $\mathbb{l}_d\mathbb{u}_d$ denotes the start matrix of the decomposition of the next iteration and
  is equal to $\mathbb{l}_d\mathbb{u}_d = \mathbb{d} - \mathbb{l}_c \mathbb{u}_b$.

This process generalizes to block matrices of any size and block-invertible matrices of any size:
$\mathbb{c}$ and $\mathbb{b}$ become a column- and row-vector of block-matrices for which the
individual block-elements of the vectorized decompositions $\mathbb{l}_c$ and $\mathbb{u}_b$ can be
obtained by solving above equations.

If, during the LU-decomposition of the pivot block, a row- and column-permutation was used,
$\mathbb{a} = \mathbb{p}_a^{-1} \mathbb{l}_a \mathbb{u}_p \mathbb{q}_a^{-1}$, and the columns of
$\mathbb{c}$ and the rows of $\mathbb{b}$ are permuted: $\mathbb{c}\mapsto\mathbb{c}\mathbb{q}_a$
and $\mathbb{b}\mapsto\mathbb{p}_a\mathbb{b}$. As a result, the equations generalize as follows.

* $\mathbb{l}_c$ is the solution to the right-multiplication matrix equation
  $\mathbb{l}_c \mathbb{u}_a = \mathbb{c} \mathbb{q}_a$.
* $\mathbb{u}_b$ is the solution to the left-multiplication matrix equation
  $\mathbb{l}_a \mathbb{u}_b = \mathbb{p}_a \mathbb{b}$.
* $\mathbb{l}_d\mathbb{u}_d$ denotes the start matrix of the decomposition of the next iteration and
  is equal to $\mathbb{l}_d\mathbb{u}_d = \mathbb{d} - \mathbb{l}_c \mathbb{u}_b$.

##### Proof of block-sparse LU-factorization process

The following proves the equations $\mathbb{l}_c \mathbb{u}_a = \mathbb{c}$,
$\mathbb{l}_a \mathbb{u}_b = \mathbb{b}$ and
$\mathbb{l}_d\mathbb{u}_d = \mathbb{d} - \mathbb{l}_c \mathbb{u}_b$ mentioned in the
[previous section](#rationale-of-the-block-sparse-lu-factorization-process).

$$
\begin{align*}
\mathbb{l}_c \mathbb{u}_a
&=
\begin{pmatrix}
    \frac{c_{11}}{a_{11}} && \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} \\
    \frac{c_{21}}{a_{11}} && \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{pmatrix} \begin{pmatrix}
    a_{11} && a_{12} \\
    0 && a_{22} - a_{12} \frac{a_{21}}{a_{11}}
\end{pmatrix} \\
&= \begin{pmatrix}
    a_{11} \frac{c_{11}}{a_{11}} && a_{12} \frac{c_{11}}{a_{11}} + \left(a_{22} - a_{12} \frac{a_{21}}{a_{11}}\right)\frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} \\
    a_{11} \frac{c_{21}}{a_{11}} && a_{12} \frac{c_{21}}{a_{11}} + \left(a_{22} - a_{12} \frac{a_{21}}{a_{11}}\right)\frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{pmatrix} \\
&= \begin{pmatrix}
    c_{11} && c_{11} + \left(c_{12} - c_{11}\right) \\
    c_{21} && c_{21} + \left(c_{22} - c_{21}\right)
\end{pmatrix} \\
&= \begin{pmatrix}
    c_{11} && c_{12} \\
    c_{21} && c_{22}
\end{pmatrix} \\
&= \mathbb{c} \\
\mathbb{l}_a \mathbb{u}_b
&= \begin{pmatrix}
    1                     && 0 \\
    \frac{a_{21}}{a_{11}} && 1
\end{pmatrix} \begin{pmatrix}
    b_{11}                               && b_{12} \\
    b_{21} - b_{11}\frac{a_{21}}{a_{11}} && b_{22} - b_{12}\frac{a_{21}}{a_{11}}
\end{pmatrix} \\
&= \begin{pmatrix}
    b_{11}                                                               && b_{12} \\
    b_{11} \frac{a_{21}}{a_{11}} + b_{21} - b_{11} \frac{a_{21}}{a_{11}} && b_{12} \frac{a_{21}}{a_{11}} + b_{22} - b_{12}\frac{a_{21}}{a_{11}}
\end{pmatrix} \\
&= \begin{pmatrix}
    b_{11} && b_{12} \\
    b_{21} && b_{22}
\end{pmatrix} \\
&= \mathbb{b} \\
\mathbb{l}_d\mathbb{u}_d
&= \mathbb{d} - \mathbb{l}_c \mathbb{u}_b \\
&= \begin{pmatrix}d_{11} && d_{12} \\ d_{21} && d_{22} \end{pmatrix} - \begin{pmatrix}
    \frac{c_{11}}{a_{11}} && \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} \\
    \frac{c_{21}}{a_{11}} && \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{pmatrix} \begin{pmatrix}
    b_{11}                               && b_{12} \\
    b_{21} - b_{11}\frac{a_{21}}{a_{11}} && b_{22} - b_{12}\frac{a_{21}}{a_{11}}
\end{pmatrix} \\
&= \begin{pmatrix}
    d_{11} - b_{11}\frac{c_{11}}{a_{11}} - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right) \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
        d_{12} - b_{12}\frac{c_{11}}{a_{11}} - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right) \frac{c_{12} - a_{12} \frac{c_{11}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} \\
    d{21} - b_{11}\frac{c_{21}}{a_{11}} - \left(b_{21} - b_{11}\frac{a_{21}}{a_{11}}\right) \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}} &&
        d_{22} - b_{12}\frac{c_{21}}{a_{11}} - \left(b_{22} - b_{12}\frac{a_{21}}{a_{11}}\right) \frac{c_{22} - a_{12} \frac{c_{21}}{a_{11}}}{a_{22} - a_{12} \frac{a_{21}}{a_{11}}}
\end{pmatrix} \\
&= \mathbb{l}_d\mathbb{u}_d
\end{align*}
$$

Generalizability to larger block sizes and block-matrix sizes follows from fhe fact that
$\mathbb{l}_c$ and $\mathbb{u}_b$ only depend on the in-block LU-decomposition of the pivot block
$(\mathbb{l}_a,\mathbb{u}_a)$ and the data in the respective blocks ($\mathbb{c}$ and $\mathbb{b}$)
in the original matrix by varying $c$ and $b$ over other blocks.

#### Block-sparse indices

The structure of the block-sparse matrices is as follows.

* The $N\times N$ block matrix $\mathbb{M}$ is interpreted as the block-matrix, with
  $\mathbb{M}\left[i,j\right]$ its block element at (0-based) indices $(i,j)$, where
  $i,j = 0..(N-1)$.
* In turn, let $\mathbb{M}\left[i,j\right]\equiv\mathbb{M}_{i,j}$ be the dense block with dimensions
  $N_i\times N_j$.

This can be graphically represented as

$$
\begin{align*}
\mathbb{M} &\equiv \begin{pmatrix}
\mathbb{M}_{0,0}   && \cdots && \mathbb{M}_{0,N-1} \\
\vdots    && \ddots && \vdots \\
\mathbb{M}_{N-1,0} && \cdots && \mathbb{M}_{N-1,N-1}
\end{pmatrix} \\
&\equiv \begin{pmatrix}
\begin{pmatrix}
   \mathbb{M}_{0,0}\left[0,0\right]     && \cdots && \mathbb{M}_{0,0}\left[0,N_j-1\right] \\
   \vdots                      && \ddots && \vdots \\
   \mathbb{M}_{0,0}\left[N_i-1,0\right] && \cdots && \mathbb{M}_{0,0}\left[N_i-1,N_j-1\right]
\end{pmatrix} && \cdots && \begin{pmatrix}
   \mathbb{M}_{0,N-1}\left[0,0\right]     && \cdots && \mathbb{M}_{0,N-1}\left[0,N_j-1\right] \\
   \vdots                        && \ddots && \vdots \\
   \mathbb{M}_{0,N-1}\left[N_i-1,0\right] && \cdots && \mathbb{M}_{0,N-1}\left[N_i-1,N_j-1\right] \end{pmatrix} \\
\vdots && \ddots && \vdots \\
\begin{pmatrix}
   \mathbb{M}_{N-1,0}\left[0,0\right]     && \cdots && \mathbb{M}_{N-1,0}\left[0,N_j-1\right] \\
   \vdots                        && \ddots && \vdots \\
   \mathbb{M}_{N-1,0}\left[N_i-1,0\right] && \cdots && \mathbb{M}_{N-1,0}\left[N_i-1,N_j-1\right]
\end{pmatrix} && \cdots && \begin{pmatrix}
   \mathbb{M}_{N-1,N-1}\left[0,0\right]     && \cdots && \mathbb{M}_{N-1,N-1}\left[0,N_j-1\right] \\
   \vdots                          && \ddots && \vdots \\
   \mathbb{M}_{N-1,N-1}\left[N_i-1,0\right] && \cdots && \mathbb{M}_{N-1,N-1}\left[N_i-1,N_j-1\right] \end{pmatrix}
\end{pmatrix}
\end{align*}
$$

Because of the sparse structure and the fact that all $\mathbb{M}_{i,j}$ have the same shape, it is
much more efficient to store the blocks $\mathbb{M}_{i,j}$ in a vector $\mathbb{M}_{\tilde{k}}$
where $\tilde{k}$ is a reordered index from $(i,j) \mapsto \tilde{k}$. This mapping, in turn, is
stored as an index pointer, i.e., a vector of vectors of indices, with the outer index given by the
row-index $i$, and the inner vector containing the values of $j$ for which $\mathbb{M}_{i,j}$ may be
non-zero. All topologically relevant matrix elements, as well as [fill-ins](#pivot-operations), are
included in this mapping. The following illustrates this mapping.

$$
\begin{align*}
\begin{pmatrix}
\mathbb{M}_{0,0} &&         &&         && \mathbb{M}_{0,3} \\
        && \mathbb{M}_{1,1} && \mathbb{M}_{1,2} &&         \\
        && \mathbb{M}_{2,1} && \mathbb{M}_{2,2} && \mathbb{M}_{2,3} \\
\mathbb{M}_{3,0} &&         && \mathbb{M}_{3,2} && \mathbb{M}_{3,3}
\end{pmatrix} &\equiv
\begin{bmatrix}
\mathbb{M}_{0,0} && \mathbb{M}_{0,3} && \mathbb{M}_{1,1} && \mathbb{M}_{1,2} && \mathbb{M}_{2,1} && \mathbb{M}_{2,2} && \mathbb{M}_{2,3} && \mathbb{M}_{3,0} && \mathbb{M}_{3,2} && \mathbb{M}_{3,3} \\
[[0 && 3] && [1 && 2] && [1 && 2 && 3] && [0 && 2 && 3]] \\
\end{bmatrix} \\
&\equiv
\begin{bmatrix}
\mathbb{M}_{0,0} && \mathbb{M}_{0,3} && \mathbb{M}_{1,1} && \mathbb{M}_{1,2} && \mathbb{M}_{2,1} && \mathbb{M}_{2,2} && \mathbb{M}_{2,3} && \mathbb{M}_{3,0} && \mathbb{M}_{3,2} && \mathbb{M}_{3,3} && \\
[0 && 3 && 1 && 2 && 1 && 2 && 3 && 0 && 2 && 3] && \\
[0 && && 2 && && 4 && && && 7 && && && && 10]
\end{bmatrix}
\end{align*}
$$

In the first equation, the upper row contains the present block entries and the bottom row their
column indices per row to obtain a flattened representation of the matrix. In the last equivalence,
the column indices, in turn, are also flattened into a separate flattened representation of the
column indices and the start indices of each row - the index pointer (`indptr`). Note that the size
of `indptr` is 1 greater than the amount of rows. This enables describing the amount of elements
in each row as the difference between its element in `indptr` and the next element.

Looping over the rows and columns becomes trivial. Let $\text{indptr}$ be the start. The double loop
becomes:

1. Loop all rows: $i=0..(N-1)$:
   1. The column indices for this row start at $\text{indptr}\left[i\right]$.
   2. The column indices for this row stop at $\text{indptr}\left[i+1\right]$.
   3. Loop all columns:
      $j=\left(\text{indptr}\left[i\right]\right)..\left(\text{indptr}\left[i+1\right]\right)$:
      1. The current row index is $i$.
      2. The current column index is $j$.

### Block-sparse LU solving

Solving an equation $\mathbb{M} \boldsymbol{x} = \boldsymbol{b}$ using a
[pre-factored LU-factorization](#block-sparse-lu-factorization) is done using the regular forward
and backward substitutions. Since the matrix is already ordered such that the amount of
[fill-ins](#pivot-operations) is minimal, permutations are restricted to within each block. The
first step of the block-sparse LU solving therefore permutes the rows within each block:
$\boldsymbol{b}^{\prime} = \mathbb{P}\boldsymbol{b}$. After that, the forward substitution step
essentially amounts to solving the matrix equation
$\mathbb{L}\boldsymbol{y} = \boldsymbol{b}^{\prime}$, followed by the backwards substitution by
solving $\mathbb{U}\boldsymbol{z} = \mathbb{y}$. The final result is then obtained by applying the
column permutation within each block: $\boldsymbol{x} = \mathbb{Q}\boldsymbol{z}$.

#### Forward substitution

The row permutation is applied as follows.

1. Loop over all block-rows: $i=0..(N-1)$:
   1. If the matrix is a block matrix:
      1. Apply the current row's block permutation:
         $\boldsymbol{b}\left[i\right] \gets \mathbb{P}\left[i\right] \cdot \boldsymbol{b}\left[i\right]$.
      2. Proceed.
   2. Else:
      1. Proceed.

The equation $\mathbb{L}\boldsymbol{y} = \mathbb{P}\boldsymbol{b}$ is solved as follows.

1. Loop over all block-rows: $i=0..(N-1)$:
   1. Loop over all lower-triangle off-diagonal columns (beware of sparsity): $j=0..(i-1)$:
      1. $\boldsymbol{b}\left[i\right] \gets \boldsymbol{b}\left[i\right] - \mathbb{L}\left[i,j\right] \cdot \boldsymbol{b}\left[j\right]$.
      2. Continue with next block-column.
   2. If the matrix is a block matrix:
      1. Follow the same steps within the block.
      2. Proceed.
   3. Else:
      1. Proceed.

#### Backward substitution

The equation $Uz = y$ is solved as follows.

1. Loop over all block-rows in reverse order: $i=(N-1)..0$:
   1. Loop over all upper-triangle off-diagonal columns (beware of sparsity): $j=(i+1)..0$:
      1. $\boldsymbol{b}\left[i\right] \gets \boldsymbol{b}\left[i\right] - \mathbb{U}\left[i,j\right] \cdot \boldsymbol{b}\left[j\right]$.
      2. Continue with next block-column.
   2. Handle the diagonal element:
      1. If the matrix is a block matrix:
         1. Follow the same steps within the block.
         2. Proceed.
      2. Else:
         1. $\boldsymbol{b}\left[i\right] \gets \boldsymbol{b}\left[i\right] / \mathbb{U}\left[i,i\right]$.
         2. Proceed.

Apply the column permutation as follows.

1. Loop over all block-rows: $i=0..(N-1)$:
   1. If the matrix is a block matrix:
      1. Apply the current row's block permutation:
         $\boldsymbol{b}\left[i\right] \gets \mathbb{Q}\left[i\right] \cdot \boldsymbol{b}\left[i\right]$.
      2. Proceed.
   2. Else:
      1. Proceed.

```{note}
If [pivot perturbation](#pivot-perturbation) was used to obtain the LU-decomposition, the solution
obtained here is an approximation of the exact solution. The approximation can be improved using
[iterative refinement](#iterative-refinement-of-lu-solver-solutions).
```

### Pivot perturbation

The LU-solver implemented in the power grid model has support for pivot perturbation. The methods
are described in
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
and
[Schenk06](https://etna.math.kent.edu/volumes/2001-2010/vol23/abstract.php?vol=23&pages=158-179).
Pivot perturbation consists of selecting a pivot element. If its magnitude is too small compared
to that of the other elements in the matrix, then it cannot be used in its current form. Selecting
another pivot element, via permutations with other blocks, is not desirable, as described in the
section on [pivot operations](#pivot-operations), so the matrix is ill-conditioned.

Instead, a small perturbation is done on the pivot element. This makes the matrix equation solvable
without selecting a different pivot element, at the cost of propagating rounding errors.
slightly different matrix that is then iteratively refined. This method is also used by
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
and
[Schenk06](https://etna.math.kent.edu/volumes/2001-2010/vol23/abstract.php?vol=23&pages=158-179),
as well as the well-known
[MKL Pardiso solver](https://www.intel.com/content/www/us/en/docs/onemkl/developer-reference-c/2023-0/onemkl-pardiso-parallel-direct-sparse-solver-iface.html)

#### Pivot perturbation algorithm

The following pivot perturbation algorithm works for both real and complex matrix equations. Let
$\mathbb{M}$ be the matrix, $\left\|\mathbb{M}\right\|_{\infty ,\text{bwod}}$ the
[block-wise off-diagonal infinite norm](#block-wise-off-diagonal-infinite-matrix-norm) of the
matrix.

1. Set $\epsilon \gets \text{perturbation_threshold} * \left\|\mathbb{M}\right\|_{\text{bwod}}$
2. If $|\text{pivot_element}| \lt \epsilon$, then:
   1. If $|\text{pivot_element}| = 0$, then:
      1. Set $\text{phase_shift} = 1$.
      2. Proceed.
   2. Else:
      1. Set $\text{phase_shift}\left(\text{pivot_element}\right) \gets \text{pivot_element} / |\text{pivot_element}|$.
      2. Proceed.
   3. Set $\text{pivot_element} \gets \epsilon * \text{phase_shift}\left(\text{pivot_element}\right)$.

$\text{phase_shift}$ ensures that the complex phase of the pivot element is preserved, with a fallback
to the positive real axis when the pivot element is identically zero.

### Iterative refinement of LU-solver solutions

This algorithm is heavily inspired by the GESP algorithm described in
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761).

[LU solving](#block-sparse-lu-solving) with an [LU-decomposition](#block-sparse-lu-factorization)
obtained using [pivot perturbation](#pivot-perturbation) yields only approximate results, because
the LU-decomposition itself is only approximate. Because of that, an iterative refinement process
is required to improve the solution to the matrix equation $\mathbb{M} \cdot \boldsymbol{x} = \boldsymbol{b}$.

The iterative refinement process is as follows: in iteration step $i$, it assumes an existing
approximation $\boldsymbol{x}_i$ for $\boldsymbol{x}$. It then defines the difference between the
current best and the actual solution $\boldsymbol{\Delta x} = \boldsymbol{x} - \boldsymbol{x}_i$.
Substiting in the original equation yields
$\mathbb{M} \cdot (\boldsymbol{x}_i + \boldsymbol{\Delta x}) = \boldsymbol{b}$, so that
$\mathbb{M} \cdot \boldsymbol{\Delta x} = \boldsymbol{b} - \mathbb{M} \cdot \boldsymbol{x}_i =: \boldsymbol{r}$, where the residual
$\boldsymbol{r}$ can be calculated. An estimation for the left-hand side can be obtained by using
the pivot-perturbed matrix $\tilde{A}$ instead of the original matrix A. Convergence can be reached
if $\boldsymbol{r} \to \boldsymbol{0}$, since then also
$\left\|\boldsymbol{\Delta x}\right\| \to 0$. Solving for $\boldsymbol{\Delta x}$ and substituting
back into $\boldsymbol{x}_{i+1} = \boldsymbol{x}_i + \boldsymbol{\Delta x}$ provides the next best
approximation $\boldsymbol{x}_{i+1}$ for $\boldsymbol{x}$.

A measure for the quality of the approximation is given by the $\text{backward_error}$ (see also
[backward error formula](#backward-error-calculation)).

Since the matrix $\mathbb{M}$ does not change during this process, the LU-decomposition remains
valid throughout the process, so that this iterative refinement can be done at a reasonably low
cost.

Given the original matrix equation $\mathbb{M} \cdot \boldsymbol{x} = \boldsymbol{b}$ to solve, the
pivot perturbated matrix $\tilde{\mathbb{M}}$ with a pre-calculated LU-decomposition, and the
convergence threshold $\epsilon$, the algorithm is as follows:

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
         1. Convergence not reached; iterative refinement not possible: raise a sparse matrix
            error.
      3. Else:
         1. Increase the number of iterations.
         2. Proceed.
   2. Solve $\tilde{\mathbb{M}} \cdot \boldsymbol{\Delta x} = \boldsymbol{r}$ for
      $\boldsymbol{\Delta x}$.
   3. Calculate the backward error with the original $\boldsymbol{x}$ and $\boldsymbol{r}$ using the
      [backward error formula](#improved-backward-error-calculation).
   4. Set the next estimation of
      $\boldsymbol{x}$: $\boldsymbol{x}_{\text{est}} \gets \boldsymbol{x}_{\text{est}} + \boldsymbol{\Delta x}$.
   5. Set the residual: $\boldsymbol{r} \gets \boldsymbol{b} - \mathbb{M} \cdot \boldsymbol{x}$.

As a result, we effectively always do:

1. A solve step; and
2. At least one iterative refinement step.

The reason a sparse matrix error is raised, and not an iteration diverge error, when the maximum
allowed amount of iterations is exceeded, is the following. It is the iterative refinement of the
matrix equation solution that cannot be solved in the set amount of iterations - not the set of
power system equations. This will only happen when the matrix equation requires iterative refinement
in the first place, which happens only when pivot perturbation is needed, namely in the case of an
ill-conditioned matrix equation. A matrix equation that is both ill-conditioned and not iteratively
refinable is underdetermined and cannot be solved.

#### Backward error calculation

The [iterative refinement algorithm](#iterative-refinement-of-lu-solver-solutions) uses a backward
error as a convergence criterion. We use the following backward error calculation, inspired by
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761),
with a few modifications described [below](#improved-backward-error-calculation).

$$
\begin{align*}
D_{\text{max}} &= \max_i\left\{\left(\left|\mathbb{M}\right|\cdot\left|\boldsymbol{x}\right| + \left|\boldsymbol{b}\right|\right)_i\right\} \\
\text{backward_error} &= \max_i \left\{
   \frac{\left|\boldsymbol{r}\right|_i}{
      \max\left\{
         \left(\left|\mathbb{M}\right|\cdot\left|\boldsymbol{x}\right| + \left|\boldsymbol{b}\right|\right)_i,
         \epsilon_{\text{backward_error}} D_{\text{max}}
      \right\}
   }
\right\}
\end{align*}
$$

$0 \leq \epsilon \leq 1$ is a value that introduces a
[cut-off value to improve stability](#improved-backward-error-calculation) of the algorithm and
should ideally be small.

```{note}
$\epsilon = 10^{-4}$ was experimentally determined to be a reasonably good value on a number of
real-world MV grids.
```

### Differences with literature

There are several differences between our implementation and the ones described in
[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
and
[Schenk06](https://etna.math.kent.edu/volumes/2001-2010/vol23/abstract.php?vol=23&pages=158-179).
They are summarized below.

#### No check for diminishing returns

[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
contains an early-out criterion for the
[iterative refinement](#iterative-refinement-of-lu-solver-solutions) that checks for deminishing
returns in consecutive iterations. It amounts to (in reverse order):

1. If $\text{backward_error} \gt \frac{1}{2}\text{last_backward_error}$, then:
   1. Stop iterative refinement.
2. Else:
   1. Go to next refinement iteration.

In power systems, however, the fact that the matrix may contain elements
[spanning several orders of magnitude](#element-size-properties-of-power-system-equations) may cause
slow convergence far away from the optimum. The diminishing return criterion would cause the
algorithm to exit before the actual solution is found. Multiple refinement iterations may still
yield better results. The power grid model therefore does not stop on deminishing returns. Instead,
a maximum amount of iterations is used in combination with the error tolerance.

#### Improved backward error calculation

In power system equations, the matrix equation $\mathbb{M} \boldsymbol{x} = \boldsymbol{b}$ can be
very unbalanced: some entries in the matrix $\mathbb{M}$ may be very large while others are zero or
very small. The same may be true for the right-hand side of the equation $\boldsymbol{b}$, as well
as its solution $\boldsymbol{x}$. In fact, there may be certain rows $i$ for which both
$\left|\boldsymbol{b}\left[i\right]\right|$ and
$\sum_j \left|\mathbb{M}\left[i,j\right]\right| \left|\boldsymbol{x}\left[j\right]\right|$ are small
and, therefore, their sum is prone to rounding errors, which may be several orders larger than
machine precision.

[Li99](https://www.semanticscholar.org/paper/A-Scalable-Sparse-Direct-Solver-Using-Static-Li-Demmel/7ea1c3360826ad3996f387eeb6d70815e1eb3761)
uses the following backward error in the
[iterative refinement algorithm](#iterative-refinement-of-lu-solver-solutions):

$$
\begin{align*}
\text{backward_error}_{\text{Li}} &= \max_i \frac{\left|\boldsymbol{r}_i\right|}{\sum_j \left|\mathbb{M}_{i,j}\right| \left|\boldsymbol{x}_j\right| + \left|\boldsymbol{b}_i\right|} \\
&= \max_i \frac{\left|\boldsymbol{b}_i - \sum_j \mathbb{M}_{i,j} \boldsymbol{x}_j\right|}{\sum_j \left|\mathbb{M}_{i,j}\right| \left|\boldsymbol{x}_j\right| + \left|\boldsymbol{b}_i\right|} \\
&= \max_i \frac{\left|\boldsymbol{r}_i\right|}{\left(\left|\mathbb{M}\right| \cdot \left|\boldsymbol{x}\right| + \left|\boldsymbol{b}\right|\right)_i}
\end{align*}
$$

In this equation, the symbolic notation $\left|\mathbb{M}\right|$ and $\left|\boldsymbol{x}\right|$
are the matrix and vector with absolute values of the elements of $\mathbb{M}$ and $\boldsymbol{x}$
as elements, i.e., $\left|\mathbb{M}\right|_{i,j} := \left|\mathbb{M}_{i,j}\right|$ and
$\left|\boldsymbol{x}\right|_i := \left|\boldsymbol{x}_i\right|$, as
defined in [Arioli89](https://epubs.siam.org/doi/10.1137/0610013).

Due to aforementioned, this is prone to rounding errors, and a single row with rounding errors may
cause the entire iterative refinement to fail. The power grid model therefore uses a modified
version, in which the denominator is capped to a minimum value, determined by the maximum across all
denominators:

$$
\begin{align*}
D_{\text{max}} &= \max_i\left\{\left(\left|\mathbb{M}\right|\cdot\left|\boldsymbol{x}\right| + \left|\boldsymbol{b}\right|\right)_i\right\} \\
\text{backward_error} &= \max_i \left\{
   \frac{\left|\boldsymbol{r}\right|_i}{
      \max\left\{
         \left(\left|\mathbb{M}\right|\cdot\left|\boldsymbol{x}\right| + \left|\boldsymbol{b}\right|\right)_i,
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
conclusion, $\epsilon$ should be chosen small, but large enough to suppress numerical instabilities.

#### Block-wise off-diagonal infinite matrix norm

For the [pivot perturbation algorithm](#pivot-perturbation-algorithm), a matrix norm is used to
determine the relative size of the current pivot element compared to the rest of the matrix as a
measure for the degree of ill-conditioning. The norm is a variant of the $L_{\infty}$ norm of a
matrix, which we call the block-wise off-diagonal infinite matrix norm
($L_{\infty ,\text{bwod}}$).

Since the power grid model solves the matrix equations using a multi-scale matrix solver (dense
intra-block, block-sparse for the full topological structure of the grid), the norm is also taken on
those same levels, so the calculation of the norm is _block-wise_.

In addition, the diagonal blocks may have much larger elements than the off-diagonal ones, while the
relevant information is contained mostly in the off-diagonal blocks. As a result, the block-diagonal
blocks would undesirably dominate the norm. The power grid model therefore restricts the calculation
of the norm to _off-diagonal_ blocks.

In short, the $L_{\infty ,\text{bwod}}$-norm it is the $L_{\infty}$ norm of the block-sparse matrix
with the $L_{\infty}$ norm of the individual blocks as elements, where the diagonal blocks are
skipped.

##### Block-wise off-diagonal infinite matrix norm algorithm

The algorithm is as follows:

Let $\mathbb{M}\equiv \mathbb{M}\left[0:N, 0:N\right]$ be the $N\times N$-matrix with a block-sparse
structure and $\mathbb{M}\left[i,j\right]$ its block element at (0-based) indices $(i,j)$, where
$i,j = 0..(N-1)$. In turn, let
$\mathbb{M}\left[i,j\right] \equiv \mathbb{M}_{i,j}\left[0:N_{i,j},0:N_{i,j}\right]$ be the dense
block with dimensions $N_i\times N_j$.

1. Set $\text{norm} \gets 0$.
2. Loop over all block-rows: $i = 0..(N-1)$:
   1. Set $\text{row_norm} \gets 0$.
   2. Loop over all block-columns: $j = 0..(N-1)$ (beware of sparse structure):
      1. If $i = j$, then:
         1. Skip this block: continue with the next block-column.
      2. Else, calculate the $L_{\infty}$ norm of the current block and add to the current row norm:
         1. Set the current block: $\mathbb{M}_{i,j} \gets \mathbb{M}\left[i,j\right]$.
         2. Set $\text{block_norm} \gets 0$.
         3. Loop over all rows of the current block: $k = 0..(N_{i,j} - 1)$:
            1. Set $\text{block_row_norm} \gets 0$.
            2. Loop over all columns of the current block: $l = 0..(N_{i,j} - 1)$:
               1. Set $\text{block_row_norm} \gets \text{block_row_norm} + \left\|\mathbb{M}_{i,j}\left[k,l\right]\right\|$.
            3. Calculate the new block norm: set
               $\text{block_norm} \gets \max\left\{\text{block_norm}, \text{block_row_norm}\right\}$.
            4. Continue with the next row of the current block.
         4. Set $\text{row_norm} \gets \text{row_norm} + \text{block_norm}$.
         5. Continue with the next block-column.
   3. Calculate the new norm: set
      $\text{norm} \gets \max\left\{\text{norm}, \text{row_norm}\right\}$.
   4. Continue with the next block-row.

##### Illustration of the block-wise off-diagonal infinite matrix norm calculation

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
  $\max\left\{\max\left\{1, 3\right\}+\max\left\{3, 0\right\},\max\left\{5, 0\right\} + \max\left\{0, \frac{1}{2}\right\}, 1\right\} = \max\left\{3+3, 5+\frac{1}{2}, 1, 1\right\} = 6$.

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
