<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Dense and selective matrix inverses

An existing LU factorization can be used either to compute every entry of a matrix inverse or only selected entries.
Lowercase italic symbols denote scalars, bold lowercase symbols denote vectors, and bold uppercase symbols denote
matrices, including dense blocks.

## Dense inverse

For a dense matrix $\mathbf{A}=\mathbf{L}\mathbf{U}$, its inverse $\mathbf{X}=\mathbf{A}^{-1}$ satisfies

$$
\mathbf{A}\mathbf{X}=\mathbf{I}.
$$

Substituting the LU factorization gives

$$
\mathbf{L}\mathbf{U}\mathbf{X}=\mathbf{I}.
$$

Introduce $\mathbf{Y}=\mathbf{U}\mathbf{X}$. First, solve the lower-triangular system by forward substitution:

$$
\mathbf{L}\mathbf{Y}=\mathbf{I}
\quad\Longrightarrow\quad
\mathbf{Y}=\mathbf{L}^{-1}.
$$

Then solve the upper-triangular system by backward substitution:

$$
\mathbf{U}\mathbf{X}=\mathbf{Y}
\quad\Longrightarrow\quad
\mathbf{X}=\mathbf{U}^{-1}\mathbf{L}^{-1}=\mathbf{A}^{-1}.
$$

For every column $\mathbf{e}_j$ of $\mathbf{I}$, this means first solving
$\mathbf{L}\mathbf{y}_j=\mathbf{e}_j$ by forward substitution and then
$\mathbf{U}\mathbf{x}_j=\mathbf{y}_j$ by backward substitution. The resulting columns $\mathbf{x}_j$ form the dense
inverse $\mathbf{X}$. In the block-sparse algorithm, the same dense procedure is applied locally to obtain the diagonal
contribution $(\mathbf{L}_p\mathbf{U}_p)^{-1}$ of each pivot block. The Takahashi equations below avoid applying it to
the complete sparse matrix by computing only inverse entries in the filled LU pattern.

## Selective inverse

The selective inverse computes only the entries of $\mathbf{A}^{-1}$ that belong to the filled sparsity pattern of an
existing LU factorization. It uses the Takahashi equations, introduced by Takahashi[^takahashi1973] and further
described by Erisman and Tinney,[^erisman1975].

The sections below derive the Takahashi equations for scalar-sparse and block-sparse matrices, then describe their
numerical implementation in PGM.

### Scalar-sparse matrices

Let $\mathbf{A}=\mathbf{L}\mathbf{U}$, where $\mathbf{L}$ is unit lower triangular and $\mathbf{U}$ is upper triangular,
and define

$$
\mathbf{Z}=\mathbf{A}^{-1}=\mathbf{U}^{-1}\mathbf{L}^{-1}.
$$

The identities

$$
\mathbf{U}\mathbf{Z}=\mathbf{L}^{-1}, \qquad
\mathbf{Z}\mathbf{L}=\mathbf{U}^{-1}
$$

give the inverse entries at pivot $p$. Because $(\mathbf{L}^{-1})_{pj}=0$ for $j>p$,

$$
u_{pp}z_{pj} + \sum_{m>p}u_{pm}z_{mj}=0
\quad\Longrightarrow\quad
z_{pj}=-\frac{1}{u_{pp}}\sum_{m>p}u_{pm}z_{mj}.
$$

Similarly, $(\mathbf{U}^{-1})_{kp}=0$ for $k>p$ and $l_{pp}=1$, hence

$$
z_{kp}+\sum_{m>p}z_{km}l_{mp}=0
\quad\Longrightarrow\quad
z_{kp}=-\sum_{m>p}z_{km}l_{mp}.
$$

Finally, $(\mathbf{L}^{-1})_{pp}=1$ gives

$$
u_{pp}z_{pp}+\sum_{m>p}u_{pm}z_{mp}=1
\quad\Longrightarrow\quad
z_{pp}=\frac{1}{u_{pp}}-\frac{1}{u_{pp}}\sum_{m>p}u_{pm}z_{mp}.
$$

