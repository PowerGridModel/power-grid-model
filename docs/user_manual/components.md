<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Components

The attributes of components are listed below.

## Base

* type name: `base`

The base type for all power-grid-model components.

#### Input

| name | data type | unit | description                                                                                                                                                       | required |        update        |
| ---- | --------- | ---- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- | :------: | :------------------: |
| `id` | `int32_t` | -    | ID of a component. The ID should be unique across all components within the same scenario, e.g., you cannot have a node with `id=5` and another line with `id=5`. | &#10004; | &#10060; (see below) |

If a component update is uniform and is updating all the elements with the same component type, IDs can be omitted or
set to `nan`s.
In any other case, the IDs need to be present in the update dataset, but cannot be changed.

Uniform component updates are ones that update the same number of component of the same type across scenarios.
By definition, a dense update is always uniform, hence the IDs can always be optional; whereas for a sparse update, the
IDs can only be optional if the index pointer is given as an arithmetic sequence of all elements (i.e. a sparse
representation of a dense buffer).
An example of the usage of optional IDs is given in [Power Flow Example](./Power%20Flow%20Example.ipynb)

#### Steady state output and Short circuit output

| name        | data type | unit | description                                                                                                                        |
| ----------- | --------- | ---- | ---------------------------------------------------------------------------------------------------------------------------------- |
| `id`        | `int32_t` | -    | ID of a component, the ID should be unique across all components, e.g., you cannot have a node with `id=5` and a line with `id=5`. |
| `energized` | `int8_t`  | -    | Indicates if a component is energized, i.e. connected to a source                                                                  |

## Node

* type name: `node`
* base: {hoverxreftooltip}`user_manual/components:base`

`node` is a point in the grid.
Physically a node can be a busbar, a joint, or other similar component.

#### Input

| name      | data type | unit     | description             | required |  update  | valid values |
| --------- | --------- | -------- | ----------------------- | :------: | :------: | :----------: |
| `u_rated` | `double`  | volt (V) | rated line-line voltage | &#10004; | &#10060; |    `> 0`     |

#### Steady state output

| name      | data type         | unit                       | description                                                                                     |
| --------- | ----------------- | -------------------------- | ----------------------------------------------------------------------------------------------- |
| `u_pu`    | `RealValueOutput` | -                          | per-unit voltage magnitude                                                                      |
| `u_angle` | `RealValueOutput` | rad                        | voltage angle                                                                                   |
| `u`       | `RealValueOutput` | volt (V)                   | voltage magnitude, line-line for symmetric calculation, line-neutral for asymmetric calculation |
| `p`       | `RealValueOutput` | watt (W)                   | active power injection                                                                          |
| `q`       | `RealValueOutput` | volt-ampere-reactive (var) | reactive power injection                                                                        |

```{note}
The `p` and `q` output of injection follows the `generator` reference direction as mentioned in  
{hoverxreftooltip}`user_manual/data-model:Reference Direction`
```

#### Short circuit output

| name      | data type         | unit     | description                      |
| --------- | ----------------- | -------- | -------------------------------- |
| `u_pu`    | `RealValueOutput` | -        | per-unit voltage magnitude       |
| `u_angle` | `RealValueOutput` | rad      | voltage angle                    |
| `u`       | `RealValueOutput` | volt (V) | voltage magnitude (line-neutral) |

## Branch

* type name: `branch`
* base: {hoverxreftooltip}`user_manual/components:base`

`branch` is the abstract base type for the component which connects two *different* nodes.
For each branch two switches are always defined at from- and to-side of the branch.
In reality such switches may not exist.
For example, a cable usually permanently connects two joints.
In this case, the attribute `from_status` and `to_status` is always 1.

#### Input

| name          | data type | unit | description                    | required |  update  |  valid values   |
| ------------- | --------- | ---- | ------------------------------ | :------: | :------: | :-------------: |
| `from_node`   | `int32_t` | -    | ID of node at from-side        | &#10004; | &#10060; | a valid node ID |
| `to_node`     | `int32_t` | -    | ID of node at to-side          | &#10004; | &#10060; | a valid node ID |
| `from_status` | `int8_t`  | -    | connection status at from-side | &#10004; | &#10004; |   `0` or `1`    |
| `to_status`   | `int8_t`  | -    | connection status at to-side   | &#10004; | &#10004; |   `0` or `1`    |

#### Steady state output

| name      | data type         | unit                       | description                                              |
| --------- | ----------------- | -------------------------- | -------------------------------------------------------- |
| `p_from`  | `RealValueOutput` | watt (W)                   | active power flowing into the branch at from-side        |
| `q_from`  | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at from-side      |
| `i_from`  | `RealValueOutput` | ampere (A)                 | magnitude of current at from-side                        |
| `s_from`  | `RealValueOutput` | volt-ampere (VA)           | apparent power flowing at from-side                      |
| `p_to`    | `RealValueOutput` | watt (W)                   | active power flowing into the branch at to-side          |
| `q_to`    | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at to-side        |
| `i_to`    | `RealValueOutput` | ampere (A)                 | magnitude of current at to-side                          |
| `s_to`    | `RealValueOutput` | volt-ampere (VA)           | apparent power flowing at to-side                        |
| `loading` | `double`          | -                          | relative loading of the line, `1.0` meaning 100% loaded. |

#### Short circuit output

| name           | data type         | unit       | description                       |
| -------------- | ----------------- | ---------- | --------------------------------- |
| `i_from`       | `RealValueOutput` | ampere (A) | magnitude of current at from-side |
| `i_from_angle` | `RealValueOutput` | rad        | current angle at from-side        |
| `i_to`         | `RealValueOutput` | ampere (A) | magnitude of current at to-side   |
| `i_to_angle`   | `RealValueOutput` | rad        | current angle at to-side          |

### Line

* type name: `line`

`line` is a {hoverxreftooltip}`user_manual/components:branch` with specified serial impedance and shunt admittance.
A cable is also modeled as `line`.
A `line` can only connect two nodes with the same rated voltage.
If `i_n` is not provided, `loading` of line will be a `nan` value.

#### Input

| name   | data type | unit       | description                                |                 required                  |  update  |            valid values            |
| ------ | --------- | ---------- | ------------------------------------------ | :---------------------------------------: | :------: | :--------------------------------: |
| `r1`   | `double`  | ohm (Ω)    | positive-sequence serial resistance        |                 &#10004;                  | &#10060; | `r1` and `x1` cannot be both `0.0` |
| `x1`   | `double`  | ohm (Ω)    | positive-sequence serial reactance         |                 &#10004;                  | &#10060; | `r1` and `x1` cannot be both `0.0` |
| `c1`   | `double`  | farad (F)  | positive-sequence shunt capacitance        |                 &#10004;                  | &#10060; |                                    |
| `tan1` | `double`  | -          | positive-sequence shunt loss factor (tan𝛿) |                 &#10004;                  | &#10060; |                                    |
| `r0`   | `double`  | ohm (Ω)    | zero-sequence serial resistance            | &#10024; only for asymmetric calculations | &#10060; | `r0` and `x0` cannot be both `0.0` |
| `x0`   | `double`  | ohm (Ω)    | zero-sequence serial reactance             | &#10024; only for asymmetric calculations | &#10060; | `r0` and `x0` cannot be both `0.0` |
| `c0`   | `double`  | farad (F)  | zero-sequence shunt capacitance            | &#10024; only for asymmetric calculations | &#10060; |                                    |
| `tan0` | `double`  | -          | zero-sequence shunt loss factor (tan𝛿)     | &#10024; only for asymmetric calculations | &#10060; |                                    |
| `i_n`  | `double`  | ampere (A) | rated current                              |                 &#10060;                  | &#10060; |               `> 0`                |

```{note}
In case of short circuit calculations, the zero-sequence parameters are required only if any of the faults in any of the
scenarios within a batch are not three-phase faults (i.e. `fault_type` is not `FaultType.three_phase`).
```

#### Electric Model

`line` is described by a $\pi$ model, where

$$
   \begin{eqnarray}
      & Z_{\text{series}} = r + \mathrm{j}x \\
      & Y_{\text{shunt}} = \frac{2 \pi fc}{\tan \delta +\mathrm{j}}
   \end{eqnarray}
$$

### Link

* type name: `link`

`link` is a {hoverxreftooltip}`user_manual/components:branch` which usually represents a short internal cable/connection
between two busbars inside a substation.
It has a very high admittance (small impedance) which is set to a fixed per-unit value (equivalent to 10e6 siemens for
10kV network).
Therefore, it is chosen by design that no sensors can be coupled to a `link`.
There is no additional attribute for `link`.

#### Electric Model

`link` is modeled by a constant admittance $Y_{\text{series}}$, where

$$
   \begin{eqnarray}
      Y_{\text{series}} = (1 + \mathrm{j}) \cdot 10^6 \,\mathrm{p.u.}
    \end{eqnarray}
$$

### Transformer

`transformer` is a {hoverxreftooltip}`user_manual/components:branch` which connects two nodes with possibly different
voltage levels.
An example of usage of transformer is given in [Transformer Examples](../examples/Transformer%20Examples.ipynb)

#### Input

