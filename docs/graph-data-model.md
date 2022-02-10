<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Graph Data Model

To represent the physical grid components, and the calculation results,
this library uses a graph data model.
In this document, the graph data model is presented with the list of all components types,
and their relevant input/output attributes.

# Enumerations

Some attributes of components are enumerations.
The enumerations are implemented using 8-bit signed integer, as explained in
[Native Data Interface](native-data-interface.md).

The table below for a list of enumerations.
They are all defined in the module `power_grid_model.enum`.
The underlying type of enumeration is `int8_t`.

| enum type name in Python | possible values                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    | usage |
| --- |------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------| --- |
| `LoadGenType` | `const_power = 0` <br> `const_impedance = 1` <br> `const_current = 2`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              | load/generation types |
| `WindingType` | `wye = 0` <br> `wye_n = 1` <br> `delta = 2`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        | transformer winding type |
| `BranchSide` | `from_side = 0` <br> `to_side = 1` <br> `side_1 = 2` <br> `side_2 = 3` <br> `side_3 = 4`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           | the side of a branch |
| `MeasuredTerminalType` | `branch_from = 0`, measuring the from-terminal between a branch and a node <br> `branch_to = 1`, measuring the to-terminal between a branch and a node <br> `source = 2`, measuring the terminal between a source and a node <br> `shunt = 3`, measuring the terminal between a shunt and a node <br> `load = 4`, measuring the terminal between a load and a node <br> `generator = 5`, measuring the terminal between a generator and a node <br> `branch3_1 = 6`, measuring the terminal-1 between a branch3 and a node <br> `branch3_2 = 7`, measuring the terminal-2 between a branch3 and a node <br> `branch3_3 = 8`, measuring the terminal-3 between a branch3 and a node | type of flow (e.g. power) measurement |
| `CalculationMethod` | `linear = 0` <br> `newton_raphson = 1` <br> `iterative_linear = 2`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 | method of calculation |


# Component Type Hierarchy and Graph Data Model

The components types are organized in an inheritance-like hierarchy.
A sub-type has all the attributes from its parent type.
The hierarchy of the component types is shown below.

**NOTE: the type names in the hierarchy are exactly the same as the component type names
in the `power_grid_model.power_grid_meta_data`, see [Native Data Interface](native-data-interface.md)**

```
                    base
                     |
                     |
    -----------------------------------------------------------------------------------------
    |           |              |               |                                            |
    |           |              |               |                                            |
   node       branch        branch3         appliance                                       |
                |              |               |                                            |
                |              |               |                                            |
        -------------------    |     ---------------------------------                      |
        |          |      |    |     |          |                    |                      |
        |          |      |    |     |          |                    |                      |
     transformer  line   link  |   source     shunt          generic_load_gen             sensor
                               |                                     |                      |
                               |                                     |                      |
                               |   ----------------------------------------                 |
                               |   |            |            |            |                 |
                               |   |            |            |            |                 |
                               | sym_load   asym_load      sym_gen     asym_gen             |
                               |                                                            |
                               |                            ----------------------------------
                               |                            |                                |
                               |                            |                                |
                    three_winding_transformer      generic_voltage_sensor           generic_power_sensor
                                                            |                                |
                                                            |                                |
                                           --------------------------                --------------------------
                                           |                        |                |                        |
                                           |                        |                |                        |
                                  sym_voltage_sensor      asym_voltage_sensor   sym_power_sensor      asym_power_sensor
```

This library uses a graph data model with three generic component types: `node`, `branch`, and `appliance`.
A node is similar to a vertex in the graph, a branch is similar to an edge in the graph.
An appliance is a component which is connected (coupled) to a node, it is seen as a user of this node.
The figure below shows a simple example:

```
node_1 ---line_3 (branch)--- node_2
 |                             |
source_5 (appliance)       sym_load_4 (appliance)
```

There are two nodes (points/vertices) in the graph of this simple grid.
The two nodes are connected by `line_3` which is a branch (edge).
Furthermore, there are two appliances in the grid.
The `source_5` is coupled to `node_1` and the `sym_load_4` is coupled to `node_2`.

# Symmetry of Components and Calculation

It should be emphasized that the symmetry of components and calculation are two independent concepts in the model.
For example, a power grid model can consist of both `sym_load` and `asym_load`.
They are symmetric or asymmetric load components.
On the other hand, the same model can execute symmetric or asymmetric calculations.
* In case of symmetric calculation, the `asym_load` will be treated as a symmetric load
by averaging the specified power through three phases.
* In case of asymmetric calculation, the `sym_load` will be treated as an asymmetric load
by dividing the total specified power equally into three phases.