Intuitively, computing entries in the upper or lower triangular part of $\mathbf{Z}$ requires already-computed entries
$z_{mj}$ below them or $z_{km}$ to their right, respectively, where $m>p$. We refer to these dependencies as "looking
down" and "looking right."

These equations are evaluated for $p=n-1,\ldots,0$. The sums only visit structurally non-zero entries in the filled LU
pattern.

### Block-sparse matrices

Now let every sparse entry be a dense block. At block pivot $p$, the packed diagonal block contains a unit lower
triangular factor $\mathbf{L}_p$ and an upper triangular factor $\mathbf{U}_p$. Off-diagonal factor blocks are denoted
by $\mathbf{L}_{mp}$ and $\mathbf{U}_{pm}$. The inverse is again
$\mathbf{Z}=\mathbf{U}^{-1}\mathbf{L}^{-1}$, so the identities

$$
\mathbf{U}\mathbf{Z}=\mathbf{L}^{-1}, \qquad
\mathbf{Z}\mathbf{L}=\mathbf{U}^{-1}
$$

can be expanded block by block.

Because $(\mathbf{L}^{-1})_{pj}=\mathbf{0}$ for $j>p$,

$$
\mathbf{U}_p\mathbf{Z}_{pj}+\sum_{m>p}\mathbf{U}_{pm}\mathbf{Z}_{mj}=\mathbf{0}
\quad\Longrightarrow\quad
\mathbf{Z}_{pj}=-\mathbf{U}_p^{-1}\sum_{m>p}\mathbf{U}_{pm}\mathbf{Z}_{mj}.
$$

Similarly, $(\mathbf{U}^{-1})_{kp}=\mathbf{0}$ for $k>p$, hence

$$
\mathbf{Z}_{kp}\mathbf{L}_p+\sum_{m>p}\mathbf{Z}_{km}\mathbf{L}_{mp}=\mathbf{0}
\quad\Longrightarrow\quad
\mathbf{Z}_{kp}=-\left(\sum_{m>p}\mathbf{Z}_{km}\mathbf{L}_{mp}\right)\mathbf{L}_p^{-1}.
$$

Finally, $(\mathbf{L}^{-1})_{pp}=\mathbf{L}_p^{-1}$ gives

$$
\mathbf{U}_p\mathbf{Z}_{pp}+\sum_{m>p}\mathbf{U}_{pm}\mathbf{Z}_{mp}=\mathbf{L}_p^{-1}.
$$

Multiplying from the left by $\mathbf{U}_p^{-1}$ yields

$$
\mathbf{Z}_{pp}=(\mathbf{L}_p\mathbf{U}_p)^{-1}
-\mathbf{U}_p^{-1}\sum_{m>p}\mathbf{U}_{pm}\mathbf{Z}_{mp}.
$$

These are the scalar equations with division replaced by dense triangular solves. Multiplication order must be
preserved: $\mathbf{U}_p^{-1}$ acts from the left, while $\mathbf{L}_p^{-1}$ acts from the right. For block size one,
$\mathbf{L}_p=[1]$ and $\mathbf{U}_p=[u_{pp}]$, so the block equations reduce exactly to the scalar equations.

### Numerical implementation

#### Dependency blocks and target blocks

For power-grid use cases, the target blocks are often only those in the original `y_bus` pattern. In radial networks,
the LU pattern and the original `y_bus` pattern are the same, so dependency blocks and target blocks coincide. In meshed
networks, factorization introduces fill-ins. The dependency block set is then the complete filled LU pattern and is
larger than the target block set.

A fill-in cannot be skipped merely because it is not part of the final `y_bus` target pattern; target blocks are
extracted only after all pivot updates have completed.

#### Reverse pivot traversal

Each update at pivot $p$ refers only to inverse blocks whose row and column indices are greater than $p$. The sweep
therefore starts at the last pivot, where all off-diagonal sums are empty, and proceeds in reverse order,
$p=n-1,\ldots,0$. When pivot $p$ is reached, the trailing inverse blocks $\mathbf{Z}_{km}$ with $k,m>p$ are already
final and will not be modified again.