| name               | data type                                                   | unit             | description                                                                                                                                                                                                                 |                        required                         |  update  |                              valid values                              |
| ------------------ | ----------------------------------------------------------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-----------------------------------------------------: | :------: | :--------------------------------------------------------------------: |
| `u1`               | `double`                                                    | volt (V)         | rated voltage at from-side                                                                                                                                                                                                  |                        &#10004;                         | &#10060; |                                 `> 0`                                  |
| `u2`               | `double`                                                    | volt (V)         | rated voltage at to-side                                                                                                                                                                                                    |                        &#10004;                         | &#10060; |                                 `> 0`                                  |
| `sn`               | `double`                                                    | volt-ampere (VA) | rated power                                                                                                                                                                                                                 |                        &#10004;                         | &#10060; |                                 `> 0`                                  |
| `uk`               | `double`                                                    | -                | relative short circuit voltage, `0.1` means 10%                                                                                                                                                                             |                        &#10004;                         | &#10060; |                    `>= pk / sn` and `> 0` and `< 1`                    |
| `pk`               | `double`                                                    | watt (W)         | short circuit (copper) loss                                                                                                                                                                                                 |                        &#10004;                         | &#10060; |                                 `>= 0`                                 |
| `i0`               | `double`                                                    | -                | relative no-load current                                                                                                                                                                                                    |                        &#10004;                         | &#10060; |                         `>= p0 / sn` and `< 1`                         |
| `p0`               | `double`                                                    | watt (W)         | no-load (iron) loss                                                                                                                                                                                                         |                        &#10004;                         | &#10060; |                                 `>= 0`                                 |
| `winding_from`     | {py:class}`WindingType <power_grid_model.enum.WindingType>` | -                | from-side winding type                                                                                                                                                                                                      |                        &#10004;                         | &#10060; |                                                                        |
| `winding_to`       | {py:class}`WindingType <power_grid_model.enum.WindingType>` | -                | to-side winding type                                                                                                                                                                                                        |                        &#10004;                         | &#10060; |                                                                        |
| `clock`            | `int8_t`                                                    | -                | clock number of phase shift. Even number is not possible if one side is Y(N) winding and the other side is not Y(N) winding. Odd number is not possible, if both sides are Y(N) winding or both sides are not Y(N) winding. |                        &#10004;                         | &#10060; |                         `>= -12` and `<= 12`                           |
| `tap_side`         | {py:class}`BranchSide <power_grid_model.enum.BranchSide>`   | -                | side of tap changer                                                                                                                                                                                                         |                        &#10004;                         | &#10060; |                                                                        |
| `tap_pos`          | `int8_t`                                                    | -                | current position of tap changer                                                                                                                                                                                             | &#10060; default `tap_nom`, if no `tap_nom` default `0` | &#10004; | `(tap_min <= tap_pos <= tap_max)` or `(tap_min >= tap_pos >= tap_max)` |
| `tap_min`          | `int8_t`                                                    | -                | position of tap changer at minimum voltage                                                                                                                                                                                  |                        &#10004;                         | &#10060; |                                                                        |
| `tap_max`          | `int8_t`                                                    | -                | position of tap changer at maximum voltage                                                                                                                                                                                  |                        &#10004;                         | &#10060; |                                                                        |
| `tap_nom`          | `int8_t`                                                    | -                | nominal position of tap changer                                                                                                                                                                                             |                  &#10060; default `0`                   | &#10060; | `(tap_min <= tap_nom <= tap_max)` or `(tap_min >= tap_nom >= tap_max)` |
| `tap_size`         | `double`                                                    | volt (V)         | size of each tap of the tap changer                                                                                                                                                                                         |                        &#10004;                         | &#10060; |                                 `>= 0`                                 |
| `uk_min`           | `double`                                                    | -                | relative short circuit voltage at minimum tap                                                                                                                                                                               |              &#10060; default same as `uk`              | &#10060; |                  `>= pk_min / sn` and `> 0` and `< 1`                  |
| `uk_max`           | `double`                                                    | -                | relative short circuit voltage at maximum tap                                                                                                                                                                               |              &#10060; default same as `uk`              | &#10060; |                  `>= pk_max / sn` and `> 0` and `< 1`                  |
| `pk_min`           | `double`                                                    | watt (W)         | short circuit (copper) loss at minimum tap                                                                                                                                                                                  |              &#10060; default same as `pk`              | &#10060; |                                 `>= 0`                                 |
| `pk_max`           | `double`                                                    | watt (W)         | short circuit (copper) loss at maximum tap                                                                                                                                                                                  |              &#10060; default same as `pk`              | &#10060; |                                 `>= 0`                                 |
| `r_grounding_from` | `double`                                                    | ohm (Ω)          | grounding resistance at from-side, if relevant                                                                                                                                                                              |                  &#10060; default `0`                   | &#10060; |                                                                        |
| `x_grounding_from` | `double`                                                    | ohm (Ω)          | grounding reactance at from-side, if relevant                                                                                                                                                                               |                  &#10060; default `0`                   | &#10060; |                                                                        |
| `r_grounding_to`   | `double`                                                    | ohm (Ω)          | grounding resistance at to-side, if relevant                                                                                                                                                                                |                  &#10060; default `0`                   | &#10060; |                                                                        |
| `x_grounding_to`   | `double`                                                    | ohm (Ω)          | grounding reactance at to-side, if relevant                                                                                                                                                                                 |                  &#10060; default `0`                   | &#10060; |                                                                        |

```{note}
It can happen that `tap_min > tap_max`.
In this case the winding voltage is decreased if the tap position is increased.
```

#### Electric Model

`transformer` is described by a $\pi$ model, where $Z_{\text{series}}$ can be computed as

$$
    \begin{eqnarray}
        & |Z_{\text{series}}| = u_k*z_{\text{base,transformer}} \\
        & \mathrm{Re}(Z_{\text{series}}) = \frac{p_k}{s_n}*z_{\text{base,transformer}} \\
        & \mathrm{Im}(Z_{\text{series}}) = \sqrt{|Z_{\text{series}}|^2-\mathrm{Re}(Z_{\text{series}})^2} \\
    \end{eqnarray}
$$

and $Y_{\text{shunt}}$ can be computed as

$$
    \begin{eqnarray}
        & |Y_{\text{shunt}}| = i_0*y_{\text{base,transformer}} \\
        & \mathrm{Re}(Y_{\text{shunt}}) = \frac{s_n}{p_0}*y_{\text{base,transformer}} \\
        & \mathrm{Im}(Y_{\text{shunt}}) = -\sqrt{|Y_{\text{shunt}}|^2-\mathrm{Re}(Y_{\text{shunt}})^2} \\
   \end{eqnarray}
$$

where $z_{\text{base,transformer}} = 1 / y_{\text{base,transformer}} = {u_{\text{2}}}^2 / s_{\text{n}}$.

### Generic Branch  

* type name: `generic_branch`

`generic_branch` is a {hoverxreftooltip}`user_manual/components:branch` that connects two nodes, potentially at
different voltage levels.
Depending on the choice of parameters, it behaves either as a line or as a transformer.
The advantage is that the input parameters are based directly on the electrical equivalent circuit model.
The PI model can be used to avoid the need to convert parameters into transformer model data.
Another use case is modeling a line when connecting two nodes with approximately the same voltage levels (in that case,
the off-nominal ratio must be given to adapt the electrical parameters).

#### Input

| name    | data type | unit             | description                   |        required        |  update  | valid values |
| ------- | --------- | ---------------- | ----------------------------- | :--------------------: | :------: | :----------: |
| `r1`    | `double`  | ohm              | positive-sequence resistance  |        &#10004;        | &#10060; |              |
| `x1`    | `double`  | ohm              | positive-sequence reactance   |        &#10004;        | &#10060; |              |
| `g1`    | `double`  | siemens          | positive-sequence conductance |        &#10004;        | &#10060; |              |
| `b1`    | `double`  | siemens          | positive-sequence susceptance |        &#10004;        | &#10060; |              |
| `k`     | `double`  | -                | off-nominal ratio             | &#10060; default `1.0` | &#10060; |    `> 0`     |
| `theta` | `double`  | radian           | angle shift                   | &#10060; default `0.0` | &#10060; |              |
| `sn`    | `double`  | volt-ampere (VA) | rated power                   | &#10060; default `0.0` | &#10060; |    `>= 0`    |

```{note}
The impedance (`r1`, `x1`) and admittance (`g1`, `b1`) attributes are calculated with reference to the "to" side of the
branch.
```

```{note}
To model a three-winding transformer using the `generic_branch`, the MVA method must be applied.
This means the user needs to calculate three equivalent `generic_branch` components and define an additional node to
represent the common winding connection point.
In rare cases, this method can result in negative electrical equivalent circuit elements.
Therefore, the input parameters are not checked for negative values.
```

```{warning}
The parameter `k` represents the **off-nominal ratio**, not the nominal voltage ratio.
This means that `k` must be explicitly provided by the user, particularly in cases involving a tap changer or a voltage
transformer.
The program does not automatically calculate `k` based on the nominal voltage levels of the connected nodes.
```

```{warning}
Asymmetric calculation is not supported for `generic_branch`.
```

#### Electric Model

`generic_branch` is described by a PI model, where

$$
   \begin{eqnarray}
      & Y_{\text{series}} = \frac{1}{r + \mathrm{j}x} \\
      & Y_{\text{shunt}} =  g + \mathrm{j}b \\
      & N = k \cdot e^{\mathrm{j} \theta} \\
   \end{eqnarray}
$$

### Asym Line

* type name: `asym_line`

`asym_line` is a {hoverxreftooltip}`user_manual/components:branch` with specified resistance and reactance per phase.
A cable can be modelled as `line` or `asym_line`. An `asym_line` can only connect two nodes with the same rated voltage.
If `i_n` is not provided, `loading` of line will be a `nan` value.
The `asym_line` denotes a 3 or 4 phase line with phases `a`, `b`, `c` and optionally `n` for neutral.

