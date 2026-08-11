<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Grid Component Modeling

```{note}
This document is about components not explicitly supported by PGM that can still be modeled using PGM components.
For documentation on the components supported by PGM, please refer to [Components](./components.md).
```

The power grid consists of many different component types and the list is ever-increasing.
As a result, it would be an impossible task to make the list of components supported by PGM exhaustive.
Moreover, many grid component types share electrical attributes, so that one such grid component may be modeled as
another.
This document contains a non-exhaustive list of such modeling examples.
New contributions and ideas for modeling grid components are very welcome!

## Ideal transformer

An ideal transformer can be modeled as a [link](./components.md#link). <!-- markdownlint-disable-line descriptive-link-text line-length -->
To this end, it is explicitly allowed to place a link between two different voltage levels.
Trivially, no additional electrical parameters need to be specified.

```{note}
The [transformer](./components.md#transformer) component as provided by the PGM models a real transformer in the grid,
i.e., with finite impedance.
This directly conflicts with the properties of an ideal transformer, so the PGM
[transformer](./components.md#transformer) cannot be used to model ideal transformers.
```

## Grounding transformer

A grounding transformer (a.k.a. earthing transformer) can be modeled as a [shunt](./components.md#shunt) with only
zero-sequence parameters (and positive-sequence parameters set to $0$).
In particular, a grounding transformer with resistance $r$ and reactance $x$ can be modeled as follows.

$$
\begin{aligned}
g_1 &= 0 \\
b_1 &= 0 \\
g_0 &= \Re\left\{\frac{1}{r + j x}\right\} \\
b_0 &= \Im\left\{\frac{1}{r + j x}\right\}
\end{aligned}
$$

## Phase-shifting transformer

A phase-shifting transformer (PST) can be modeled directly with `generic_branch`.
The complex ratio

$$
N = k \cdot e^{\mathrm{j} \theta}
$$

already contains the phase shift;
the series impedance (`r1` and `x1`) and the magnetizing branch (`g1` and `b1`) are given as input for any transformer.
No dedicated PST component is needed.

Note that the angle `theta` must be provided explicitly by user.
For transformers with tap changers, the parameters `k`, `theta`, and `x1` are dynamic.
They must be calculated based on the current tap position, using the voltage magnitude boost and
the phase injection angle defined per tap step.

Let

* $m = \text{tap\_pos} - \text{tap\_neutral}$ be the tap position relative to the neutral position,
* $\Delta u$ be the additional voltage per tap step in per-unit of the winding voltage,
* $\psi$ be the injection angle of the additional voltage relative to the winding voltage
  (e.g., $\psi = 90^{\circ}$ for pure quadrature/phase regulation).

The tap-dependent complex ratio is then

$$
\rho = 1 + m \, \Delta u \, e^{\mathrm{j} \psi}
$$

so that

$$
\begin{aligned}
k_{\text{tap}} &= |\rho| = \sqrt{\left(1 + m \, \Delta u \cos\psi\right)^2
                                 + \left(m \, \Delta u \sin\psi\right)^2} \\
\theta_{\text{tap}} &= \operatorname{atan2}\!\left(m \, \Delta u \sin\psi,\;
                                                  1 + m \, \Delta u \cos\psi\right)
\end{aligned}
$$

The `generic_branch` input attributes `k` and `theta` follow as

$$
\begin{aligned}
k      &= k_{\text{off-nominal}} \cdot k_{\text{tap}} \\
\theta &= \theta_{\text{fixed}} + \theta_{\text{tap}}
\end{aligned}
$$

where $k_{\text{off-nominal}}$ is the off-nominal ratio of the transformer at the neutral tap
position and $\theta_{\text{fixed}}$ is the fixed phase shift of the winding configuration
(e.g., from the clock number).

**Special case: ideal phase-shifting tap changer.**
Some tools (e.g., pandapower with `tap_changer_type="Ideal"`) model tap changers that change only
the phase angle, while the voltage magnitude ratio stays constant.
Note that "ideal" refers to the tap changer, not to a zero-impedance transformer: the series
impedance and the magnetizing branch are still modeled through `r1`, `x1`, `g1`, `b1`.
In this case, `k` and `theta` reduce to

$$
\begin{aligned}
k      &= k_{\text{off-nominal}} \\
\theta &= \theta_{\text{fixed}} + m \cdot \theta_{\text{step}}
\end{aligned}
$$

with $\theta_{\text{step}}$ the angle shift per tap step
(equivalent to `tap_step_degree` in pandapower: converted to radian).

A lossless but not impedance-free PST is obtained with `r1 = 0`, `x1 > 0`, `g1 = 0`, `b1 = 0`.

```{warning}
An impedance-free ideal phase link cannot be represented by setting both `r1` and `x1` to zero. The branch model uses
`1 / (r1 + j x1)` for its series admittance, so zero series impedance would make the model singular.
```

### Mapping from CGMES / ENTSO-E PST models
<!-- markdownlint-disable-next-line MD013 -->
The [ENTSO-E PST modelling document (CGMES v2.4)](https://eepublicdownloads.entsoe.eu/clean-documents/CIM_documents/Grid_Model_CIM/ENTSOE_CGMES_v2.4_28May2014_PSTmodelling.pdf)
describes the standard PST types and their mapping to CIM classes.
All of these types can be represented with `generic_branch`.
In ENTSO-E models, the windings resistance and core magnetizing components are neglected (`r1 = 0`, `g1 = 0`, `b1 = 0`);
consequently, the transformer is defined solely by its reactance (`x1`), tap ratio (`k`),
and phase shift angle (`theta`).
With $m = n - n_0$ (in CIM: `step` − `neutralStep`), $\Delta u$ = `voltageStepIncrement`,
$\theta_{\text{step}}$ = `stepPhaseShiftIncrement` and
$\psi$ = `windingConnectionAngle`, the per-tap values are:

| CIM class                                | angle $\alpha$ per tap                                                                       | magnitude ratio                                                                   |
| ---------------------------------------- | -------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------- |
| `PhaseTapChangerLinear`                  | $\alpha = m \cdot \delta\theta_{\text{step}}$                                                | constant ($k_{\text{off-nominal}}$)                                               |
| `PhaseTapChangerSymmetrical`             | $\alpha = 2 \operatorname{atan}\!\left(\tfrac{m \, \Delta u}{2}\right)$                      | constant ($r = 1$)                                                                |
| `PhaseTapChangerAsymmetrical`            | $\alpha = \operatorname{atan2}\!\left(m \Delta u \sin\psi,\, 1 + m \Delta u \cos\psi\right)$ | $\lvert\rho\rvert = \sqrt{(1 + m \Delta u \cos\psi)^2 + (m \Delta u \sin\psi)^2}$ |
| quadrature booster ($\psi = 90^{\circ}$) | $\alpha = \operatorname{atan}(m \, \Delta u)$                                                | $\lvert\rho\rvert = \sqrt{1 + (m \, \Delta u)^2}$                                 |
| `PhaseTapChangerTabular`                 | $\alpha$ = `angle` per tap (from table)                                                      | `ratio` per tap (from table)                                                      |

The symmetrical phase shifter changes only the angle ($r = 1$); it is therefore the physical
counterpart of the "ideal phase-shifting tap changer" described above.

**Tap-dependent series reactance.**
The ENTSO-E document also specifies that the equivalent series reactance of a PST depends on the
phase shift angle, exchanged in CIM via `xMin` (at neutral tap) and `xMax` (at maximum phase shift
$\alpha_{\max}$):

$$
\begin{aligned}
\text{symmetrical:} \quad
  X(\alpha) &= X_{\min} + \left(X_{\max} - X_{\min}\right)
               \left(\frac{\sin(\alpha/2)}{\sin(\alpha_{\max}/2)}\right)^{2} \\
\text{quadrature booster:} \quad
  X(\alpha) &= X_{\min} + \left(X_{\max} - X_{\min}\right)
               \left(\frac{\tan\alpha}{\tan\alpha_{\max}}\right)^{2} \\
\text{asymmetrical:} \quad
  X(\alpha) &= X_{\min} + \left(X_{\max} - X_{\min}\right)
               \left(\frac{\tan\alpha}{\tan\alpha_{\max}} \cdot
                     \frac{\sin\psi - \tan\alpha_{\max}\cos\psi}
                          {\sin\psi - \tan\alpha\cos\psi}\right)^{2}
\end{aligned}
$$

For `generic_branch` this means that a tap change on a CGMES PST in general requires recalculating
**three** attributes: `k`, `theta` and `x1`.
When `PhaseTapChangerTabular` data is available (recommended by ENTSO-E), `k`, `theta` and `x1` can
be taken directly from the per-tap table columns `ratio`, `angle` and `x` without evaluating the
formulas above.

```{note}
Mind the direction convention: the ENTSO-E document defines the ratio as
$r = 1 / \lvert\rho\rvert$ (referenced from the opposite side).
Depending on which side of the `generic_branch` the tap winding is located, either
$\lvert\rho\rvert$ or its reciprocal must be used for `k`, consistent with the fact that `r1`,
`x1`, `g1`, `b1` are referenced to the "to" side of the branch.
```

```{warning}
A fully impedance-free ideal phase link (`r1 = x1 = 0`) cannot be modeled, because the series
admittance $Y_{\text{series}} = 1 / (r_1 + \mathrm{j} x_1)$ would become singular.
```

```{note}
The attributes `k`, `theta` and `x1` are not updatable.
Changing the tap position of a phase-shifting transformer therefore requires recalculating these
attributes and rebuilding the model.
```

```{warning}
Automatic tap control via the transformer tap regulator is not available for `generic_branch`.
```

A use case of `generic_branch` as PST can be found in
[Generic Branch Example](../examples/Generic%20Branch%20Example.ipynb).

## Choke coil

A choke coil (a.k.a. reactance coil) can be modeled as a [line](./components.md#line) with its shunt parameters
(`c1`, `tan1`, `c0`, `tan0`) set to $0$.

```{note}
A choke coil can also be modeled using a [generic branch](./components.md#generic-branch).
However, at the time of writing (June 2026; see
[PGM issue #739](https://github.com/PowerGridModel/power-grid-model/issues/739)), this component does not yet support
zero-sequence parameters. As a result, asymmetric calculations are not supported, and this model currently only works
for symmetric calculations.
```

## Cable

A cable is equivalent to a line.

## Ideal source

An ideal source  (a.k.a. slack bus) can be modeled using a regular PGM [source](./components.md#source) with a high
`sk`, e.g. $s_k = 10^{50}\,\text{VA}$.

```{warning}
At the time of writing, very high `sk` can result in unresolved infinities.
The value mentioned here is a safe middle ground, as it is high enough for practical purposes but not so high that it
would lead to those edge cases.
For details, please refer to [PGM issue #733](https://github.com/PowerGridModel/power-grid-model/issues/733), which is
a `good-first-issue` and can be picked up by anyone who would like to.
```