# Attributes of Components

The attributes of components are listed in the tables in the sections below.
The column names of the tables are as follows:

* name: name of the attribute.
  It is exactly the same as the attribute name in `power_grid_model.power_grid_meta_data`.
* data type: data type of the attribute.
  It is either a type from the table in [Native Data Interface](native-data-interface.md).
  Or it can be an enumeration as above defined. There is two special data types `RealValueInput` and `RealValueOutput`.
    * `RealValueInput` is used for some input attributes.
      It is a `double` for a symmetric class (e.g. `sym_load`) and `double[3]` an asymmetric class (e.g. `asym_load`).
      It is explained in detail in the corresponding types.
    * `RealValueOutput` is used for many output attributes.
      It is a `double` in symmetric calculation and `double[3]` for asymmetric calculation.
    * As noted above, these two special types are independent.
* unit: unit of the attribute, if it is applicable. As a general rule, only standard SI units without any prefix are used.
* description: description of the attribute.
* required: if the attribute is required. If not, then it is optional.
  Note if you choose not to specify an optional attribute,
  it should have the null value as defined in [Native Data Interface](native-data-interface.md).
* input: if the attribute is part of an input dataset.
* update: if the attribute can be mutated by the update call `PowerGridModel.update` on an existing instance,
  only applicable when this attribute is part of an input dataset.
* output: if the attribute is part of an output dataset.
* valid values: if applicable, an indication which values are valid for the input data

# Validation
For performance reasons, the input/update data is not automatically validated.
There are validation functions available in the `power_grid_model.validation` module:

```python
# Manual validation
#   validate_input_data() assumes that you won't be using update data in your calculation.
#   validate_batch_data() validates input_data in combination with batch/update data.
validate_input_data(input_data, calculation_type, symmetric) -> List[ValidationError]
validate_batch_data(input_data, update_data, calculation_type, symmetric) -> Dict[int, List[ValidationError]]

# Assertions
#   assert_valid_input_data() and assert_valid_batch_data() raise a ValidationException,
#   containing the list/dict of errors, when the data is invalid.
assert_valid_input_data(input_data, calculation_type, symmetric) raises ValidationException
assert_valid_batch_data(input_data, calculation_type, update_data, symmetric) raises ValidationException

# Utilities
#   errors_to_string() converts a set of errors to a human readable (multi-line) string representation
errors_to_string(errors, name, details)
```

Have a look at the Jupyter Notebook "[Validation Examples](../examples/Validation%20Examples.ipynb)" for more information 
on how to apply these functions.

# Component Types

## Base

* type name: `base`

The base type for all power grid components.

| name | data type | unit | description | required | input | update | output |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |
| `id` | `int32_t` | - | ID of a component, the id should be unique along all components, i.e. you cannot have a node with `id` 5 and a line with `id` 5. | &#10004; | &#10004; | &#10060; (id needs to be specified in the update query, but cannot be changed) | &#10004; |
| `energized` | `int8_t` | - | Indicates if a component is energized, i.e. connected to a source | &#10004; | &#10060; | &#10060; | &#10004; |

## Node

* type name: `node`

`node` is a point in the grid. Physically a node can be a busbar, a joint, or other similar component.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `u_rated` | `double` | volt (V) | rated line-line voltage | &#10004; | &#10004; | &#10060; | &#10060; | `> 0` |
| `u_pu` | `RealValueOutput` | - | per-unit voltage magnitude | &#10004; | &#10060; | &#10060; | &#10004; | |
| `u_angle` | `RealValueOutput` | rad | voltage angle | &#10004; | &#10060; | &#10060; | &#10004; | |
| `u` | `RealValueOutput` | volt (V) | voltage magnitude, line-line for symmetric calculation, line-neutral for asymmetric calculation | &#10004; | &#10060; | &#10060; | &#10004; | |


## Branch

* type name: `branch`