#### Input

The provided values will be converted to a matrix representing the line's properties per phase in the following form:

$$
   \begin{bmatrix}
      \text{aa} & \text{ba} & \text{ca} & \text{na}\\
      \text{ba} & \text{bb} & \text{cb} & \text{nb}\\
      \text{ca} & \text{cb} & \text{cc} & \text{nc}\\
      \text{na} & \text{nb} & \text{nc} & \text{nn}
   \end{bmatrix}
$$

This representation holds for all values `r_aa` ... `r_nn`, `x_aa` ... `x_nn` and `c_aa` ... `c_cc`.
If the neutral values are not provided, the last row and column from the above matrix are omitted.

| name   | data type | unit       | description                       | required                     |  update  | valid values |
| ------ | --------- | ---------- | --------------------------------- | ---------------------------- | :------: | :----------: |
| `r_aa` | `double`  | ohm (Ω)    | Series serial resistance aa       | &#10004;                     | &#10060; |    `> 0`     |
| `r_ba` | `double`  | ohm (Ω)    | Series serial resistance ba       | &#10004;                     | &#10060; |    `> 0`     |
| `r_bb` | `double`  | ohm (Ω)    | Series serial resistance bb       | &#10004;                     | &#10060; |    `> 0`     |
| `r_ca` | `double`  | ohm (Ω)    | Series serial resistance ca       | &#10004;                     | &#10060; |    `> 0`     |
| `r_cb` | `double`  | ohm (Ω)    | Series serial resistance cb       | &#10004;                     | &#10060; |    `> 0`     |
| `r_cc` | `double`  | ohm (Ω)    | Series serial resistance cc       | &#10004;                     | &#10060; |    `> 0`     |
| `r_na` | `double`  | ohm (Ω)    | Series serial resistance na       | &#10024; for a neutral phase | &#10060; |    `> 0`     |
| `r_nb` | `double`  | ohm (Ω)    | Series serial resistance nb       | &#10024; for a neutral phase | &#10060; |    `> 0`     |
| `r_nc` | `double`  | ohm (Ω)    | Series serial resistance nc       | &#10024; for a neutral phase | &#10060; |    `> 0`     |
| `r_nn` | `double`  | ohm (Ω)    | Series serial resistance nn       | &#10024; for a neutral phase | &#10060; |    `> 0`     |
| `x_aa` | `double`  | ohm (Ω)    | Series serial reactance aa        | &#10004;                     | &#10060; |    `> 0`     |
| `x_ba` | `double`  | ohm (Ω)    | Series serial reactance ba        | &#10004;                     | &#10060; |    `> 0`     |
| `x_bb` | `double`  | ohm (Ω)    | Series serial reactance bb        | &#10004;                     | &#10060; |    `> 0`     |
| `x_ca` | `double`  | ohm (Ω)    | Series serial reactance ca        | &#10004;                     | &#10060; |    `> 0`     |
| `x_cb` | `double`  | ohm (Ω)    | Series serial reactance cb        | &#10004;                     | &#10060; |    `> 0`     |
| `x_cc` | `double`  | ohm (Ω)    | Series serial reactance cc        | &#10004;                     | &#10060; |    `> 0`     |
| `x_na` | `double`  | ohm (Ω)    | Series serial reactance na        | &#10024; for a neutral phase | &#10060; |    `> 0`     |
| `x_nb` | `double`  | ohm (Ω)    | Series serial reactance nb        | &#10024; for a neutral phase | &#10060; |    `> 0`     |
| `x_nc` | `double`  | ohm (Ω)    | Series serial reactance nc        | &#10024; for a neutral phase | &#10060; |    `> 0`     |
| `x_nn` | `double`  | ohm (Ω)    | Series serial reactance nn        | &#10024; for a neutral phase | &#10060; |    `> 0`     |
| `c_aa` | `double`  | farad (F)  | Shunt nodal capacitance matrix aa | &#10024; for a full c matrix | &#10060; |    `> 0`     |
| `c_ba` | `double`  | farad (F)  | Shunt nodal capacitance matrix ba | &#10024; for a full c matrix | &#10060; |    `> 0`     |
| `c_bb` | `double`  | farad (F)  | Shunt nodal capacitance matrix bb | &#10024; for a full c matrix | &#10060; |    `> 0`     |
| `c_ca` | `double`  | farad (F)  | Shunt nodal capacitance matrix ca | &#10024; for a full c matrix | &#10060; |    `> 0`     |
| `c_cb` | `double`  | farad (F)  | Shunt nodal capacitance matrix cb | &#10024; for a full c matrix | &#10060; |    `> 0`     |
| `c_cc` | `double`  | farad (F)  | Shunt nodal capacitance matrix cc | &#10024; for a full c matrix | &#10060; |    `> 0`     |
| `c0`   | `double`  | farad (F)  | zero-sequence shunt capacitance   | &#10024; without a c matrix  | &#10060; |    `> 0`     |
| `c1`   | `double`  | farad (F)  | Series shunt capacitance          | &#10024; without a c matrix  | &#10060; |    `> 0`     |
| `i_n`  | `double`  | ampere (A) | rated current                     | &#10060;                     | &#10060; |    `> 0`     |

For the r and x matrices providing values for the neutral phase is optional.
To clarify which input values are required, please consult the tables below:

| r_aa ... r_cc | r_na     | r_nb     | r_nc     | r_nn     | result   | Validation Error          |
| ------------- | -------- | -------- | -------- | -------- | -------- | ------------------------- |
| &#10004;      | &#10004; | &#10004; | &#10004; | &#10004; | &#10004; |                           |
| &#10004;      | &#10004; | &#10004; | &#10004; | &#10060; | &#10060; | MultiFieldValidationError |
| &#10004;      | &#10004; | &#10004; | &#10004; | &#10060; | &#10060; | MultiFieldValidationError |
| &#10004;      | &#10004; | &#10004; | &#10060; | &#10060; | &#10060; | MultiFieldValidationError |
| &#10004;      | &#10004; | &#10060; | &#10060; | &#10060; | &#10060; | MultiFieldValidationError |
| &#10004;      | &#10060; | &#10060; | &#10060; | &#10060; | &#10004; |                           |
| &#10060;      | &#10060; | &#10060; | &#10060; | &#10060; | &#10060; | MultiFieldValidationError |

| x_aa ... x_cc | x_na     | x_nb     | x_nc     | x_nn     | result   | Validation Error          |
| ------------- | -------- | -------- | -------- | -------- | -------- | ------------------------- |
| &#10004;      | &#10004; | &#10004; | &#10004; | &#10004; | &#10004; |                           |
| &#10004;      | &#10004; | &#10004; | &#10004; | &#10060; | &#10060; | MultiFieldValidationError |
| &#10004;      | &#10004; | &#10004; | &#10004; | &#10060; | &#10060; | MultiFieldValidationError |
| &#10004;      | &#10004; | &#10004; | &#10060; | &#10060; | &#10060; | MultiFieldValidationError |
| &#10004;      | &#10004; | &#10060; | &#10060; | &#10060; | &#10060; | MultiFieldValidationError |
| &#10004;      | &#10060; | &#10060; | &#10060; | &#10060; | &#10004; |                           |
| &#10060;      | &#10060; | &#10060; | &#10060; | &#10060; | &#10060; | MultiFieldValidationError |

For the c-matrix values there are two options.
Either provide all the required c-matrix values i.e. `c_aa` ... `c_cc` or provide `c0`, `c1`.
Whenever both sets are supplied the powerflow calculations will use `c0`, `c1`.
The table below provides guidance in providing valid input.

| c_aa ... c_cc | c0       | c1       | result   | Validation Error          |
| ------------- | -------- | -------- | -------- | ------------------------- |
| &#10004;      | &#10004; | &#10004; | &#10004; |                           |
| &#10004;      | &#10004; | &#10060; | &#10004; |                           |
| &#10004;      | &#10060; | &#10060; | &#10004; |                           |
| &#10060;      | &#10004; | &#10060; | &#10060; | MultiFieldValidationError |
| &#10060;      | &#10004; | &#10004; | &#10004; |                           |

#### Electric Model

The cable properties are described using matrices where the $Z_{\text{series}}$ matrix is computed as:

$$
   Z_{\text{series}} = R + \mathrm{j} * X
$$

Where $R$ and $X$ denote the resistance and reactance matrices build from the input respectively.

Whenever the neutral phase is provided in the $Z_{\text{series}}$, the $Z_{\text{series}}$ matrix will first be reduced
to a 3 phase matrix $Z_{\text{reduced}}$ with a kron reduction as follows:

$$
   Z_{\text{aa}} = \begin{bmatrix}
                     Z_{\text{0,0}} & Z_{\text{1,0}} & Z_{\text{2,0}}\\
                     Z_{\text{1,0}} & Z_{\text{1,1}} & Z_{\text{2,1}}\\
                     Z_{\text{2,0}} & Z_{\text{2,1}} & Z_{\text{2,2}}
                   \end{bmatrix}
$$
$$
   Z_{\text{ab}} = \begin{bmatrix}
                     Z_{\text{0,3}} \\
                     Z_{\text{1,3}} \\
                     Z_{\text{2,3}}
                   \end{bmatrix}
$$
$$
   Z_{\text{ba}} = \begin{bmatrix}
                     Z_{\text{3,0}} \\
                     Z_{\text{3,1}} \\
                     Z_{\text{3,2}}
                   \end{bmatrix}
