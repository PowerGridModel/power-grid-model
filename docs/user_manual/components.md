<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Components

The attributes of components are listed in the tables in the sections below. The column names of the tables are as
follows:

## Base

* type name: `base`

The base type for all power grid components.

| name | data type | unit | description | required | input | update | output |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |
| `id` | `int32_t` | - | ID of a component, the id should be unique along all components, i.e. you cannot have a node with `id` 5 and a line with `id` 5. | &#10004; | &#10004; | &#10060; (id needs to be specified in the update query, but cannot be changed) | &#10004; |
| `energized` | `int8_t` | - | Indicates if a component is energized, i.e. connected to a source | | &#10060; | &#10060; | &#10004; |

## Node

* type name: `node`
* base: {hoverxreftooltip}`user_manual/components:base`

`node` is a point in the grid. Physically a node can be a busbar, a joint, or other similar component.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `u_rated` | `double` | volt (V) | rated line-line voltage | &#10004; | &#10004; | &#10060; | &#10060; | `> 0` |
| `u_pu` | `RealValueOutput` | - | per-unit voltage magnitude | | &#10060; | &#10060; | &#10004; | |
| `u_angle` | `RealValueOutput` | rad | voltage angle | | &#10060; | &#10060; | &#10004; | |
| `u` | `RealValueOutput` | volt (V) | voltage magnitude, line-line for symmetric calculation, line-neutral for asymmetric calculation | | &#10060; | &#10060; | &#10004; | |

## Branch

* type name: `branch`
* base: {hoverxreftooltip}`user_manual/components:base`

`branch` is the abstract base type for the component which connects two *different* nodes. For each branch two switches
are always defined at from- and to-side of the branch. In reality such switches may not exist. For example, a cable
usually permanently connects two joints. In this case, the attribute `from_status` and `to_status` is always 1.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `from_node` | `int32_t` | - | ID of node at from-side | &#10004; | &#10004; | &#10060; | &#10060; | a valid node id |
| `to_node` | `int32_t` | - | ID of node at to-side | &#10004; | &#10004; | &#10060; | &#10060; | a valid node id |
| `from_status` | `int8_t` | - | connection status at from-side | &#10004; | &#10004; | &#10004; | &#10060; | `0` or `1` |
| `to_status` | `int8_t` | - | connection status at to-side | &#10004; | &#10004; | &#10004; | &#10060; | `0` or `1` |
| `p_from` | `RealValueOutput` | watt (W) | active power flowing into the branch at from-side | | &#10060; | &#10060; | &#10004; | |
| `q_from` | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at from-side | | &#10060; | &#10060; | &#10004; | |
| `i_from` | `RealValueOutput` | ampere (A) | current at from-side | | &#10060; | &#10060; | &#10004; | |
| `s_from` | `RealValueOutput` | volt-ampere (VA) | apparent power flowing at from-side | | &#10060; | &#10060; | &#10004; | |
| `p_to` | `RealValueOutput` | watt (W) | active power flowing into the branch at to-side | | &#10060; | &#10060; | &#10004; | |
| `q_to` | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at to-side | | &#10060; | &#10060; | &#10004; | |
| `i_to` | `RealValueOutput` | ampere (A) | current at to-side | | &#10060; | &#10060; | &#10004; | |
| `s_to` | `RealValueOutput` | volt-ampere (VA) | apparent power flowing at to-side | | &#10060; | &#10060; | &#10004; | |
| `loading` | `double` | - | relative loading of the line, `1.0` meaning 100% loaded. | | &#10060; | &#10060; | &#10004; | |

### Line

* type name: 'line'