`branch` is the abstract base type for the component which connects two *different* nodes.
For each branch two switches are always defined at from- and to-side of the branch.
In reality such switches may not exist.
For example, a cable usually permanently connects two joints.
In this case, the attribute `from_status` and `to_status` is always 1.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `from_node` | `int32_t` | - | ID of node at from-side | &#10004; | &#10004; | &#10060; | &#10060; | a valid node id |
| `to_node` | `int32_t` | - | ID of node at to-side | &#10004; | &#10004; | &#10060; | &#10060; | a valid node id |
| `from_status` | `int8_t` | - | connection status at from-side | &#10004; | &#10004; | &#10004; | &#10060; | `0` or `1` |
| `to_status` | `int8_t` | - | connection status at to-side | &#10004; | &#10004; | &#10004; | &#10060; | `0` or `1` |
| `p_from` | `RealValueOutput` | watt (W) | active power flowing into the branch at from-side | &#10004; | &#10060; | &#10060; | &#10004; | |
| `q_from` | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at from-side | &#10004; | &#10060; | &#10060; | &#10004; | |
| `i_from` | `RealValueOutput` | ampere (A) | current at from-side | &#10004; | &#10060; | &#10060; | &#10004; | |
| `s_from` | `RealValueOutput` | volt-ampere (VA) | apparent power flowing at from-side | &#10004; | &#10060; | &#10060; | &#10004; | |
| `p_to` | `RealValueOutput` | watt (W) | active power flowing into the branch at to-side | &#10004; | &#10060; | &#10060; | &#10004; | |
| `q_to` | `RealValueOutput` | volt-ampere-reactive (var) | reactive power flowing into the branch at to-side | &#10004; | &#10060; | &#10060; | &#10004; | |
| `i_to` | `RealValueOutput` | ampere (A) | current at to-side | &#10004; | &#10060; | &#10060; | &#10004; | |
| `s_to` | `RealValueOutput` | volt-ampere (VA) | apparent power flowing at to-side | &#10004; | &#10060; | &#10060; | &#10004; | |
| `loading` | `double` | - | relative loading of the line, `1.0` meaning 100% loaded. | &#10004; | &#10060; | &#10060; | &#10004; | |


### Line

* type name: 'line'

`line` is a branch with specified serial impedance and shunt admittance. A cable is also modeled as `line`.
A `line` can only connect two nodes with the same rated voltage.

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

`link` usually represents a short internal cable/connection between two busbars inside a substation.
It has a very high admittance (small impedance) which is set to a fixed per-unit value
(equivalent to 10e6 siemens for 10kV network).
There is no additional attribute for `link`.

### Transformer

`transformer` connects two nodes with possibly different voltage levels.

**Note: it can happen that `tap_min > tap_max`.
In this case the winding voltage is decreased if the tap position is increased.**

| name | data type | unit | description | required | input | update | output |                              valid values                              |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |:----------------------------------------------------------------------:|
| `u1` | `double` | volt (V) | rated voltage at from-side | &#10004; | &#10004; | &#10060; | &#10060; |                                 `> 0`                                  |
| `u2` | `double` | volt (V) | rated voltage at to-side | &#10004; | &#10004; | &#10060; | &#10060; |                                 `> 0`                                  |
| `sn` | `double` | volt-ampere (VA) | rated power | &#10004; | &#10004; | &#10060; | &#10060; |                                 `> 0`                                  |
| `uk` | `double` | - | relative short circuit voltage, `0.1` means 10% | &#10004; | &#10004; | &#10060; | &#10060; |                    `>= pk / sn` and `> 0` and `< 1`                    |
| `pk` | `double` | watt (W) | short circuit (copper) loss | &#10004; | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `i0` | `double` | - | relative no-load current | &#10004; | &#10004; | &#10060; | &#10060; |                         `>= p0 / sn` and `< 1`                         |
| `p0` | `double` | watt (W) | no-load (iron) loss | &#10004; | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `winding_from` | `WindingType` | - | from-side winding type | &#10004; | &#10004; | &#10060; | &#10060; |                                                                        |
| `winding_to` | `WindingType` | - | to-side winding type | &#10004; | &#10004; | &#10060; | &#10060; |                                                                        |
| `clock` | `int8_t` | - | clock number of phase shift, odd number is only allowed for Dy(n) or Y(N)d configuration.| &#10004; | &#10004; | &#10060; | &#10060; |                           `>= 0` and `<= 12`                           |
| `tap_side` | `BranchSide` | - | side of tap changer | &#10004; | &#10004; | &#10060; | &#10060; |                        `from_side` or `to_side`                         |
| `tap_pos`   | `int8_t` | - | current position of tap changer | &#10004; | &#10004; | &#10004; | &#10060; | `(tap_min <= tap_pos <= tap_max)` or `(tap_min >= tap_pos >= tap_max)` |
| `tap_min` | `int8_t` | - | position of tap changer at minimum voltage | &#10004; | &#10004; | &#10060; | &#10060; |                                                                        |
| `tap_max` | `int8_t` | - | position of tap changer at maximum voltage | &#10004; | &#10004; | &#10060; | &#10060; |                                                                        |
| `tap_nom`   | `int8_t` | - | nominal position of tap changer | &#10060; default zero | &#10004; | &#10060; | &#10060; | `(tap_min <= tap_nom <= tap_max)` or `(tap_min >= tap_nom >= tap_max)` |
| `tap_size` | `double` | volt (V) | size of each tap of the tap changer | &#10004; | &#10004; | &#10060; | &#10060; |                                 `> 0`                                  |
| `uk_min` | `double` | - | relative short circuit voltage at minimum tap | &#10060; default same as `uk` | &#10004; | &#10060; | &#10060; |                  `>= pk_min / sn` and `> 0` and `< 1`                  |
| `uk_max` | `double` | - | relative short circuit voltage at maximum tap | &#10060; default same as `uk` | &#10004; | &#10060; | &#10060; |                  `>= pk_max / sn` and `> 0` and `< 1`                  |
| `pk_min` | `double` | watt (W) | short circuit (copper) loss at minimum tap | &#10060; default same as `pk` | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `pk_max` | `double` | watt (W) | short circuit (copper) loss at maximum tap | &#10060; default same as `pk` | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `r_grounding_from` | `double` | ohm (Ω) | grounding resistance at from-side, if relevant | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `x_grounding_from` | `double` | ohm (Ω) | grounding reactance at from-side, if relevant | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `r_grounding_to` | `double` | ohm (Ω) | grounding resistance at to-side, if relevant | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `x_grounding_to` | `double` | ohm (Ω) | grounding reactance at to-side, if relevant | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |

## Branch3

* type name: `branch3`

`branch3` is the abstract base type for the component which connects three *different* nodes.
For each branch3 three switches are always defined at side 1, 2, or 3 of the branch.
In reality such switches may not exist.

| name | data type | unit | description | required | input | update | output | valid values |
| -- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
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

`three_winding_transformer` connects three nodes with possibly different voltage levels.

**Note: it can happen that `tap_min > tap_max`.
In this case the winding voltage is decreased if the tap position is increased.**

**TODO modify this attributes**

| name | data type | unit | description | required | input | update | output |                              valid values                              |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |:----------------------------------------------------------------------:|
| `u1` | `double` | volt (V) | rated voltage at from-side | &#10004; | &#10004; | &#10060; | &#10060; |                                 `> 0`                                  |
| `u2` | `double` | volt (V) | rated voltage at to-side | &#10004; | &#10004; | &#10060; | &#10060; |                                 `> 0`                                  |
| `sn` | `double` | volt-ampere (VA) | rated power | &#10004; | &#10004; | &#10060; | &#10060; |                                 `> 0`                                  |
| `uk` | `double` | - | relative short circuit voltage, `0.1` means 10% | &#10004; | &#10004; | &#10060; | &#10060; |                    `>= pk / sn` and `> 0` and `< 1`                    |
| `pk` | `double` | watt (W) | short circuit (copper) loss | &#10004; | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `i0` | `double` | - | relative no-load current | &#10004; | &#10004; | &#10060; | &#10060; |                         `>= p0 / sn` and `< 1`                         |
| `p0` | `double` | watt (W) | no-load (iron) loss | &#10004; | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `winding_from` | `WindingType` | - | from-side winding type | &#10004; | &#10004; | &#10060; | &#10060; |                                                                        |
| `winding_to` | `WindingType` | - | to-side winding type | &#10004; | &#10004; | &#10060; | &#10060; |                                                                        |
| `clock` | `int8_t` | - | clock number of phase shift, odd number is only allowed for Dy(n) or Y(N)d configuration.| &#10004; | &#10004; | &#10060; | &#10060; |                           `>= 0` and `<= 12`                           |
| `tap_side` | `BranchSide` | - | side of tap changer | &#10004; | &#10004; | &#10060; | &#10060; |                        `from_side` or `to_side`                         |
| `tap_pos`   | `int8_t` | - | current position of tap changer | &#10004; | &#10004; | &#10004; | &#10060; | `(tap_min <= tap_pos <= tap_max)` or `(tap_min >= tap_pos >= tap_max)` |
| `tap_min` | `int8_t` | - | position of tap changer at minimum voltage | &#10004; | &#10004; | &#10060; | &#10060; |                                                                        |
| `tap_max` | `int8_t` | - | position of tap changer at maximum voltage | &#10004; | &#10004; | &#10060; | &#10060; |                                                                        |
| `tap_nom`   | `int8_t` | - | nominal position of tap changer | &#10060; default zero | &#10004; | &#10060; | &#10060; | `(tap_min <= tap_nom <= tap_max)` or `(tap_min >= tap_nom >= tap_max)` |
| `tap_size` | `double` | volt (V) | size of each tap of the tap changer | &#10004; | &#10004; | &#10060; | &#10060; |                                 `> 0`                                  |
| `uk_min` | `double` | - | relative short circuit voltage at minimum tap | &#10060; default same as `uk` | &#10004; | &#10060; | &#10060; |                  `>= pk_min / sn` and `> 0` and `< 1`                  |
| `uk_max` | `double` | - | relative short circuit voltage at maximum tap | &#10060; default same as `uk` | &#10004; | &#10060; | &#10060; |                  `>= pk_max / sn` and `> 0` and `< 1`                  |
| `pk_min` | `double` | watt (W) | short circuit (copper) loss at minimum tap | &#10060; default same as `pk` | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `pk_max` | `double` | watt (W) | short circuit (copper) loss at maximum tap | &#10060; default same as `pk` | &#10004; | &#10060; | &#10060; |                                 `>= 0`                                 |
| `r_grounding_from` | `double` | ohm (Ω) | grounding resistance at from-side, if relevant | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `x_grounding_from` | `double` | ohm (Ω) | grounding reactance at from-side, if relevant | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `r_grounding_to` | `double` | ohm (Ω) | grounding resistance at to-side, if relevant | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |
| `x_grounding_to` | `double` | ohm (Ω) | grounding reactance at to-side, if relevant | &#10060; default zero | &#10004; | &#10060; | &#10060; |                                                                        |