#### Pivot step order

With the reverse-traversal invariant, each pivot step performs the following operations:

1. Buffer the packed pivot block, the original $\mathbf{U}_{pm}$ blocks, and the original $\mathbf{L}_{mp}$
   blocks.
2. Compute the lower blocks $\mathbf{Z}_{kp}$.
3. Compute the upper blocks $\mathbf{Z}_{pj}$.
4. Compute $\mathbf{Z}_{pp}$ last, using the newly computed lower blocks.

This order permits the LU storage to be overwritten in place without losing values needed by the current step.

#### Why row/column buffers are needed

At pivot $p$, the sparse data array still contains the original $\mathbf{U}_{pm}$ blocks in row $p$ and the original
$\mathbf{L}_{mp}$ blocks in column $p$. The pivot step overwrites those same locations with $\mathbf{Z}_{pj}$ and
$\mathbf{Z}_{kp}$. However, every row and column update needs the complete original pivot row or column. Without
buffers, an early update could make a later sum read an inverse block where an LU factor block is required.

The implementation therefore copies the off-diagonal pivot row and column before updating either one. Column entries
are scattered in the row-oriented sparse storage, so their data indices are buffered as well. The diagonal update can
then use the buffered $\mathbf{U}_{pm}$ blocks together with the newly computed $\mathbf{Z}_{mp}$ blocks. This requires
temporary storage only for the active pivot row and column, while the full sparse inverse remains in place.

#### Pivot block lifetime

The packed pivot block stores $\mathbf{L}_p$ below the diagonal and $\mathbf{U}_p$ on and above the diagonal. A
factorized copy must remain available for the entire pivot step because the lower, upper, and diagonal updates all
use it. Only after those operations are complete may the stored pivot location be replaced by $\mathbf{Z}_{pp}$.

#### Why the pivot must stay factorized

Overwriting $\mathbf{U}_p$ with its inverse would destroy the upper factor in the packed pivot representation. The
remaining row and column updates would then no longer have the factors required for their triangular solves or for the
local diagonal contribution $(\mathbf{L}_p\mathbf{U}_p)^{-1}$. Keeping one factorized pivot copy alive avoids this
dependency while still allowing the sparse data array to be overwritten in place.

#### Restoring dense block permutations

The sparse LU solver in PGM uses dense full pivoting inside each block. It therefore produces the factorization

$$
\mathbf{P}\mathbf{A}\mathbf{Q}=\mathbf{L}\mathbf{U},
$$

where $\mathbf{P}=\operatorname{blockdiag}(\mathbf{P}_i)$ and
$\mathbf{Q}=\operatorname{blockdiag}(\mathbf{Q}_i)$. The selective inverse sweep operates directly on the stored
$\mathbf{L}$ and $\mathbf{U}$ factors and computes

$$
\mathbf{Z}=(\mathbf{L}\mathbf{U})^{-1}
=(\mathbf{P}\mathbf{A}\mathbf{Q})^{-1}
=\mathbf{U}^{-1}\mathbf{L}^{-1}
$$

on the filled pattern. This is the inverse in the permuted ordering, not yet $\mathbf{A}^{-1}$. Rearranging the
factorization shows that the original ordering is restored by applying $\mathbf{Q}$ from the left and $\mathbf{P}$ from
the right:

$$
\mathbf{A}^{-1}=\mathbf{Q}\mathbf{Z}\mathbf{P}.
$$

Because $\mathbf{P}$ and $\mathbf{Q}$ are block diagonal, this can be done independently for every stored block:

$$
(\mathbf{A}^{-1})_{ij}=\mathbf{Q}_i\mathbf{Z}_{ij}\mathbf{P}_j.
$$

[^takahashi1973]: K. Takahashi, “Formation of sparse bus impedance matrix and its application to short circuit
    study,” in *Proceedings of the PICA Conference*, June 1973.

[^erisman1975]: A. M. Erisman and W. F. Tinney, “On computing certain elements of the inverse of a sparse matrix,”
    *Communications of the ACM*, vol. 18, no. 3, pp. 177–179, 1975.