`line` is a {hoverxreftooltip}`user_manual/components:branch` with specified serial impedance and shunt admittance. A cable is
also modeled as `line`. A `line` can only connect two nodes with the same rated voltage.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `r1` | `double` | ohm (Ω) | positive-sequence serial resistance | &#10004; | &#10004; | &#10060; | &#10060; | `r1` and `x1` cannot be both zero |
| `x1` | `double` | ohm (Ω) | positive-sequence serial reactance | &#10004; | &#10004; | &#10060; | &#10060; | `r1` and `x1` cannot be both zero |
| `c1` | `double` | farad (F) | positive-sequence shunt capacitance | &#10004; | &#10004; | &#10060; | &#10060; | |
| `tan1` | `double` | - | positive-sequence shunt loss factor (tan𝛿) | &#10004; | &#10004; | &#10060; | &#10060; | |
| `r0` | `double` | ohm (Ω) | zero-sequence serial resistance | &#10024; only for asymmetric calculations | &#10004; | &#10060; | &#10060; | `r0` and `x0` cannot be both zero |
| `x0` | `double` | ohm (Ω) | zero-sequence serial reactance | &#10024; only for asymmetric calculations | &#10004; | &#10060; | &#10060; | `r0` and `x0` cannot be both zero |
| `c0` | `double` | farad (F) | zero-sequence shunt capacitance | &#10024; only for asymmetric calculations | &#10004; | &#10060; | &#10060; | |
| `tan0` | `double` | - | zero-sequence shunt loss factor (tan𝛿) | &#10024; only for asymmetric calculations | &#10004; | &#10060; | &#10060; | |
| `i_n` | `double` | ampere (A) | rated current | &#10004; | &#10004; | &#10060; | &#10060; | `> 0` |

### Link

* type name: `link`

`link` is a {hoverxreftooltip}`user_manual/components:branch` which usually represents a short internal cable/connection between
two busbars inside a substation. It has a very high admittance (small impedance) which is set to a fixed per-unit value
(equivalent to 10e6 siemens for 10kV network). Therefore, it is chosen by design that no sensors can be coupled to a `link`.
There is no additional attribute for `link`.

### Transformer

`transformer` is a {hoverxreftooltip}`user_manual/components:branch` which connects two nodes with possibly different voltage
levels.

```{note} 
it can happen that `tap_min > tap_max`. In this case the winding voltage is decreased if the tap position is
increased.
```