## Appliance

* type name: `appliance`

`appliance` is an abstract user which is coupled to a `node`.
For each `appliance` a switch is defined between the `appliance` and the `node`.

**The sign of active/reactive power of the appliance depends on the reference direction.**

* For load reference direction, positive active power means the power flows *from the node to the appliance*.
* For generator reference direction, positive active power means the power flows *from the appliance to the node*.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `node` | `int32_t` | - | ID of the coupled node | &#10004; | &#10004; | &#10060; | &#10060; | a valid node id |
| `status` | `int8_t` | - | connection status to the node | &#10004; | &#10004; | &#10004; | &#10060; | `0` or `1` |
| `p` | `RealValueOutput` | watt (W) | active power | &#10004; | &#10060; | &#10060; | &#10004; | |
| `q` | `RealValueOutput` | volt-ampere-reactive (var) | reactive power | &#10004; | &#10060; | &#10060; | &#10004; | |
| `i` | `RealValueOutput` | ampere (A) | current | &#10004; | &#10060; | &#10060; | &#10004; | |
| `s` | `RealValueOutput` | volt-ampere (VA) | apparent power | &#10004; | &#10060; | &#10060; | &#10004; | |
| `pf` | `RealValueOutput` | - | power factor | &#10004; | &#10060; | &#10060; | &#10004; | |

### Source

* type name: `source`
* reference direction: generator