$$
$$
   Z_{\text{bb}}^{-1} = \frac{1}{Z_{\text{3,3}}}
$$
$$
   Z_{\text{reduced}} = Z_{\text{aa}} - Z_{\text{ba}} \otimes Z_{\text{ab}} \cdot Z_{\text{bb}}^{-1}
$$

Where $Z_{\text{i,j}}$ denotes the row and column of the $Z_{\text{series}}$ matrix.

## Branch3

* type name: `branch3`
* base: {hoverxreftooltip}`user_manual/components:base`

`branch3` is the abstract base type for the component which connects three *different* nodes.
For each branch3 three switches are always defined at side 1, 2, or 3 of the branch.
In reality such switches may not exist.

#### Input

| name       | data type | unit | description                 | required |  update  |  valid values   |
| ---------- | --------- | ---- | --------------------------- | :------: | :------: | :-------------: |
| `node_1`   | `int32_t` | -    | ID of node at side 1        | &#10004; | &#10060; | a valid node ID |
| `node_2`   | `int32_t` | -    | ID of node at side 2        | &#10004; | &#10060; | a valid node ID |
| `node_3`   | `int32_t` | -    | ID of node at side 3        | &#10004; | &#10060; | a valid node ID |
| `status_1` | `int8_t`  | -    | connection status at side 1 | &#10004; | &#10004; |   `0` or `1`    |
| `status_2` | `int8_t`  | -    | connection status at side 2 | &#10004; | &#10004; |   `0` or `1`    |
| `status_3` | `int8_t`  | -    | connection status at side 3 | &#10004; | &#10004; |   `0` or `1`    |

#### Steady state output

| name      | data type         | unit                       | description                                                |
| --------- | ----------------- | -------------------------- | ---------------------------------------------------------- |
| `p_1`     | `RealValueOutput` | watt (W)                   | active power flowing into the branch at side 1             |
| `q_1`     | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at side 1           |
| `i_1`     | `RealValueOutput` | ampere (A)                 | current at side 1                                          |
| `s_1`     | `RealValueOutput` | volt-ampere (VA)           | apparent power flowing at side 1                           |
| `p_2`     | `RealValueOutput` | watt (W)                   | active power flowing into the branch at side 2             |
| `q_2`     | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at side 2           |
| `i_2`     | `RealValueOutput` | ampere (A)                 | current at side 2                                          |
| `s_2`     | `RealValueOutput` | volt-ampere (VA)           | apparent power flowing at side 2                           |
| `p_3`     | `RealValueOutput` | watt (W)                   | active power flowing into the branch at side 3             |
| `q_3`     | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at side 3           |
| `i_3`     | `RealValueOutput` | ampere (A)                 | current at side 3                                          |
| `s_3`     | `RealValueOutput` | volt-ampere (VA)           | apparent power flowing at side 3                           |
| `loading` | `double`          | -                          | relative loading of the branch, `1.0` meaning 100% loaded. |

#### Short circuit output

| name        | data type         | unit       | description             |
| ----------- | ----------------- | ---------- | ----------------------- |
| `i_1`       | `RealValueOutput` | ampere (A) | current at side 1       |
| `i_1_angle` | `RealValueOutput` | rad        | current angle at side 1 |
| `i_2`       | `RealValueOutput` | ampere (A) | current at side 2       |
| `i_2_angle` | `RealValueOutput` | rad        | current angle at side 2 |
| `i_3`       | `RealValueOutput` | ampere (A) | current at side 3       |
| `i_3_angle` | `RealValueOutput` | rad        | current angle at side 3 |

### Three-Winding Transformer

`three_winding_transformer` is a {hoverxreftooltip}`user_manual/components:branch3` connects three nodes with possibly
different voltage levels.
An example of usage of three-winding transformer is given in
[Transformer Examples](../examples/Transformer%20Examples.ipynb).

#### Input

| name            | data type                                                   | unit             | description                                                                                               |                        required                         |  update  |                              valid values                              |
| --------------- | ----------------------------------------------------------- | ---------------- | --------------------------------------------------------------------------------------------------------- | :-----------------------------------------------------: | :------: | :--------------------------------------------------------------------: |
| `u1`            | `double`                                                    | volt (V)         | rated voltage at side 1                                                                                   |                        &#10004;                         | &#10060; |                                 `> 0`                                  |
| `u2`            | `double`                                                    | volt (V)         | rated voltage at side 2                                                                                   |                        &#10004;                         | &#10060; |                                 `> 0`                                  |
| `u3`            | `double`                                                    | volt (V)         | rated voltage at side 3                                                                                   |                        &#10004;                         | &#10060; |                                 `> 0`                                  |
| `sn_1`          | `double`                                                    | volt-ampere (VA) | rated power at side 1                                                                                     |                        &#10004;                         | &#10060; |                                 `> 0`                                  |
| `sn_2`          | `double`                                                    | volt-ampere (VA) | rated power at side 2                                                                                     |                        &#10004;                         | &#10060; |                                 `> 0`                                  |
| `sn_3`          | `double`                                                    | volt-ampere (VA) | rated power at side 3                                                                                     |                        &#10004;                         | &#10060; |                                 `> 0`                                  |
| `uk_12`         | `double`                                                    | -                | relative short circuit voltage across side 1-2, `0.1` means 10%                                           |                        &#10004;                         | &#10060; |            `>= pk_12 / min(sn_1, sn_2)` and `> 0` and `< 1`            |
| `uk_13`         | `double`                                                    | -                | relative short circuit voltage across side 1-3, `0.1` means 10%                                           |                        &#10004;                         | &#10060; |            `>= pk_13 / min(sn_1, sn_3)` and `> 0` and `< 1`            |
| `uk_23`         | `double`                                                    | -                | relative short circuit voltage across side 2-3, `0.1` means 10%                                           |                        &#10004;                         | &#10060; |            `>= pk_23 / min(sn_2, sn_3)` and `> 0` and `< 1`            |
| `pk_12`         | `double`                                                    | watt (W)         | short circuit (copper) loss across side 1-2                                                               |                        &#10004;                         | &#10060; |                                 `>= 0`                                 |
| `pk_13`         | `double`                                                    | watt (W)         | short circuit (copper) loss across side 1-3                                                               |                        &#10004;                         | &#10060; |                                 `>= 0`                                 |
| `pk_23`         | `double`                                                    | watt (W)         | short circuit (copper) loss across side 2-3                                                               |                        &#10004;                         | &#10060; |                                 `>= 0`                                 |
| `i0`            | `double`                                                    | -                | relative no-load current with respect to side 1                                                           |                        &#10004;                         | &#10060; |                         `>= p0 / sn` and `< 1`                         |
| `p0`            | `double`                                                    | watt (W)         | no-load (iron) loss                                                                                       |                        &#10004;                         | &#10060; |                                 `>= 0`                                 |
| `winding_1`     | {py:class}`WindingType <power_grid_model.enum.WindingType>` | -                | side 1 winding type                                                                                       |                        &#10004;                         | &#10060; |                                                                        |
| `winding_2`     | {py:class}`WindingType <power_grid_model.enum.WindingType>` | -                | side 2 winding type                                                                                       |                        &#10004;                         | &#10060; |                                                                        |
| `winding_3`     | {py:class}`WindingType <power_grid_model.enum.WindingType>` | -                | side 3 winding type                                                                                       |                        &#10004;                         | &#10060; |                                                                        |
| `clock_12`      | `int8_t`                                                    | -                | clock number of phase shift across side 1-2, odd number is only allowed for Dy(n) or Y(N)d configuration. |                        &#10004;                         | &#10060; |                         `>= -12` and `<= 12`                           |
| `clock_13`      | `int8_t`                                                    | -                | clock number of phase shift across side 1-3, odd number is only allowed for Dy(n) or Y(N)d configuration. |                        &#10004;                         | &#10060; |                         `>= -12` and `<= 12`                           |
| `tap_side`      | {py:class}`Branch3Side <power_grid_model.enum.Branch3Side>` | -                | side of tap changer                                                                                       |                        &#10004;                         | &#10060; |                    `side_1` or `side_2` or `side_3`                    |
| `tap_pos`       | `int8_t`                                                    | -                | current position of tap changer                                                                           | &#10060; default `tap_nom`, if no `tap_nom` default `0` | &#10004; | `(tap_min <= tap_pos <= tap_max)` or `(tap_min >= tap_pos >= tap_max)` |
| `tap_min`       | `int8_t`                                                    | -                | position of tap changer at minimum voltage                                                                |                        &#10004;                         | &#10060; |                                                                        |
| `tap_max`       | `int8_t`                                                    | -                | position of tap changer at maximum voltage                                                                |                        &#10004;                         | &#10060; |                                                                        |
| `tap_nom`       | `int8_t`                                                    | -                | nominal position of tap changer                                                                           |                  &#10060; default `0`                   | &#10060; | `(tap_min <= tap_nom <= tap_max)` or `(tap_min >= tap_nom >= tap_max)` |
| `tap_size`      | `double`                                                    | volt (V)         | size of each tap of the tap changer                                                                       |                        &#10004;                         | &#10060; |                                 `> 0`                                  |
| `uk_12_min`     | `double`                                                    | -                | relative short circuit voltage at minimum tap, across side 1-2                                            |            &#10060; default same as `uk_12`             | &#10060; |          `>= pk_12_min / min(sn_1, sn_2)` and `> 0` and `< 1`          |
| `uk_12_max`     | `double`                                                    | -                | relative short circuit voltage at maximum tap, across side 1-2                                            |            &#10060; default same as `uk_12`             | &#10060; |          `>= pk_12_max / min(sn_1, sn_2)` and `> 0` and `< 1`          |
| `pk_12_min`     | `double`                                                    | watt (W)         | short circuit (copper) loss at minimum tap, across side 1-2                                               |            &#10060; default same as `pk_12`             | &#10060; |                                 `>= 0`                                 |
| `pk_12_max`     | `double`                                                    | watt (W)         | short circuit (copper) loss at maximum tap, across side 1-2                                               |            &#10060; default same as `pk_12`             | &#10060; |                                 `>= 0`                                 |
| `uk_13_min`     | `double`                                                    | -                | relative short circuit voltage at minimum tap, across side 1-3                                            |            &#10060; default same as `uk_13`             | &#10060; |          `>= pk_13_min / min(sn_1, sn_3)` and `> 0` and `< 1`          |
| `uk_13_max`     | `double`                                                    | -                | relative short circuit voltage at maximum tap, across side 1-3                                            |            &#10060; default same as `uk_13`             | &#10060; |          `>= pk_13_max / min(sn_1, sn_3)` and `> 0` and `< 1`          |
| `pk_13_min`     | `double`                                                    | watt (W)         | short circuit (copper) loss at minimum tap, across side 1-3                                               |            &#10060; default same as `pk_13`             | &#10060; |                                 `>= 0`                                 |
| `pk_13_max`     | `double`                                                    | watt (W)         | short circuit (copper) loss at maximum tap, across side 1-3                                               |            &#10060; default same as `pk_13`             | &#10060; |                                 `>= 0`                                 |
| `uk_23_min`     | `double`                                                    | -                | relative short circuit voltage at minimum tap, across side 2-3                                            |            &#10060; default same as `uk_23`             | &#10060; |          `>= pk_23_min / min(sn_2, sn_3)` and `> 0` and `< 1`          |
| `uk_23_max`     | `double`                                                    | -                | relative short circuit voltage at maximum tap, across side 2-3                                            |            &#10060; default same as `uk_23`             | &#10060; |          `>= pk_23_max / min(sn_2, sn_3)` and `> 0` and `< 1`          |
| `pk_23_min`     | `double`                                                    | watt (W)         | short circuit (copper) loss at minimum tap, across side 2-3                                               |            &#10060; default same as `pk_23`             | &#10060; |                                 `>= 0`                                 |
| `pk_23_max`     | `double`                                                    | watt (W)         | short circuit (copper) loss at maximum tap, across side 2-3                                               |            &#10060; default same as `pk_23`             | &#10060; |                                 `>= 0`                                 |
| `r_grounding_1` | `double`                                                    | ohm (Ω)          | grounding resistance at side 1, if relevant                                                               |                  &#10060; default `0`                   | &#10060; |                                                                        |
| `x_grounding_1` | `double`                                                    | ohm (Ω)          | grounding reactance at side 1, if relevant                                                                |                  &#10060; default `0`                   | &#10060; |                                                                        |
| `r_grounding_2` | `double`                                                    | ohm (Ω)          | grounding resistance at side 2, if relevant                                                               |                  &#10060; default `0`                   | &#10060; |                                                                        |
| `x_grounding_2` | `double`                                                    | ohm (Ω)          | grounding reactance at side 2, if relevant                                                                |                  &#10060; default `0`                   | &#10060; |                                                                        |
| `r_grounding_3` | `double`                                                    | ohm (Ω)          | grounding resistance at side 3, if relevant                                                               |                  &#10060; default `0`                   | &#10060; |                                                                        |
| `x_grounding_3` | `double`                                                    | ohm (Ω)          | grounding reactance at side 3, if relevant                                                                |                  &#10060; default `0`                   | &#10060; |                                                                        |