| name | data type | unit | description                                                                                                                                                                                                                        | required | input | update | output | valid values |
| --- | --- | --- |------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------| :---: | :---: | :---: | :---: | :---: |
| `u1` | `double` | volt (V) | rated voltage at from-side                                                                                                                                                                                                         | &#10004; | &#10004; | &#10060; | &#10060; | `> 0` |
| `u2` | `double` | volt (V) | rated voltage at to-side                                                                                                                                                                                                           | &#10004; | &#10004; | &#10060; | &#10060; | `> 0` |
| `sn` | `double` | volt-ampere (VA) | rated power                                                                                                                                                                                                                        | &#10004; | &#10004; | &#10060; | &#10060; | `> 0` |
| `uk` | `double` | - | relative short circuit voltage, `0.1` means 10%                                                                                                                                                                                    | &#10004; | &#10004; | &#10060; | &#10060; | `>= pk / sn` and `> 0` and `< 1` |
| `pk` | `double` | watt (W) | short circuit (copper) loss                                                                                                                                                                                                        | &#10004; | &#10004; | &#10060; | &#10060; | `>= 0` |
| `i0` | `double` | - | relative no-load current                                                                                                                                                                                                           | &#10004; | &#10004; | &#10060; | &#10060; | `>= p0 / sn` and `< 1` |
| `p0` | `double` | watt (W) | no-load (iron) loss                                                                                                                                                                                                                | &#10004; | &#10004; | &#10060; | &#10060; | `>= 0` |
| `winding_from` | {py:class}`WindingType <power_grid_model.enum.WindingType>` | - | from-side winding type                                                                                                                                                                                                             | &#10004; | &#10004; | &#10060; | &#10060; | |
| `winding_to` | {py:class}`WindingType <power_grid_model.enum.WindingType>` | - | to-side winding type                                                                                                                                                                                                               | &#10004; | &#10004; | &#10060; | &#10060; | |
| `clock` | `int8_t` | - | clock number of phase shift. <br> Even number is not possible if one side is Y(N) winding and the other side is not Y(N) winding. <br> Odd number is not possible, if both sides are Y(N) winding or both sides are not Y(N) winding. | &#10004; | &#10004; | &#10060; | &#10060; | `>= 0` and `<= 12` |
| `tap_side` | {py:class}`BranchSide <power_grid_model.enum.BranchSide>` | - | side of tap changer                                                                                                                                                                                                                | &#10004; | &#10004; | &#10060; | &#10060; | |
| `tap_pos`   | `int8_t` | - | current position of tap changer                                                                                                                                                                                                    | &#10004; | &#10004; | &#10004; | &#10060; |  `(tap_min <= tap_pos <= tap_max)` or `(tap_min >= tap_pos >= tap_max)` |
| `tap_min` | `int8_t` | - | position of tap changer at minimum voltage                                                                                                                                                                                         | &#10004; | &#10004; | &#10060; | &#10060; | |
| `tap_max` | `int8_t` | - | position of tap changer at maximum voltage                                                                                                                                                                                         | &#10004; | &#10004; | &#10060; | &#10060; | |
| `tap_nom`   | `int8_t` | - | nominal position of tap changer                                                                                                                                                                                                    | &#10060; default zero | &#10004; | &#10060; | &#10060; | `(tap_min <= tap_nom <= tap_max)` or `(tap_min >= tap_nom >= tap_max)` |
| `tap_size` | `double` | volt (V) | size of each tap of the tap changer                                                                                                                                                                                                | &#10004; | &#10004; | &#10060; | &#10060; | `>= 0` |
| `uk_min` | `double` | - | relative short circuit voltage at minimum tap                                                                                                                                                                                      | &#10060; default same as `uk` | &#10004; | &#10060; | &#10060; | `>= pk_min / sn` and `> 0` and `< 1` |
| `uk_max` | `double` | - | relative short circuit voltage at maximum tap                                                                                                                                                                                      | &#10060; default same as `uk` | &#10004; | &#10060; | &#10060; | `>= pk_max / sn` and `> 0` and `< 1` |
| `pk_min` | `double` | watt (W) | short circuit (copper) loss at minimum tap                                                                                                                                                                                         | &#10060; default same as `pk` | &#10004; | &#10060; | &#10060; | `>= 0` |
| `pk_max` | `double` | watt (W) | short circuit (copper) loss at maximum tap                                                                                                                                                                                         | &#10060; default same as `pk` | &#10004; | &#10060; | &#10060; | `>= 0` |
| `r_grounding_from` | `double` | ohm (Ω) | grounding resistance at from-side, if relevant                                                                                                                                                                                     | &#10060; default zero | &#10004; | &#10060; | &#10060; | |
| `x_grounding_from` | `double` | ohm (Ω) | grounding reactance at from-side, if relevant                                                                                                                                                                                      | &#10060; default zero | &#10004; | &#10060; | &#10060; | |
| `r_grounding_to` | `double` | ohm (Ω) | grounding resistance at to-side, if relevant                                                                                                                                                                                       | &#10060; default zero | &#10004; | &#10060; | &#10060; | |
| `x_grounding_to` | `double` | ohm (Ω) | grounding reactance at to-side, if relevant                                                                                                                                                                                        | &#10060; default zero | &#10004; | &#10060; | &#10060; | |

## Branch3

* type name: `branch3`
* base: {hoverxreftooltip}`user_manual/components:base`

