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

## Phase-shifting transformer

A [generic branch](./components.md#generic-branch) can represent a static phase-shifting transformer with finite series
impedance. For the tap position being calculated, use:

* `k` for the transformer magnitude ratio;
* `theta` for the fixed and tap-dependent phase shift, in radians;
* `r1` and `x1` for the transformer series resistance and reactance; and
* `g1` and `b1` for the magnetizing conductance and susceptance.

For example, a lossless phase-shifting transformer that still has finite series reactance can use `r1 = 0`, `x1 > 0`,
`g1 = 0`, `b1 = 0`, `k = 1`, and `theta` equal to the desired phase shift.

```{warning}
An impedance-free ideal phase link cannot be represented by setting both `r1` and `x1` to zero. The branch model uses
`1 / (r1 + j x1)` for its series admittance, so zero series impedance would make the model singular.
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
