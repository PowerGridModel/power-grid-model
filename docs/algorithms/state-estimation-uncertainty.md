<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# State estimation uncertainty

State estimation by default returns deterministic state estimates.
The math solver can also report how uncertain that estimate is, with the variance of the voltage magnitude and voltage angle
of requested buses.
This is available for [iterative linear](se-algorithms.md#iterative-linear-state-estimation) (ILSE) and
[Newton-Raphson](se-algorithms.md#newton-raphson-state-estimation) (NRSE) state estimation, in symmetric and asymmetric
calculations.

```{note}
For now, this is an internal interface of the math solver.
It is not exposed in the C API or the Python API.
```

After the estimate has converged, the calculation takes three steps:

1. Obtain the LU factors of the augmented matrix that state estimation solves.
2. Compute the blocks of its inverse on the LU sparsity pattern. Their state part is the state covariance.
3. Convert the covariance block of each requested bus into a magnitude and an angle variance.


## Augmented system

Let $x$ be the state: the complex bus voltages $U$ for ILSE, and the polar coordinates $[\theta, v]$ with
$U = v e^{j\theta}$ for NRSE.
State estimation treats two groups of measurements differently.
Voltage and flow measurements have the measurement matrix $F_b$ and the weights $W_b = \Sigma_b^{-1}$.
Nodal injection measurements have the measurement matrix $F_g$ and the variances $\Sigma_g$, where a zero variance is for
a zero-injection constraint.
State estimation solves a linear system with the augmented matrix:

$$
A = \begin{bmatrix}
G & Q^H \\
Q & R
\end{bmatrix},
\qquad
G = F_b^H W_b F_b, \quad Q = F_g, \quad R = -\Sigma_g.
$$

For ILSE the measurement matrices are constant.
For NRSE they are the Jacobians of the real measurement equations at the estimate, and $Q^H$ becomes a real transpose.

### Variance normalization

To keep the matrix well scaled, state estimation divides every sensor variance by a common factor $\alpha$:

$$
\bar{\sigma}^2 = \frac{\sigma^2}{\alpha}.
$$

$\alpha$ is the smallest nonzero variance of the measurements.

A bar marks a quantity built from normalized variances.
The same symbol without a bar is its counterpart in physical units.

Normalization multiplies every weight by $\alpha$ and divides every variance by $\alpha$.
Then the scaled augmented matrix is:

$$
\bar{A}
= \begin{bmatrix}
\bar{G} & Q^H \\
Q & \bar{R}
\end{bmatrix}
= \begin{bmatrix}
\alpha G & Q^H \\
Q & R / \alpha
\end{bmatrix}
= D A D,
\qquad
D = \begin{bmatrix}
\sqrt{\alpha} \thinspace I & 0 \\
0 & I / \sqrt{\alpha}
\end{bmatrix}.
$$

The estimate does not depend on $\alpha$, but the inverse does: $\bar{A}^{-1} = D^{-1} A^{-1} D^{-1}$.

A bus without an injection measurement has a zero row in $Q$ and a placeholder $-1$ on the diagonal of $\bar{R}$.


## State covariance

Propagating the sensor errors through the augmented system gives the covariance of the estimate as the state rows and
columns of the inverse:

$$
C_x = \mathrm{Cov}(\hat{x}) = \lbrack A^{-1} \rbrack_{xx} = \alpha \lbrack \bar{A}^{-1} \rbrack_{xx}.
$$

When all injection variances are positive, this equals the inverse $(F^H W F)^{-1}$ of the gain matrix of the
[weighted least squares formulation](se-algorithms.md#weighted-least-squares-formulation), with $F$ and $W$ covering
both groups of measurements.
Unlike that inverse, it also exists with exact injection constraints, and it includes their effect.
For ILSE, $C_x = V$ is the covariance of the complex voltages.
For NRSE, it is the covariance of the angles and magnitudes.

## Selective inversion

PGM does not construct $\bar{A}$, but its perfect-shuffle $\bar A_{\mathrm{ps}}$.
The perfect-shuffle permutation $\Pi$ places the state coordinates and the multipliers of each bus next to each other:

$$
\bar A_{\mathrm{ps}} = \Pi \bar{A} \Pi^T,
\qquad
\bar{Z} = \bar A_{\mathrm{ps}}^{-1}.
$$

After the shuffle, the unknowns of bus $i$ form one group: its state coordinates $x_i$ first, then the multipliers
$\phi_i$ of its injection equations.

|                | Unknowns of bus $i$, in order              | Symmetric | Asymmetric |
| -------------- | ------------------------------------------ | --------- | ---------- |
| ILSE (complex) | $U_i,\ \phi_i$                             | 2         | 6          |
| NRSE (real)    | $\theta_i,\ v_i,\ \phi_{P,i},\ \phi_{Q,i}$ | 4         | 12         |

In an asymmetric calculation every listed unknown has three values, one per phase.
The group keeps the order of the table and expands each unknown into its phases $a, b, c$:

- **ILSE:** $U_a, U_b, U_c, \phi_a, \phi_b, \phi_c$
- **NRSE:** $\theta_a, \theta_b, \theta_c, v_a, v_b, v_c, \phi_{P,a}, \phi_{P,b}, \phi_{P,c},
  \phi_{Q,a}, \phi_{Q,b}, \phi_{Q,c}$

Both $\bar A_{\mathrm{ps}}$ and $\bar{Z}$ are therefore matrices of bus blocks.
The block $\bar Z_{ij}$ is the part of $\bar{Z}$ in the rows of bus $i$ and the columns of bus $j$.
Rows and columns both follow the order of the group, so the block splits into a state part and a multiplier part:

$$
\bar Z_{ij}
= \begin{bmatrix}
\lbrack \bar Z_{ij} \rbrack_{xx} & \lbrack \bar Z_{ij} \rbrack_{x\phi} \\
\lbrack \bar Z_{ij} \rbrack_{\phi x} & \lbrack \bar Z_{ij} \rbrack_{\phi\phi}
\end{bmatrix}.
$$

For symmetric NRSE this is a $4 \times 4$ block with a $2 \times 2$ state part.
The covariance between buses $i$ and $j$ is that state part:

$$
C_{x,ij} = \alpha \lbrack \bar Z_{ij} \rbrack_{xx}.
$$

The two estimators label this block differently:

- **ILSE:** $C_x = V$, so the block is written $V_{ij}$.
  It is the covariance between the complex voltages of buses $i$ and $j$.
- **NRSE:** the block keeps the name $C_{x,ij}$, and its entries are labelled by the state coordinates.
  $\lbrack C_{x,ij} \rbrack_{\theta\theta}$ is the covariance between the angles of buses $i$ and $j$,
  $\lbrack C_{x,ij} \rbrack_{vv}$ the one between their magnitudes, and $\lbrack C_{x,ij} \rbrack_{\theta v}$
  the one between the angle of bus $i$ and the magnitude of bus $j$.

The [block selective inverse](dense-and-selective-inverse.md#block-sparse-matrices) replaces the LU factors of
$\bar A_{\mathrm{ps}}$ by the blocks $\bar Z_{ij}$ on the LU sparsity pattern.
A bus voltage needs only the diagonal block $C_{x,ii}$ of its own bus.

The LU factors come from the estimator:

- **ILSE:** the matrix does not depend on the state, so the factors of the estimate are reused.
  They are copied first, because the inversion overwrites them.
- **NRSE:** the factors of the last iteration precede the final update of the state.
  The matrix is therefore assembled once more at the returned voltages, with $\theta = \arg U$ and $v = |U|$, and
  factorized.

The factors must be free of [pivot perturbation](lu-solver.md#pivot-perturbation), otherwise the uncertainty is
reported as invalid. The handling of pivot perturbation will come later.


## Voltage variances


For NRSE, angle and magnitude are state coordinates, so their variances are diagonal entries of $C_{x,ii}$:

$$
\mathrm{Var}(\theta_i) = \lbrack C_{x,ii} \rbrack_{\theta\theta},
\qquad
\mathrm{Var}(|U_i|) = \lbrack C_{x,ii} \rbrack_{vv}.
$$

For ILSE, the error of the complex voltage is assumed as proper (zero pseudo-covariance).
Its radial and tangential components then each carry half of the variance $V_{ii}$:

$$
\mathrm{Var}(|U_i|) = \frac{1}{2} \thinspace \mathrm{Re}(V_{ii}),
\qquad
\mathrm{Var}(\theta_i) = \frac{\mathrm{Re}(V_{ii})}{2 |U_i|^2}.
$$


### Angle reference

If the island has a voltage angle measurement, angles are absolute and the variances above are reported.

Otherwise the measurements leave a common rotation of all voltage angles of the island undetermined.
State estimation then reports every angle relative to phase $a$ of the slack bus $s$.
It subtracts the estimated angle $\theta_s$ of that phase from all voltage angles, so the reference angle becomes zero.
The uncertainty has to describe these relative angles, so it goes through the same change of reference.

In polar coordinates this subtraction is a linear transformation.
Collect the angles $\theta$ and the magnitudes $v$ of all bus voltages in one vector $y$, and define two vectors of the
same size:

$$
y = \begin{bmatrix}
\theta \\
v
\end{bmatrix},
\qquad
n = \begin{bmatrix}
\mathbf{1} \\
\mathbf{0}
\end{bmatrix},
\qquad
e_s^T y = \theta_s.
$$

$n$ has a one for every angle and a zero for every magnitude.
$e_s$ is the unit vector that selects the reference angle.
Subtracting $\theta_s$ from every angle is then

$$
y^r = y - n \thinspace \theta_s = \left(I - n \thinspace e_s^T\right) y = P y,
\qquad
P = I - n \thinspace e_s^T.
$$

Every angle becomes $\theta_i^r = \theta_i - \theta_s$ and every magnitude stays as it is.
The covariance follows the same transformation:

$$
C_y^r = P \thinspace C_y P^T,
$$

where $C_y$ is the covariance before the change of reference.

Magnitude variances do not change, and an angle variance becomes

$$
\mathrm{Var}(\theta_i^r)
= \mathrm{Var}(\theta_i) + \mathrm{Var}(\theta_s) - 2 \thinspace \mathrm{Cov}(\theta_i, \theta_s).
$$

The variance of the reference angle has a different value before and after the change of reference:

| Quantity                         | Meaning                                           | Value              |
| -------------------------------- | ------------------------------------------------- | ------------------ |
| $\mathrm{Var}(\theta_s)$   | Before the change of reference, an entry of $C_y$ | Nonzero in general |
| $\mathrm{Var}(\theta_s^r)$ | As reported, an entry of $C_y^r$                  | Exactly zero       |

Before the change of reference, $\theta_s$ is an estimate like any other angle.
Its variance and its covariance with $\theta_i$ therefore enter the reported variance of every other angle.
After it, $\theta_s^r = \theta_s - \theta_s = 0$ whatever the estimate is, so the reported reference angle has no
uncertainty. Phases $b$ and $c$ of the slack bus keep a nonzero reported variance.

Because $P n = 0$, an uncertainty that is common to all angles drops out, and the reported variances describe angle
differences only.

PGM does not construct $C_y^r$.
It evaluates the three terms for each requested angle, and reports the zero of the reference angle directly.
$\mathrm{Cov}(\theta_i, \theta_s)$ is the angle entry of $C_{x,is}$ for NRSE and
$\frac{1}{2} \thinspace \mathrm{Re}\left(V_{is} / (U_i U_s^\ast)\right)$ for ILSE.

The block $C_{x,is}$ is in general not on the LU sparsity pattern, so the selective inverse does not provide it.
PGM obtains the entries it needs from one additional linear solve with $\bar A_{\mathrm{ps}}$.
The right-hand side is the unit vector of the state coordinate of phase $a$ of the slack bus: its angle for NRSE and
its complex voltage for ILSE.
The solution is the matching column of $\bar{Z}$.
Multiplied by $\alpha$, its state part at bus $i$ is the column of $C_{x,is}$ for that coordinate, for every bus $i$
at once.
The solve needs the LU factors, so it runs before the selective inversion overwrites them.
The [appendix](#appendix-angle-reference-transformation) derives these formulas.

## Request and result

The request and the result are passed to `run_state_estimation` in an optional `StateEstimationUQContext`.
A request lists buses by their index in the island and selects magnitude, angle or both.
The result has one row per request entry, in request order, with one value per phase.
It is written only after both the estimate and its uncertainty are complete.
A bus index out of range is rejected before solving, and a failed estimate leaves the result untouched.

Each value holds the signed variance $v$ (`raw_variance`), the sum $a$ of the magnitudes of the terms that were added
to obtain it (`absolute_scale`), the standard deviation `sigma` and a status.
With $\varepsilon$ the machine epsilon of `double`:

| Status             | Condition                                                        | `sigma`    |
| ------------------ | ---------------------------------------------------------------- | ---------- |
| `valid`            | $v \ge 0$                                                        | $\sqrt{v}$ |
| `clipped_roundoff` | $-1000 \varepsilon a \le v < 0$, attributed to cancellation      | $0$        |
| `invalid`          | $v < -1000 \varepsilon a$, or nonfinite                          | NaN        |
| `not_requested`    | the quantity was not selected                                    | NaN        |

If the LU factors are not available, because the factorization failed or needed pivot perturbation, the estimate is
still returned and every requested value is `invalid`.


## Appendix: angle reference transformation

This appendix derives the formulas of [Angle reference](#angle-reference).
Below, $\delta q = \hat{q} - q$ is the error of an estimated quantity $q$.
To first order it has zero mean, so $\mathrm{Cov}(q, r) = \mathrm{E}[\delta q \thinspace \delta r]$ for real $q$
and $r$.

### The transformation

State estimation multiplies every voltage by $e^{-j \theta_s}$, with $\theta_s$ the estimated angle of phase $a$ of
the slack bus.
This subtracts $\theta_s$ from every angle and leaves every magnitude unchanged, which is $y^r = P y$ with
$P = I - n \thinspace e_s^T$.
Because the reference angle is one of the angles, $e_s^T n = 1$.
Three properties follow:

$$
P n = n - n \left(e_s^T n\right) = 0,
\qquad
e_s^T P = e_s^T - \left(e_s^T n\right) e_s^T = 0,
\qquad
P^2 = P.
$$

The first says that a common shift of all angles does not change $y^r$.
The second says that the reference angle is zero in $y^r$, whatever $y$ is.
The third says that $P$ is a projection: it cannot be inverted, because the common angle is discarded.

### The transformed covariance

The transformation is linear, so the errors transform in the same way, $\delta y^r = P \thinspace \delta y$, and

$$
C_y^r
= \mathrm{E}\left[\delta y^r \left(\delta y^r\right)^T\right]
= P \thinspace \mathrm{E}\left[\delta y \thinspace \delta y^T\right] P^T
= P \thinspace C_y P^T.
$$

Written per entry, with $\delta\theta_i^r = \delta\theta_i - \delta\theta_s$ and $\delta v_i^r = \delta v_i$:

$$
\begin{aligned}
\mathrm{Cov}(\theta_i^r, \theta_j^r)
&= \mathrm{E}\left[(\delta\theta_i - \delta\theta_s)(\delta\theta_j - \delta\theta_s)\right] \\
&= \mathrm{Cov}(\theta_i, \theta_j) - \mathrm{Cov}(\theta_i, \theta_s) -
   \mathrm{Cov}(\theta_s, \theta_j) + \mathrm{Var}(\theta_s), \\
\mathrm{Cov}(v_i^r, \theta_j^r)
&= \mathrm{Cov}(v_i, \theta_j) - \mathrm{Cov}(v_i, \theta_s), \\
\mathrm{Cov}(v_i^r, v_j^r)
&= \mathrm{Cov}(v_i, v_j).
\end{aligned}
$$

Setting $j = i$ in the first line gives the reported angle variance:

$$
\mathrm{Var}(\theta_i^r)
= \mathrm{Var}(\theta_i) + \mathrm{Var}(\theta_s) - 2 \thinspace \mathrm{Cov}(\theta_i, \theta_s).
$$

For $i = s$ the covariance equals $\mathrm{Var}(\theta_s)$ and the three terms cancel.
Setting only the variance of the reference angle to zero would not be enough, because the last two terms also change
every other angle.

### Angle covariance for NRSE

Both angles are state coordinates, so $C_y = C_x$.
Their covariance is therefore an entry of the state covariance, in the block of buses $i$ and $s$:

$$
\mathrm{Cov}(\theta_i, \theta_s) = \lbrack C_{x,is} \rbrack_{\theta\theta}.
$$

### Angle covariance for ILSE

For ILSE the state is the complex voltage.
The angle is not a state coordinate, so its covariance has to be derived from $V$.
The quantity wanted is

$$
\mathrm{Cov}(\theta_i, \theta_s) = \mathrm{E}\left[\delta\theta_i \thinspace \delta\theta_s\right],
$$

and two facts about the voltage errors are known:

$$
\mathrm{E}\left[\delta U_i \thinspace \delta U_s^\ast\right] = V_{is},
\qquad
\mathrm{E}\left[\delta U_i \thinspace \delta U_s\right] = 0.
$$

The first is the definition of the entry $V_{is}$.
The second is the assumption that the error is proper.
The derivation writes the angle errors in terms of the voltage errors, and then applies these two facts.

**Step 1: the angle error in terms of the voltage error.**
Write the voltage in polar form, $U_i = v_i e^{j\theta_i}$.
A small error $\delta v_i$ of the magnitude and $\delta\theta_i$ of the angle changes the voltage, to first order, by

$$
\delta U_i
= e^{j\theta_i} \thinspace \delta v_i + j v_i e^{j\theta_i} \thinspace \delta\theta_i
= U_i \left(\frac{\delta v_i}{v_i} + j \thinspace \delta\theta_i\right).
$$

Dividing by $U_i$ gives the relative voltage error $w_i$.
Its real part is the relative magnitude error and its imaginary part is the angle error:

$$
w_i = \frac{\delta U_i}{U_i} = \frac{\delta v_i}{v_i} + j \thinspace \delta\theta_i,
\qquad
\delta\theta_i = \mathrm{Im}(w_i).
$$

**Step 2: the product of two angle errors.**
With $\mathrm{Im}(w) = (w - w^\ast) / (2j)$ and $z + z^\ast = 2 \thinspace \mathrm{Re}(z)$:

$$
\begin{aligned}
\delta\theta_i \thinspace \delta\theta_s
= \mathrm{Im}(w_i) \thinspace \mathrm{Im}(w_s)
&= -\frac{1}{4} \left(w_i - w_i^\ast\right) \left(w_s - w_s^\ast\right) \\
&= \frac{1}{4} \left(w_i w_s^\ast + w_i^\ast w_s\right) - \frac{1}{4} \left(w_i w_s + w_i^\ast w_s^\ast\right) \\
&= \frac{1}{2} \thinspace \mathrm{Re}\left(w_i w_s^\ast\right) - \frac{1}{2} \thinspace \mathrm{Re}\left(w_i w_s\right).
\end{aligned}
$$

The product is now a sum of two terms: one with a conjugate and one without.

**Step 3: the expectation of both terms.**
The voltages $U_i$ and $U_s$ are fixed numbers, taken at the estimate; only the errors are random.
They therefore come out of the expectation, and the two known facts apply:

$$
\mathrm{E}\left[w_i w_s^\ast\right]
= \frac{\mathrm{E}\left[\delta U_i \thinspace \delta U_s^\ast\right]}{U_i U_s^\ast}
= \frac{V_{is}}{U_i U_s^\ast},
\qquad
\mathrm{E}\left[w_i w_s\right]
= \frac{\mathrm{E}\left[\delta U_i \thinspace \delta U_s\right]}{U_i U_s}
= 0.
$$

**Result.**
The covariance is the expectation of the product of step 2.
Inserting step 3 gives

$$
\begin{aligned}
\mathrm{Cov}(\theta_i, \theta_s)
&= \mathrm{E}\left[\delta\theta_i \thinspace \delta\theta_s\right] \\
&= \frac{1}{2} \thinspace \mathrm{Re}\left(\mathrm{E}\left[w_i w_s^\ast\right]\right) -
   \frac{1}{2} \thinspace \mathrm{Re}\left(\mathrm{E}\left[w_i w_s\right]\right) \\
&= \frac{1}{2} \thinspace \mathrm{Re}\left(\frac{V_{is}}{U_i U_s^\ast}\right).
\end{aligned}
$$

For $s = i$ this reduces to the angle variance $\mathrm{Re}(V_{ii}) / (2 |U_i|^2)$ of
[Voltage variances](#voltage-variances).