`branch3` is the abstract base type for the component which connects three *different* nodes. For each branch3 three
switches are always defined at side 1, 2, or 3 of the branch. In reality such switches may not exist.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `node_1` | `int32_t` | - | ID of node at side 1 | &#10004; | &#10004; | &#10060; | &#10060; | a valid node id |
| `node_2` | `int32_t` | - | ID of node at side 2 | &#10004; | &#10004; | &#10060; | &#10060; | a valid node id |
| `node_3` | `int32_t` | - | ID of node at side 3 | &#10004; | &#10004; | &#10060; | &#10060; | a valid node id |
| `status_1` | `int8_t` | - | connection status at side 1 | &#10004; | &#10004; | &#10004; | &#10060; | `0` or `1` |
| `status_2` | `int8_t` | - | connection status at side 2 | &#10004; | &#10004; | &#10004; | &#10060; | `0` or `1` |
| `status_3` | `int8_t` | - | connection status at side 3 | &#10004; | &#10004; | &#10004; | &#10060; | `0` or `1` |
| `p_1` | `RealValueOutput` | watt (W) | active power flowing into the branch at side 1 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `q_1` | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at side 1 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `i_1` | `RealValueOutput` | ampere (A) | current at side 1 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `s_1` | `RealValueOutput` | volt-ampere (VA) | apparent power flowing at side 1 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `p_2` | `RealValueOutput` | watt (W) | active power flowing into the branch at side 2 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `q_2` | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at side 2 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `i_2` | `RealValueOutput` | ampere (A) | current at side 2 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `s_2` | `RealValueOutput` | volt-ampere (VA) | apparent power flowing at side 2 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `p_3` | `RealValueOutput` | watt (W) | active power flowing into the branch at side 3 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `q_3` | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at side 3 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `i_3` | `RealValueOutput` | ampere (A) | current at side 3 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `s_3` | `RealValueOutput` | volt-ampere (VA) | apparent power flowing at side 3 | &#10004; | &#10060; | &#10060; | &#10004; | |
| `loading` | `double` | - | relative loading of the branch, `1.0` meaning 100% loaded. | &#10004; | &#10060; | &#10060; | &#10004; | |

### Three-Winding Transformer

`three_winding_transformer` is a {hoverxreftooltip}`user_manual/components:branch3` connects three nodes with possibly different
voltage levels.

```{note}
It can happen that `tap_min > tap_max`. In this case the winding voltage is decreased if the tap position is
increased.
```