```{note}
It can happen that `tap_min > tap_max`.
In this case the winding voltage is decreased if the tap position is increased.
```

#### Electric Model

`three_winding_transformer` is modelled as 3 transformers of `pi` model each connected together in star configuration.
However, there are only 2 `pi` "legs": One at `side_1` and one in the centre of star.
The values between windings (for e.g., `uk_12` or `pk_23`) are converted from delta to corresponding star configuration
values.
The calculation of series and shunt admittance from `uk`, `pk`, `i0` and `p0` is same as mentioned in
{hoverxreftooltip}`user_manual/components:transformer`.

## Appliance

* type name: `appliance`
* base: {hoverxreftooltip}`user_manual/components:base`

`appliance` is an abstract user which is coupled to a `node`.
For each `appliance`, a switch is defined between the `appliance` and the `node`.
The reference direction for power flows is mentioned in {hoverxreftooltip}`user_manual/data-model:Reference Direction`.

#### Input

| name     | data type | unit | description                   | required |  update  |  valid values   |
| -------- | --------- | ---- | ----------------------------- | :------: | :------: | :-------------: |
| `node`   | `int32_t` | -    | ID of the coupled node        | &#10004; | &#10060; | a valid node ID |
| `status` | `int8_t`  | -    | connection status to the node | &#10004; | &#10004; |   `0` or `1`    |

#### Steady state output

| name | data type         | unit                       | description    |
| ---- | ----------------- | -------------------------- | -------------- |
| `p`  | `RealValueOutput` | watt (W)                   | active power   |
| `q`  | `RealValueOutput` | volt-ampere-reactive (var) | reactive power |
| `i`  | `RealValueOutput` | ampere (A)                 | current        |
| `s`  | `RealValueOutput` | volt-ampere (VA)           | apparent power |
| `pf` | `RealValueOutput` | -                          | power factor   |

#### Short circuit output

| name      | data type         | unit       | description   |
| --------- | ----------------- | ---------- | ------------- |
| `i`       | `RealValueOutput` | ampere (A) | current       |
| `i_angle` | `RealValueOutput` | rad        | current angle |

### Source

* type name: `source`
* {hoverxreftooltip}`user_manual/data-model:Reference Direction`: generator