`source` is representing the external network with a
[Thévenin's equivalence](https://en.wikipedia.org/wiki/Th%C3%A9venin%27s_theorem).
It has an infinite voltage source with an internal impedance.
The impedance is specified by convention as short circuit power.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `u_ref` | `double` | - | reference voltage in per-unit | &#10024; only for power flow | &#10004; | &#10004; | &#10060; | `> 0` | 
| `sk` | `double` | volt-ampere (VA) | short circuit power | &#10060; default 1e10 | &#10004; | &#10060; | &#10060; | `> 0` | 
| `rx_ratio` | `double` | - | R to X ratio | &#10060; default 0.1 | &#10004; | &#10060; | &#10060; | `>= 0` |
| `z01_ratio` | `double` | - | zero sequence to positive sequence impedance ratio | &#10060; default 1.0 | &#10004; | &#10060; | &#10060; | `> 0` | 

### Generic Load and Generator

* type name: `generic_load_gen`

`generic_load_gen` is an abstract load/generation
which contains only the type of the load/generation with response to voltage.

| name | data type | unit | description | required | input | update | output |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |
| `type` | `LoadGenType` | - | type of load/generator with response to voltage | &#10004; | &#10004; | &#10060; | &#10060; |

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
* reference direction: load

`shunt` is an `appliance` with a fixed admittance (impedance).
It behaves similar to a load/generator with type `const_impedance`.

| name | data type | unit | description | required | input | update | output |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |
| `g1` | `double` | siemens (S) | positive-sequence shunt conductance | &#10004; | &#10004; | &#10060; | &#10060; |
| `b1` | `double` | siemens (S) | positive-sequence shunt susceptance | &#10004; | &#10004; | &#10060; | &#10060; |
| `g0` | `double` | siemens (S) | zero-sequence shunt conductance | &#10024; only for asymmetric calculation | &#10004; | &#10060; | &#10060; |
| `b0` | `double` | siemens (S) | zero-sequence shunt susceptance | &#10024; only for asymmetric calculation | &#10004; | &#10060; | &#10060; |


## Sensor

* type name: `sensor`

`sensor` is an abstract type for all the sensor types.
A sensor does not have any physical meaning. Rather, it provides measurement data for the state estimation algorithm.
The state estimator uses the data to evaluate the state of the grid with the highest probability.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `measured_object` | `int32_t` | - | id of the measured object | &#10004; | &#10004; | &#10060; | &#10060; | a valid object id |

### Generic Voltage Sensor

* type name: `generic_voltage_sensor`

`generic_voltage_sensor` is an abstract class for symmetric and asymmetric voltage sensor.
It measures the magnitude and (optionally) the angle of the voltage of a `node`.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `u_sigma` | `double` | volt (V) | standard deviation of the measurement error. Usually this is the absolute measurement error range divided by 3. | &#10024; only for state estimation | &#10004; | &#10004; | &#10060; | `> 0` |


#### Voltage Sensor Concrete Types

There are two concrete types of voltage sensor. They share similar attributes:
the meaning of `RealValueInput` is different, as shown in the table below. In a `sym_voltage_sensor` the measured voltage is a line-to-line voltage.
In a `asym_voltage_sensor` the measured voltage is a 3-phase line-to-ground voltage.

| type name | meaning of `RealValueInput` |
| --- | --- |
| `sym_voltage_sensor` | `double` |
| `asym_voltage_sensor` | `double[3]` |

The table below shows a list of attributes.

| name | data type | unit | description | required | input | update | output | valid values |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: | :---: |
| `u_measured` | `RealValueInput` | volt (V) | measured voltage magnitude | &#10024; only for state estimation | &#10004; | &#10004; | &#10060; | `> 0` |
| `u_angle_measured` | `RealValueInput` | rad | measured voltage angle (only possible with phasor measurement units) |&#10060; | &#10004; | &#10004; | &#10060; | |
| `u_residual` | `RealValueOutput` | volt (V) | residual value between measured voltage magnitude and calculated voltage magnitude | &#10004; | &#10060; | &#10060; | &#10004; | |
| `u_angle_residual` | `RealValueOutput` | rad | residual value between measured voltage angle and calculated voltage angle (only possible with phasor measurement units) | &#10060; | &#10060; | &#10060; | &#10004; | |


### Generic Power Sensor

* type name: `generic_power_sensor`

`power_sensor` is an abstract class for symmetric and asymmetric power sensor.
It measures the active/reactive power flow of a terminal.
The terminal is either connecting an `appliance` and a `node`,
or connecting the from/to end of a `branch` and a `node`.
In case of a terminal between an `appliance` and a `node`,
the power reference direction in the measurement data is the same as the reference direction of the `appliance`.
For example, if a `power_sensor` is measuring a `source`,
a positive `p_measured` indicates that the active power flows from the source to the node.

| name | data type | unit | description | required | input | update | output |                    valid values                     |
| --- | --- | --- | --- | :---: | :---: | :---: | :---: |:---------------------------------------------------:|
| `measured_terminal_type` | `MeasuredTerminalType` | - | indicate if it measures an `appliance` or a `branch`| &#10004; | &#10004; | &#10060; | &#10060; | the terminal type should match the `measured_object` |
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
| `p_residual` | `RealValueOutput` | watt (W) | residual value between measured active power and calculated active power | &#10004; | &#10060; | &#10060; | &#10004; |
| `q_residual` | `RealValueOutput` | volt-ampere-reactive (var) | residual value between measured reactive power and calculated reactive power | &#10004; | &#10060; | &#10060; | &#10004; |