| name        | data type | unit | description                                                                                               |             required             | input | update | output |                   valid values                   |
|-------------| --- | --- |-----------------------------------------------------------------------------------------------------------|:--------------------------------:| :---: | :---: | :---: |:------------------------------------------------:|
| `u1`        | `double` | volt (V) | rated voltage at side 1                                                                                   |             &#10004;             | &#10004; | &#10060; | &#10060; |                      `> 0`                       |
| `u2`        | `double` | volt (V) | rated voltage at side 2                                                                                   |             &#10004;             | &#10004; | &#10060; | &#10060; |                      `> 0`                       |
| `u3`        | `double` | volt (V) | rated voltage at side 3                                                                                   |             &#10004;             | &#10004; | &#10060; | &#10060; |                      `> 0`                       |
| `sn_1`      | `double` | volt-ampere (VA) | rated power at side 1                                                                                     |             &#10004;             | &#10004; | &#10060; | &#10060; |                      `> 0`                       |
| `sn_2`      | `double` | volt-ampere (VA) | rated power at side 2                                                                                     |             &#10004;             | &#10004; | &#10060; | &#10060; |                      `> 0`                       |
| `sn_3`      | `double` | volt-ampere (VA) | rated power at side 3                                                                                     |             &#10004;             | &#10004; | &#10060; | &#10060; |                      `> 0`                       |
| `uk_12`     | `double` | - | relative short circuit voltage across side 1-2, `0.1` means 10%                                           |             &#10004;             | &#10004; | &#10060; | &#10060; | `>= pk_12 / min(sn_1, sn_2)` and `> 0` and `< 1` |
| `uk_13`     | `double` | - | relative short circuit voltage across side 1-3, `0.1` means 10%                                           |             &#10004;             | &#10004; | &#10060; | &#10060; | `>= pk_13 / min(sn_1, sn_3)` and `> 0` and `< 1` |
| `uk_23`     | `double` | - | relative short circuit voltage across side 2-3, `0.1` means 10%                                           |             &#10004;             | &#10004; | &#10060; | &#10060; | `>= pk_23 / min(sn_2, sn_3)` and `> 0` and `< 1` |
| `pk_12`     | `double` | watt (W) | short circuit (copper) loss across side 1-2                                                               |             &#10004;             | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `pk_13`     | `double` | watt (W) | short circuit (copper) loss across side 1-3                                                               |             &#10004;             | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `pk_23`     | `double` | watt (W) | short circuit (copper) loss across side 2-3                                                               |             &#10004;             | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `i0`        | `double` | - | relative no-load current with respect to side 1                                                           |             &#10004;             | &#10004; | &#10060; | &#10060; |                         `>= p0 / sn` and `< 1`                         |
| `p0`        | `double` | watt (W) | no-load (iron) loss                                                                                       |             &#10004;             | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `winding_1` | {py:class}`WindingType <power_grid_model.enum.WindingType>` | - | side 1 winding type                                                                                       |             &#10004;             | &#10004; | &#10060; | &#10060; |                                                                        |
| `winding_2` | {py:class}`WindingType <power_grid_model.enum.WindingType>` | - | side 2 winding type                                                                                       |             &#10004;             | &#10004; | &#10060; | &#10060; |                                                                        |
| `winding_3` | {py:class}`WindingType <power_grid_model.enum.WindingType>` | - | side 3 winding type                                                                                       |             &#10004;             | &#10004; | &#10060; | &#10060; |                                                                        |
| `clock_12`  | `int8_t` | - | clock number of phase shift across side 1-2, odd number is only allowed for Dy(n) or Y(N)d configuration. |             &#10004;             | &#10004; | &#10060; | &#10060; |                           `>= 0` and `<= 12`                           |
| `clock_13`  | `int8_t` | - | clock number of phase shift across side 1-3, odd number is only allowed for Dy(n) or Y(N)d configuration. |             &#10004;             | &#10004; | &#10060; | &#10060; |                           `>= 0` and `<= 12`                           |
| `tap_side` | {py:class}`Branch3Side <power_grid_model.enum.Branch3Side>` | - | side of tap changer                                                                                       |             &#10004;             | &#10004; | &#10060; | &#10060; |                        `side_1` or `side_2` or `side_3`                         |
| `tap_pos`   | `int8_t` | - | current position of tap changer                                                                           |             &#10004;             | &#10004; | &#10004; | &#10060; | `(tap_min <= tap_pos <= tap_max)` or `(tap_min >= tap_pos >= tap_max)` |
| `tap_min` | `int8_t` | - | position of tap changer at minimum voltage                                                                |             &#10004;             | &#10004; | &#10060; | &#10060; |                                                                        |
| `tap_max` | `int8_t` | - | position of tap changer at maximum voltage                                                                |             &#10004;             | &#10004; | &#10060; | &#10060; |                                                                        |
| `tap_nom`   | `int8_t` | - | nominal position of tap changer                                                                           |      &#10060; default zero       | &#10004; | &#10060; | &#10060; | `(tap_min <= tap_nom <= tap_max)` or `(tap_min >= tap_nom >= tap_max)` |
| `tap_size` | `double` | volt (V) | size of each tap of the tap changer                                                                       |             &#10004;             | &#10004; | &#10060; | &#10060; |                                 `> 0`                                  |
| `uk_12_min` | `double` | - | relative short circuit voltage at minimum tap, across side 1-2                                            | &#10060; default same as `uk_12` | &#10004; | &#10060; | &#10060; |                  `>= pk_12_min / min(sn_1, sn_2)` and `> 0` and `< 1`                  |
| `uk_12_max` | `double` | - | relative short circuit voltage at maximum tap, across side 1-2                                            | &#10060; default same as `uk_12` | &#10004; | &#10060; | &#10060; |                  `>= pk_12_max / min(sn_1, sn_2)` and `> 0` and `< 1`                  |
| `pk_12_min` | `double` | watt (W) | short circuit (copper) loss at minimum tap, across side 1-2                                               | &#10060; default same as `pk_12` | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `pk_12_max` | `double` | watt (W) | short circuit (copper) loss at maximum tap, across side 1-2                                               | &#10060; default same as `pk_12` | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `uk_13_min` | `double` | - | relative short circuit voltage at minimum tap, across side 1-3                                            | &#10060; default same as `uk_13` | &#10004; | &#10060; | &#10060; |                  `>= pk_13_min / min(sn_1, sn_3)` and `> 0` and `< 1`                  |
| `uk_13_max` | `double` | - | relative short circuit voltage at maximum tap, across side 1-3                                            | &#10060; default same as `uk_13` | &#10004; | &#10060; | &#10060; |                  `>= pk_13_max / min(sn_1, sn_3)` and `> 0` and `< 1`                  |
| `pk_13_min` | `double` | watt (W) | short circuit (copper) loss at minimum tap, across side 1-3                                               | &#10060; default same as `pk_13` | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `pk_13_max` | `double` | watt (W) | short circuit (copper) loss at maximum tap, across side 1-3                                               | &#10060; default same as `pk_13` | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `uk_23_min` | `double` | - | relative short circuit voltage at minimum tap, across side 2-3                                            | &#10060; default same as `uk_23` | &#10004; | &#10060; | &#10060; |                  `>= pk_23_min / min(sn_2, sn_3)` and `> 0` and `< 1`                  |
| `uk_23_max` | `double` | - | relative short circuit voltage at maximum tap, across side 2-3                                            | &#10060; default same as `uk_23` | &#10004; | &#10060; | &#10060; |                  `>= pk_23_max / min(sn_2, sn_3)` and `> 0` and `< 1`                  |
| `pk_23_min` | `double` | watt (W) | short circuit (copper) loss at minimum tap, across side 2-3                                               | &#10060; default same as `pk_23` | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `pk_23_max` | `double` | watt (W) | short circuit (copper) loss at maximum tap, across side 2-3                                               | &#10060; default same as `pk_23` | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `r_grounding_1` | `double` | ohm (Ω) | grounding resistance at side 1, if relevant                                                               | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `x_grounding_1` | `double` | ohm (Ω) | grounding reactance at side 1, if relevant                                                                | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `r_grounding_2` | `double` | ohm (Ω) | grounding resistance at side 2, if relevant                                                               | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `x_grounding_2` | `double` | ohm (Ω) | grounding reactance at side 2, if relevant                                                                | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `r_grounding_3` | `double` | ohm (Ω) | grounding resistance at side 3, if relevant                                                               | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `x_grounding_3` | `double` | ohm (Ω) | grounding reactance at side 3, if relevant                                                                | &#10060; default zero | &#10004; | &#10060; | &#10060; |