`source` is an {hoverxreftooltip}`user_manual/components:appliance` representing the external network with a
[Thévenin's equivalence](https://en.wikipedia.org/wiki/Th%C3%A9venin%27s_theorem).
It has an infinite voltage source with an internal impedance.
The impedance is specified by convention as short circuit power.

#### Input

| name          | data type | unit             | description                                        |           required           |  update  | valid values |
| ------------- | --------- | ---------------- | -------------------------------------------------- | :--------------------------: | :------: | :----------: |
| `u_ref`       | `double`  | -                | reference voltage in per-unit                      | &#10024; only for power flow | &#10004; |    `> 0`     |
| `u_ref_angle` | `double`  | rad              | reference voltage angle                            |    &#10060; default `0.0`    | &#10004; |              |
| `sk`          | `double`  | volt-ampere (VA) | short circuit power                                |   &#10060; default `1e10`    | &#10060; |    `> 0`     |
| `rx_ratio`    | `double`  | -                | R to X ratio                                       |    &#10060; default `0.1`    | &#10060; |    `>= 0`    |
| `z01_ratio`   | `double`  | -                | zero-sequence to positive sequence impedance ratio |    &#10060; default `1.0`    | &#10060; |    `> 0`     |

#### Electric Model

`source` is modeled by an internal constant impedance $r+\mathrm{j}x$ with positive sequence and zero-sequence.
Its value can be computed using following equations:

* for positive sequence,

$$
   \begin{eqnarray}
        & z_{\text{source}} = \frac{s_{\text{base}}}{s_k} \\
        & x_1 = \frac{z_{\text{source}}}{\sqrt{1+ \left(\frac{r}{x}\right)^2}} \\
        & r_1 = x_1 \cdot \left(\frac{r}{x}\right)
   \end{eqnarray}
$$

where $s_{\text{base}}$ is a constant value determined by the solver, and $\frac{r}{x}$ indicates `rx_ratio` as input.

* for zero-sequence,

$$
   \begin{eqnarray}
        & z_{\text{source,0}} = z_{\text{source}} \cdot \frac{z_0}{z_1} \\
        & x_0 = \frac{z_{\text{source,0}}}{\sqrt{1 + \left(\frac{r}{x}\right)^2}} \\
        & r_0 = x_0 \cdot \left(\frac{r}{x}\right)
   \end{eqnarray}
$$

### Generic Load and Generator

* type name: `generic_load_gen`

`generic_load_gen` is an abstract load/generation {hoverxreftooltip}`user_manual/components:appliance` which contains
only the type of the load/generation with response to voltage.

| name   | data type                                                   | unit | description                                     | required |  update  |
| ------ | ----------------------------------------------------------- | ---- | ----------------------------------------------- | :------: | :------: |
| `type` | {py:class}`LoadGenType <power_grid_model.enum.LoadGenType>` | -    | type of load/generator with response to voltage | &#10004; | &#10060; |

#### Load/Generator Concrete Types

There are four concrete types of load/generator.
They share similar attributes: specified active/reactive power.
However, the reference direction and meaning of `RealValueInput` is different, as shown in the table below.

| type name   | reference direction | meaning of `RealValueInput` |
| ----------- | ------------------- | --------------------------- |
| `sym_load`  | load                | `double`                    |
| `sym_gen`   | generator           | `double`                    |
| `asym_load` | load                | `double[3]`                 |
| `asym_gen`  | generator           | `double[3]`                 |

##### Input

| name          | data type        | unit                       | description              |           required           |  update  |
| ------------- | ---------------- | -------------------------- | ------------------------ | :--------------------------: | :------: |
| `p_specified` | `RealValueInput` | watt (W)                   | specified active power   | &#10024; only for power flow | &#10004; |
| `q_specified` | `RealValueInput` | volt-ampere-reactive (var) | specified reactive power | &#10024; only for power flow | &#10004; |

##### Electric model

`generic_load_gen` is modelled by using the so-called ZIP load model in power-grid-model, where a load/generator is
represented as a composition of constant power (P), constant current (I) and constant impedance (Z).

The injection of each ZIP model type can be computed as follows:

* for a constant impedance (Z) load/generator,

$$
   \begin{eqnarray}
        S = S_{\text{specified}} \cdot \bar{u}^2
   \end{eqnarray}
$$

* for a constant current (I) load/generator,

$$
   \begin{eqnarray}
        S = S_{\text{specified}} \cdot \bar{u}
   \end{eqnarray}
$$

* for a constant power (P) load/generator:,

$$
   \begin{eqnarray}
        S = S_{\text{specified}}
   \end{eqnarray}
$$

where $\bar{u}$ is the calculated node voltage.

### Shunt

* type name: `shunt`
* {hoverxreftooltip}`user_manual/data-model:Reference Direction`: load

`shunt` is an {hoverxreftooltip}`user_manual/components:appliance` with a fixed admittance (impedance).
It behaves similar to a load/generator with type `const_impedance`.

#### Input

| name | data type | unit        | description                         |                 required                 |  update  |
| ---- | --------- | ----------- | ----------------------------------- | :--------------------------------------: | :------: |
| `g1` | `double`  | siemens (S) | positive-sequence shunt conductance |                 &#10004;                 | &#10004; |
| `b1` | `double`  | siemens (S) | positive-sequence shunt susceptance |                 &#10004;                 | &#10004; |
| `g0` | `double`  | siemens (S) | zero-sequence shunt conductance     | &#10024; only for asymmetric calculation | &#10004; |
| `b0` | `double`  | siemens (S) | zero-sequence shunt susceptance     | &#10024; only for asymmetric calculation | &#10004; |

```{note}
In case of short circuit calculations, the zero-sequence parameters are required only if any of the faults in any of the
scenarios within a batch are not three-phase faults (i.e. `fault_type` is not `FaultType.three_phase`).
```

#### Electric Model

`shunt` is modelled by a fixed admittance which equals to $g + \mathrm{j}b$.

## Sensor

* type name: `sensor`
* base: {hoverxreftooltip}`user_manual/components:base`

`sensor` is an abstract type for all the sensor types.
A sensor does not have any physical meaning.
Rather, it provides measurement data for the state estimation algorithm.
The state estimator uses the data to evaluate the state of the grid with the highest probability.

#### Input

| name              | data type | unit | description               | required |  update  |   valid values    |
| ----------------- | --------- | ---- | ------------------------- | :------: | :------: | :---------------: |
| `measured_object` | `int32_t` | -    | ID of the measured object | &#10004; | &#10060; | a valid object ID |

#### Output

A sensor only has output for state estimation.
For other calculation types, sensor output is undefined.

### Generic Voltage Sensor

* type name: `generic_voltage_sensor`

`generic_voltage_sensor` is an abstract class for symmetric and asymmetric voltage sensor and derived from
{hoverxreftooltip}`user_manual/components:sensor`.
It measures the magnitude and (optionally) the angle of the voltage of a `node`.

#### Input

| name      | data type | unit     | description                                                                                                     |              required              |  update  | valid values |
| --------- | --------- | -------- | --------------------------------------------------------------------------------------------------------------- | :--------------------------------: | :------: | :----------: |
| `u_sigma` | `double`  | volt (V) | standard deviation of the measurement error. Usually this is the absolute measurement error range divided by 3. | &#10024; only for state estimation | &#10004; |    `> 0`     |

#### Voltage Sensor Concrete Types

There are two concrete types of voltage sensor.
They share similar attributes: the meaning of `RealValueInput` is different, as shown in the table below.
In a `sym_voltage_sensor` the measured voltage is a line-to-line voltage.
In a `asym_voltage_sensor` the measured voltage is a 3-phase line-to-ground voltage.

| type name             | meaning of `RealValueInput` |
| --------------------- | --------------------------- |
| `sym_voltage_sensor`  | `double`                    |
| `asym_voltage_sensor` | `double[3]`                 |

##### Input

| name               | data type        | unit     | description                                                          |                                                     required                                                     |  update  | valid values |
| ------------------ | ---------------- | -------- | -------------------------------------------------------------------- | :--------------------------------------------------------------------------------------------------------------: | :------: | :----------: |
| `u_measured`       | `RealValueInput` | volt (V) | measured voltage magnitude                                           |                                        &#10024; only for state estimation                                        | &#10004; |    `> 0`     |
| `u_angle_measured` | `RealValueInput` | rad      | measured voltage angle (only possible with phasor measurement units) | &#10024; only for state estimation when a current sensor with `global_angle` `angle_measurement_type` is present | &#10004; |              |

```{note}
When a current sensor with `global_angle` `angle_measurement_type` is present there needs to be a voltage sensor with
`u_angle_measured` in the grid as a reference angle (when performing a state estimation).
```

##### Steady state output

```{note}
A sensor only has output for state estimation.
For other calculation types, sensor output is undefined.
```

| name               | data type         | unit     | description                                                                                                              |
| ------------------ | ----------------- | -------- | ------------------------------------------------------------------------------------------------------------------------ |
| `u_residual`       | `RealValueOutput` | volt (V) | residual value between measured voltage magnitude and calculated voltage magnitude                                       |
| `u_angle_residual` | `RealValueOutput` | rad      | residual value between measured voltage angle and calculated voltage angle (only possible with phasor measurement units) |

#### Electric Model

`generic_voltage_sensor` is modeled by following equations:

$$
   \begin{eqnarray}
        & u_{\text{residual}} = u_{\text{measured}} - u_{\text{state}} \\
        & \theta_{\text{residual}} = \theta_{\text{measured}} - \theta_{\text{state}} \pmod{2 \pi}
   \end{eqnarray}
$$

The $\pmod{2\pi}$ is handled such that $-\pi \lt \theta_{\text{angle},\text{residual}} \leq \pi$.

### Generic Power Sensor

* type name: `generic_power_sensor`

`power_sensor` is an abstract class for symmetric and asymmetric power sensor and is derived from
{hoverxreftooltip}`user_manual/components:sensor`.
It measures the active/reactive power flow of a terminal.
The terminal is either connecting an `appliance` and a `node`, or connecting the from/to end of a `branch` (except
`link`) and a `node`.
In case of a terminal between an `appliance` and a `node`, the power
{hoverxreftooltip}`user_manual/data-model:Reference Direction` in the measurement data is the same as the reference
direction of the `appliance`.
For example, if a `power_sensor` is measuring a `source`, a positive `p_measured` indicates that the active power flows
from the source to the node.

```{note}
1. Due to the high admittance of a `link` it is chosen that a power sensor cannot be coupled to a `link`, even though a
link is a `branch`

2. The node injection power sensor gets placed on a node.
In the state estimation result, the power from this injection is distributed equally among the connected appliances at
that node.
Because of this distribution, at least one appliance is required to be connected to the node where an injection sensor
is placed for it to function.
```

```{note}
It is not possible to mix [power sensors](./components.md#generic-power-sensor) with
[current sensors](./components.md#generic-current-sensor) on the same terminal of the same component.
It is also not possible to mix
[current sensors with global angle measurement type](./components.md#generic-current-sensor) with
[current sensors with local angle measurement type](./components.md#generic-current-sensor) on the same terminal of the
same component.
However, such mixing of sensor types is allowed as long as they are on different terminals.
```

##### Input

| name                     | data type                                                                     | unit             | description                                                                                                                                                                                 |                                                           required                                                            |  update  |                     valid values                     |
| ------------------------ | ----------------------------------------------------------------------------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :---------------------------------------------------------------------------------------------------------------------------: | :------: | :--------------------------------------------------: |
| `measured_terminal_type` | {py:class}`MeasuredTerminalType <power_grid_model.enum.MeasuredTerminalType>` | -                | indicate if it measures an `appliance` or a `branch`                                                                                                                                        |                                                           &#10004;                                                            | &#10060; | the terminal type should match the `measured_object` |
| `power_sigma`            | `double`                                                                      | volt-ampere (VA) | standard deviation of the measurement error. Usually this is the absolute measurement error range divided by 3. See {hoverxreftooltip}`user_manual/components:Power Sensor Concrete Types`. | &#10024; in certain cases for state estimation. See the explanation for [concrete types](#power-sensor-concrete-types) below. | &#10004; |                        `> 0`                         |

#### Power Sensor Concrete Types

There are two concrete types of power sensor.
They share similar attributes: the meaning of `RealValueInput` is different, as shown in the table below.

| type name           | meaning of `RealValueInput` |
| ------------------- | --------------------------- |
| `sym_power_sensor`  | `double`                    |
| `asym_power_sensor` | `double[3]`                 |

##### Input

| name         | data type        | unit                       | description                                                                                                                    |              required               |  update  | valid values |
| ------------ | ---------------- | -------------------------- | ------------------------------------------------------------------------------------------------------------------------------ | :---------------------------------: | :------: | :----------: |
| `p_measured` | `RealValueInput` | watt (W)                   | measured active power                                                                                                          | &#10024; only for state estimation  | &#10004; |              |
| `q_measured` | `RealValueInput` | volt-ampere-reactive (var) | measured reactive power                                                                                                        | &#10024; only for state estimation  | &#10004; |              |
| `p_sigma`    | `RealValueInput` | watt (W)                   | standard deviation of the active power measurement error. Usually this is the absolute measurement error range divided by 3.   | &#10060; see the explanation below. | &#10004; |    `> 0`     |
| `q_sigma`    | `RealValueInput` | volt-ampere-reactive (var) | standard deviation of the reactive power measurement error. Usually this is the absolute measurement error range divided by 3. | &#10060; see the explanation below. | &#10004; |    `> 0`     |

Valid combinations of `power_sigma`, `p_sigma` and `q_sigma` are:

| `power_sigma` | `p_sigma` | `q_sigma` |  result  |
| :-----------: | :-------: | :-------: | :------: |
|   &#10004;    | &#10004;  | &#10004;  | &#10004; |
|   &#10004;    | &#10004;  |           | &#10060; |
|   &#10004;    |           | &#10004;  | &#10060; |
|   &#10004;    |           |           | &#10004; |
|               | &#10004;  | &#10004;  | &#10004; |
|               | &#10004;  |           | &#10060; |
|               |           | &#10004;  | &#10060; |
|               |           |           | &#10060; |

```{note}
1. If both `p_sigma` and `q_sigma` are provided (i.e., not NaN), they represent the standard deviation of the active and
reactive power, respectively, and the value of `power_sigma` is ignored. Any infinite component disables the entire
measurement.

2. If neither `p_sigma` nor `q_sigma` are provided, `power_sigma` represents the standard deviation of the apparent
power. In this case, infinite value of `power_sigma` disables the entire measurement.

3. Providing only one of `p_sigma` and `q_sigma` results in undefined behaviour.
```

See the documentation on [state estimation calculation methods](calculations.md#state-estimation-algorithms) for details
per method on how the variances are taken into account for both the active and reactive power and for the individual
phases.

##### Steady state output

```{note}
A sensor only has output for state estimation.
For other calculation types, sensor output is undefined.
```

| name         | data type         | unit                       | description                                                                  |
| ------------ | ----------------- | -------------------------- | ---------------------------------------------------------------------------- |
| `p_residual` | `RealValueOutput` | watt (W)                   | residual value between measured active power and calculated active power     |
| `q_residual` | `RealValueOutput` | volt-ampere-reactive (var) | residual value between measured reactive power and calculated reactive power |

#### Electric Model

`Generic Power Sensor` is modeled by following equations:

$$
   \begin{eqnarray}
        & p_{\text{residual}} = p_{\text{measured}} - p_{\text{state}} \\
        & q_{\text{residual}} = q_{\text{measured}} - q_{\text{state}}
   \end{eqnarray}
$$

### Generic Current Sensor

* type name: `generic_current_sensor`

`current_sensor` is an abstract class for symmetric and asymmetric current sensor and is derived from
{hoverxreftooltip}`user_manual/components:sensor`.
It measures the magnitude and angle of the current flow of a terminal.
The terminal is connecting the from/to end of a `branch` (except `link`) and a `node`.

```{note}
Due to the high admittance of a `link` it is chosen that a current sensor cannot be coupled to a `link`, even though a
link is a `branch`.
```

```{note}
It is not possible to mix [power sensors](./components.md#generic-power-sensor) with
[current sensors](./components.md#generic-current-sensor) on the same terminal of the same component.
It is also not possible to mix
[current sensors with global angle measurement type](./components.md#generic-current-sensor) with
[current sensors with local angle measurement type](./components.md#generic-current-sensor) on the same terminal of the
same component.
However, such mixing of sensor types is allowed as long as they are on different terminals.
```

##### Input

| name                     | data type                                                                     | unit       | description                                                                                                                               |              required              |  update  |                     valid values                     |
| ------------------------ | ----------------------------------------------------------------------------- | ---------- | ----------------------------------------------------------------------------------------------------------------------------------------- | :--------------------------------: | :------: | :--------------------------------------------------: |
| `measured_terminal_type` | {py:class}`MeasuredTerminalType <power_grid_model.enum.MeasuredTerminalType>` | -          | indicate the side of the `branch`                                                                                                         |              &#10004;              | &#10060; | the terminal type should match the `measured_object` |
| `angle_measurement_type` | {py:class}`AngleMeasurementType <power_grid_model.enum.AngleMeasurementType>` | -          | indicate whether the measured angle is a global angle or a local angle; (see the [electric model](#local-angle-current-sensors) below)    |              &#10004;              | &#10060; |                                                      |
| `i_sigma`                | `double`                                                                      | ampere (A) | standard deviation of the current (`i`) measurement error. Usually this is the absolute measurement error range divided by 3.             | &#10024; only for state estimation | &#10004; |                        `> 0`                         |
| `i_angle_sigma`          | `double`                                                                      | rad        | standard deviation of the current (`i`) phase angle measurement error. Usually this is the absolute measurement error range divided by 3. | &#10024; only for state estimation | &#10004; |                        `> 0`                         |

#### Current Sensor Concrete Types

There are two concrete types of current sensor.
They share similar attributes: the meaning of `RealValueInput` is different, as shown in the table below.

| type name             | meaning of `RealValueInput` |
| --------------------- | --------------------------- |
| `sym_current_sensor`  | `double`                    |
| `asym_current_sensor` | `double[3]`                 |

##### Input

| name               | data type        | unit       | description                                                                                             |              required              |  update  |
| ------------------ | ---------------- | ---------- | ------------------------------------------------------------------------------------------------------- | :--------------------------------: | :------: |
| `i_measured`       | `RealValueInput` | ampere (A) | measured current (`i`) magnitude                                                                        | &#10024; only for state estimation | &#10004; |
| `i_angle_measured` | `RealValueInput` | rad        | measured phase angle of the current (`i`; see the [electric model](#local-angle-current-sensors) below) | &#10024; only for state estimation | &#10004; |

See the documentation on [state estimation calculation methods](calculations.md#state-estimation-algorithms) for details
per method on how the variances are taken into account for both the global and local angle measurement types and for the
individual phases.

```{note}
The combination of `i_measured=0` and `i_angle_measured=nπ/2` renders the current sensor invalid for PGM. 
See [State estimate sensor transformations](calculations.md#state-estimate-sensor-transformations).
```

##### Steady state output

```{note}
A sensor only has output for state estimation.
For other calculation types, sensor output is undefined.
```

| name               | data type         | unit       | description                                                                                 |
| ------------------ | ----------------- | ---------- | ------------------------------------------------------------------------------------------- |
| `i_residual`       | `RealValueOutput` | ampere (A) | residual value between measured current (`i`) and calculated current (`i`)                  |
| `i_angle_residual` | `RealValueOutput` | rad        | residual value between measured phase angle and calculated phase angle of the current (`i`) |

#### Electric Model

`Generic Current Sensor` is modeled by following equations:

##### Global angle current sensors

Current sensors with `angle_measurement_type` equal to `AngleMeasurementType.global_angle` measure the phase of the
current relative to some reference angle that is the same across the entire grid.
This reference angle must be the same one as for [voltage phasor measurements](#generic-voltage-sensor).
Because the reference point may be ambiguous in the case of current sensor measurements, the power-grid-model imposes
the following requirement:

```{note}
Global angle current measurements require at least one voltage angle measurement to make sense.
```

As a sign convention, the angle is the phase shift of the current relative to the reference angle, i.e.,

$$
   \begin{eqnarray}
      \underline{I} = \text{i}_{\text{measured}} \cdot e^{j \text{i}_{\text{angle,measured}}} \text{ .}
   \end{eqnarray}
$$

##### Local angle current sensors

Current sensors with `angle_measurement_type` equal to `AngleMeasurementType.local_angle` measure the phase shift
between the voltage and the current phasor, i.e.,
$\text{i_angle_measured} = \text{voltage_phase} - \text{current_phase}$.
As a result, the global current phasor depends on the local voltage phase offset and is obtained using the following
formula.

$$
   \underline{I} = \underline{I}_{\text{local}}^{*} \frac{\underline{U}}{|\underline{U}|}
      = \text{i}_{\text{measured}} \cdot e^{\mathrm{j} \left(\theta_{U} - \text{i}_{\text{angle,measured}}\right)}
$$

```{note}
As a result, the local angle current sensors have a different sign convention from the global angle current sensors.
```

##### Residuals

$$
   \begin{eqnarray}
        & i_{\text{residual}}
               = i_{\text{measured}} - i_{\text{state}} && \\
        & i_{\text{angle},\text{residual}}
               = i_{\text{angle},\text{measured}} - i_{\text{angle},\text{state}} \pmod{2 \pi}
   \end{eqnarray}
$$

The $\pmod{2\pi}$ is handled such that $-\pi \lt i_{\text{angle},\text{residual}} \leq \pi$.

## Fault

* type name: `fault`
* base: {hoverxreftooltip}`user_manual/components:base`

`fault` defines a short circuit location in the grid.
A fault can only happen at a `node`.

#### Input

| name           | data type                                                 | unit    | description                                         |                                                required                                                 |  update  |   valid values    |
| -------------- | --------------------------------------------------------- | ------- | --------------------------------------------------- | :-----------------------------------------------------------------------------------------------------: | :------: | :---------------: |
| `status`       | `int8_t`                                                  | -       | whether the fault is active                         |                                                &#10004;                                                 | &#10004; |    `0` or `1`     |
| `fault_type`   | {py:class}`FaultType <power_grid_model.enum.FaultType>`   | -       | the type of the fault                               |                                     &#10024; only for short circuit                                     | &#10004; |                   |
| `fault_phase`  | {py:class}`FaultPhase <power_grid_model.enum.FaultPhase>` | -       | the phase(s) of the fault                           | &#10060; default `FaultPhase.default_value` (see [below](#fault-types-fault-phases-and-default-values)) | &#10004; |                   |
| `fault_object` | `int32_t`                                                 | -       | ID of the component where the short circuit happens |                                                &#10004;                                                 | &#10004; | A valid `node` ID |
| `r_f`          | `double`                                                  | ohm (Ω) | short circuit resistance                            |                                         &#10060; default `0.0`                                          | &#10004; |                   |
| `x_f`          | `double`                                                  | ohm (Ω) | short circuit reactance                             |                                         &#10060; default `0.0`                                          | &#10004; |                   |

```{note}
Multiple faults may exist within one calculation.
Currently, all faults in one scenario are required to have the same `fault_type` and `fault_phase`.
Across scenarios in a batch, the `fault_type` and `fault_phase` may differ.
```

```{note}
If any of the faults in any of the scenarios within a batch are not `three_phase` (i.e. `fault_type` is not
`FaultType.three_phase`), the calculation is treated as asymmetric.
```

#### Steady state output

A `fault` has no steady state output.

#### Short circuit output

| name        | data type         | unit       | description   |
| ----------- | ----------------- | ---------- | ------------- |
| `i_f`       | `RealValueOutput` | ampere (A) | current       |
| `i_f_angle` | `RealValueOutput` | rad        | current angle |

#### Electric Model

##### Fault types, fault phases and default values

Four types of short circuit fault are included in power-grid-model, each with their own set of supported values for
`fault_phase`.
In case the `fault_phase` is not specified or is equal to `FaultPhase.default_value`, the power-grid-model assumes a
`fault_type`-dependent set of fault phases.
The supported values of `fault_phase`, as well as its default value, are listed in the table below.

| `fault_type`                       | supported values of `fault_phase`                 | `FaultPhase.default_value` | description                                                            |
| ---------------------------------- | ------------------------------------------------- | -------------------------- | ---------------------------------------------------------------------- |
| `FaultType.three_phase`            | `FaultPhase.abc`                                  | `FaultPhase.abc`           | Three phases are connected with fault impedance.                       |
| `FaultType.single_phase_to_ground` | `FaultPhase.a`, `FaultPhase.b`, `FaultPhase.c`    | `FaultPhase.a`             | One phase is grounded with fault impedance, and other phases are open. |
| `FaultType.two_phase`              | `FaultPhase.bc`, `FaultPhase.ac`, `FaultPhase.ab` | `FaultPhase.bc`            | Two phases are connected with fault impedance.                         |
| `FaultType.two_phase_to_ground`    | `FaultPhase.bc`, `FaultPhase.ac`, `FaultPhase.ab` | `FaultPhase.bc`            | Two phases are connected with fault impedance then grounded.           |

## Regulator

* type name: `regulator`
* base: {hoverxreftooltip}`user_manual/components:base`

`regulator` is an abstract regulator that is coupled to a given `regulated_object`. For each `regulator`, a switch is
defined between the `regulator` and the `regulated_object`.
Which object types are supported as `regulated_object` is regulator type-dependent.

#### Input

| name               | data type | unit | description                               | required |  update  |        valid values         |
| ------------------ | --------- | ---- | ----------------------------------------- | :------: | :------: | :-------------------------: |
| `regulated_object` | `int32_t` | -    | ID of the regulated object                | &#10004; | &#10060; | a valid regulated object ID |
| `status`           | `int8_t`  | -    | connection status to the regulated object | &#10004; | &#10004; |         `0` or `1`          |

### Transformer tap regulator

* type name: `transformer_tap_regulator`
* base: {hoverxreftooltip}`user_manual/components:regulator`

`transformer_tap_regulator` defines a regulator for transformers in the grid.
A transformer tap regulator regulates a component that is either a
{hoverxreftooltip}`user_manual/components:transformer` or a
{hoverxreftooltip}`user_manual/components:Three-Winding Transformer`.

The transformer tap regulator changes the `tap_pos` of the transformer it regulates in the range set by the user via
`tap_min` and `tap_max` (i.e., `(tap_min <= tap_pos <= tap_max)` or `(tap_min >= tap_pos >= tap_max)`).
It regulates the tap position so that the voltage on the control side is in the chosen voltage band.
Other points further into the grid on the control side, away from the transformer, can also be regulated by providing
the cumulative impedance across branches to that point as an additional line drop compensation.
This line drop compensation only affects the controlled voltage and does not have any impact on the actual grid.
It may therefore be treated as a virtual impedance in the grid.

```{note}
The regulator outputs the optimal tap position of the transformer.
The actual grid state is not changed after calculations are done.
```

#### Input

| name                       | data type                                                                                                                                                                                                                                                                                                          | unit     | description                                                                                             |           required           |  update  |                           valid values                           |
| -------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------- | ------------------------------------------------------------------------------------------------------- | :--------------------------: | :------: | :--------------------------------------------------------------: |
| `control_side`             | {py:class}`BranchSide <power_grid_model.enum.BranchSide>` if the regulated object is a {hoverxreftooltip}`user_manual/components:transformer` and {py:class}`Branch3Side <power_grid_model.enum.Branch3Side>` if it the regulated object is a {hoverxreftooltip}`user_manual/components:Three-Winding Transformer` | -        | the controlled side of the transformer                                                                  | &#10024; only for power flow | &#10060; | `control_side` should be the relatively further side to a source |
| `u_set`                    | `double`                                                                                                                                                                                                                                                                                                           | volt (V) | the voltage setpoint (at the center of the band)                                                        | &#10024; only for power flow | &#10004; |                              `>= 0`                              |
| `u_band`                   | `double`                                                                                                                                                                                                                                                                                                           | volt (V) | the width of the voltage band ($=2*\left(\Delta U\right)_{\text{acceptable}}$)                          | &#10024; only for power flow | &#10004; |                        `> 0` (see below)                         |
| `line_drop_compensation_r` | `double`                                                                                                                                                                                                                                                                                                           | ohm (Ω)  | compensation for voltage drop due to resistance during transport (see [below](#line-drop-compensation)) |    &#10060; default `0.0`    | &#10004; |                              `>= 0`                              |
| `line_drop_compensation_x` | `double`                                                                                                                                                                                                                                                                                                           | ohm (Ω)  | compensation for voltage drop due to reactance during transport (see [below](#line-drop-compensation))  |    &#10060; default `0.0`    | &#10004; |                              `>= 0`                              |

The following additional requirements exist on the input parameters.

* Depending on the resultant voltage being transformed, the voltage band must be sufficiently large: At zero line drop
  compensation if the expected resultant voltages are in the proximity of the rated transformer voltages, it is
  recommended to have the $u_{band} >= \frac{u_2}{1+ u_1 / \text{tap}_{\text{size}}}$
* The line drop compensation is small, in the sense that its product with the typical current through the transformer is
  much smaller (in absolute value) than the smallest change in voltage due to a change in tap position.
* The `control_side` of a transformer regulator should be on the relatively further side to a source in the connected
  grid.

These requirements make sure no edge cases with undefined behavior are encountered.
Typical real-world power grids already satisfy these requirements and they should therefore not cause any problems.

#### Steady state output

| name      | data type | unit | description          |
| --------- | --------- | ---- | -------------------- |
| `tap_pos` | `int8_t`  | -    | optimal tap position |

#### Short circuit output

A `transformer_tap_regulator` has no short circuit output.

#### Electric Model

The transformer tap regulator itself does not have a direct contribution to the grid state.
Instead, it regulates the tap position of the regulated object until the voltage at the control side is in the specified
voltage band:

$$
   \begin{eqnarray}
      U_{\text{control}} \in
         \left[U_{\text{set}} - \frac{U_{\text{band}}}{2}, U_{\text{set}} + \frac{U_{\text{band}}}{2}\right]
   \end{eqnarray}
$$

##### Line drop compensation

The transformer tap regulator tries to regulate the voltage in a specified virtual location in the grid, according to
the folowing model.

```txt
tap_side   control_side                         part of grid where voltage is to be regulated
------^\oo -*---------------------virtual_impedance----*
      |    U_node, I_transformer       Z_comp       U_control
      |                                                |
     regulator <=======================================/
```

The control voltage is the voltage at the node, compensated with the voltage drop corresponding to the specified line
drop compensation.

$$
   \begin{eqnarray}
      & Z_{\text{compensation}} = r_{\text{compensation}} + \mathrm{j} x_{\text{compensation}} \\
      & U_{\text{control}} = \left|\underline{U}_{\text{node}} - \underline{I}_{\text{transformer,out}}
                                 \cdot \underline{Z}_{\text{compensation}}\right|
                           = \left|\underline{U}_{\text{node}} + \underline{I}_{\text{transformer}}
                                 \cdot \underline{Z}_{\text{compensation}}\right|
   \end{eqnarray}
$$

where $\underline{U}_{\text{node}}$ and $\underline{I}_{\text{transformer}}$ are the calculated voltage and current
phasors at the control side and may be obtained from a regular power flow calculation.
The plus sign in the last equality follows from canceling minus signs from the current direction convention and the
compensation direction.

For example, if we want to regulate the voltage at `load_7` in the following grid, the line drop compensation impedance
is the approximate impedance of `line_5`.

```txt
node_1 --- transformer_4 --- node_2 --- line_5 --- node_3
  |          |                                       |
source_6     |                                    load_7
      transformer_tap_regulator_8
```