## Appliance

* type name: `appliance`
* base: {hoverxreftooltip}`user_manual/components:base`

`appliance` is an abstract user which is coupled to a `node`. For each `appliance` a switch is defined between
the `appliance` and the `node`. The reference direction for power flows is mentioned in
{hoverxreftooltip}`user_manual/data-model:Reference Direction`.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `node` | `int32_t` | - | ID of the coupled node | &#10004; | &#10004; | &#10060; | &#10060; | a valid node id |
| `status` | `int8_t` | - | connection status to the node | &#10004; | &#10004; | &#10004; | &#10060; | `0` or `1` |
| `p` | `RealValueOutput` | watt (W) | active power | | &#10060; | &#10060; | &#10004; | |
| `q` | `RealValueOutput` | volt-ampere-reactive (var) | reactive power | | &#10060; | &#10060; | &#10004; | |
| `i` | `RealValueOutput` | ampere (A) | current | | &#10060; | &#10060; | &#10004; | |
| `s` | `RealValueOutput` | volt-ampere (VA) | apparent power | | &#10060; | &#10060; | &#10004; | |
| `pf` | `RealValueOutput` | - | power factor | | &#10060; | &#10060; | &#10004; | |

### Source

* type name: `source`
* {hoverxreftooltip}`user_manual/data-model:Reference Direction`: generator

`source` is an {hoverxreftooltip}`user_manual/components:appliance` representing the external network with a
[Thévenin's equivalence](https://en.wikipedia.org/wiki/Th%C3%A9venin%27s_theorem). It has an infinite voltage source
with an internal impedance. The impedance is specified by convention as short circuit power.

| name          | data type | unit | description                                        |           required           | input | update | output | valid values |
|---------------| --- | --- |----------------------------------------------------|:----------------------------:| :---: | :---: | :---: |:------------:|
| `u_ref`       | `double` | - | reference voltage in per-unit                      | &#10024; only for power flow | &#10004; | &#10004; | &#10060; |    `> 0`     | 
| `u_ref_angle` | `double` | rad | reference voltage angle                            |     &#10060; default 0.0     | &#10004; | &#10004; | &#10060; |              |
| `sk`          | `double` | volt-ampere (VA) | short circuit power                                |    &#10060; default 1e10     | &#10004; | &#10060; | &#10060; | `> 0` | 
| `rx_ratio`    | `double` | - | R to X ratio                                       |     &#10060; default 0.1     | &#10004; | &#10060; | &#10060; | `>= 0` |
| `z01_ratio`   | `double` | - | zero sequence to positive sequence impedance ratio |     &#10060; default 1.0     | &#10004; | &#10060; | &#10060; | `> 0` | 

### Generic Load and Generator

* type name: `generic_load_gen`

`generic_load_gen` is an abstract load/generation {hoverxreftooltip}`user_manual/components:appliance` which contains only the
type of the load/generation with response to voltage.

| name | data type | unit | description | required | input | update | output |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |
| `type` | {py:class}`LoadGenType <power_grid_model.enum.LoadGenType>` | - | type of load/generator with response to voltage | &#10004; | &#10004; | &#10060; | &#10060; |

#### Load/Generator Concrete Types

There are four concrete types of load/generator. They share similar attributes: specified active/reactive power.
However, the reference direction and meaning of `RealValueInput` is different, as shown in the table below.

| type name | reference direction | meaning of `RealValueInput` |
| --- | --- | --- |
| `sym_load` | load | `double` |
| `sym_gen` | generator | `double` |
| `asym_load` | load | `double[3]` |
| `asym_gen` | generator | `double[3]` |

The table below shows a list of attributes.

| name | data type | unit | description | required | input | update | output |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |
| `p_specified` | `RealValueInput` | watt (W) | specified active power | &#10024; only for power flow  | &#10004; | &#10004; | &#10060; |
| `q_specified` | `RealValueInput` | volt-ampere-reactive (var) | specified reactive power | &#10024; only for power flow | &#10004; | &#10004; | &#10060; |

### Shunt

* type name: `shunt`
* {hoverxreftooltip}`user_manual/data-model:Reference Direction`: load

`shunt` is an {hoverxreftooltip}`user_manual/components:appliance` with a fixed admittance (impedance). It behaves similar to a
load/generator with type `const_impedance`.

| name | data type | unit | description | required | input | update | output |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |
| `g1` | `double` | siemens (S) | positive-sequence shunt conductance | &#10004; | &#10004; | &#10060; | &#10060; |
| `b1` | `double` | siemens (S) | positive-sequence shunt susceptance | &#10004; | &#10004; | &#10060; | &#10060; |
| `g0` | `double` | siemens (S) | zero-sequence shunt conductance | &#10024; only for asymmetric calculation | &#10004; | &#10060; | &#10060; |
| `b0` | `double` | siemens (S) | zero-sequence shunt susceptance | &#10024; only for asymmetric calculation | &#10004; | &#10060; | &#10060; |

## Sensor

* type name: `sensor`
* base: {hoverxreftooltip}`user_manual/components:base`

`sensor` is an abstract type for all the sensor types. A sensor does not have any physical meaning. Rather, it provides
measurement data for the state estimation algorithm. The state estimator uses the data to evaluate the state of the grid
with the highest probability.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `measured_object` | `int32_t` | - | id of the measured object | &#10004; | &#10004; | &#10060; | &#10060; | a valid object id |

### Generic Voltage Sensor

* type name: `generic_voltage_sensor`

`generic_voltage_sensor` is an abstract class for symmetric and asymmetric voltage sensor and derived from
{hoverxreftooltip}`user_manual/components:sensor`. It measures the magnitude and (optionally) the angle of the voltage of
a `node`.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `u_sigma` | `double` | volt (V) | standard deviation of the measurement error. Usually this is the absolute measurement error range divided by 3. | &#10024; only for state estimation | &#10004; | &#10004; | &#10060; | `> 0` |

#### Voltage Sensor Concrete Types

There are two concrete types of voltage sensor. They share similar attributes:
the meaning of `RealValueInput` is different, as shown in the table below. In a `sym_voltage_sensor` the measured
voltage is a line-to-line voltage. In a `asym_voltage_sensor` the measured voltage is a 3-phase line-to-ground voltage.

| type name | meaning of `RealValueInput` |
| --- | --- |
| `sym_voltage_sensor` | `double` |
| `asym_voltage_sensor` | `double[3]` |

The table below shows a list of attributes.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `u_measured` | `RealValueInput` | volt (V) | measured voltage magnitude | &#10024; only for state estimation | &#10004; | &#10004; | &#10060; | `> 0` |
| `u_angle_measured` | `RealValueInput` | rad | measured voltage angle (only possible with phasor measurement units) |&#10060; | &#10004; | &#10004; | &#10060; | |
| `u_residual` | `RealValueOutput` | volt (V) | residual value between measured voltage magnitude and calculated voltage magnitude | | &#10060; | &#10060; | &#10004; | |
| `u_angle_residual` | `RealValueOutput` | rad | residual value between measured voltage angle and calculated voltage angle (only possible with phasor measurement units) | | &#10060; | &#10060; | &#10004; | |

### Generic Power Sensor

* type name: `generic_power_sensor`

`power_sensor` is an abstract class for symmetric and asymmetric power sensor and is derived from
{hoverxreftooltip}`user_manual/components:sensor`. It measures the active/reactive power flow of a terminal. The terminal is
either connecting an `appliance` and a `node`, or connecting the from/to end of a `branch` (except `link`) and a `node`. In case of a
terminal between an `appliance` and a `node`, the power {hoverxreftooltip}`user_manual/data-model:Reference Direction` in the
measurement data is the same as the reference direction of the `appliance`. For example, if a `power_sensor` is
measuring a `source`, a positive `p_measured` indicates that the active power flows from the source to the node.

Note: due to the high admittance of a `link` it is chosen that a power sensor cannot be coupled to a `link`, even though a link is a `branch`

| name | data type | unit | description | required | input | update | output |                    valid values                     |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |:---------------------------------------------------:|
| `measured_terminal_type` | {py:class}`MeasuredTerminalType <power_grid_model.enum.MeasuredTerminalType>` | - | indicate if it measures an `appliance` or a `branch`| &#10004; | &#10004; | &#10060; | &#10060; | the terminal type should match the `measured_object` |
| `power_sigma` | `double` | volt-ampere (VA) | standard deviation of the measurement error. Usually this is the absolute measurement error range divided by 3. | &#10024; only for state estimation| &#10004; | &#10004; | &#10060; |                        `> 0`                        |

#### Power Sensor Concrete Types

There are two concrete types of power sensor. They share similar attributes:
the meaning of `RealValueInput` is different, as shown in the table below.

| type name | meaning of `RealValueInput` |
| --- | --- |
| `sym_power_sensor` | `double` |
| `asym_power_sensor` | `double[3]` |

The table below shows a list of attributes.

| name | data type | unit | description | required | input | update | output |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |
| `p_measured` | `RealValueInput` | watt (W) | measured active power | &#10024; only for state estimation | &#10004; | &#10004; | &#10060; |
| `q_measured` | `RealValueInput` | volt-ampere-reactive (var) | measured reactive power | &#10024; only for state estimation | &#10004; | &#10004; | &#10060; |
| `p_residual` | `RealValueOutput` | watt (W) | residual value between measured active power and calculated active power | | &#10060; | &#10060; | &#10004; |
| `q_residual` | `RealValueOutput` | volt-ampere-reactive (var) | residual value between measured reactive power and calculated reactive power | | &#10060; | &#10060; | &#10004; |

