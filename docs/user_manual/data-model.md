<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Component Type Hierarchy and Graph Data Model

To represent the physical grid components and the calculation results, this library utilizes a graph data model.
In this document, the graph data model is presented with the list of all components types, and their relevant
input/output attributes.

The components types are organized in an inheritance-like hierarchy.
A sub-type has all the attributes from its parent type.
The hierarchy tree of the component types is shown below.

```{mermaid}
graph LR
    base-->node
    base-->branch
      branch-->line
      branch-->link
      branch-->generic_branch
      branch-->transformer      
    base-->branch3
      branch3-->three_winding_transformer
    base-->appliance
      appliance-->generic_load_gen
        generic_load_gen-->sym_load
        generic_load_gen-->sym_gen
        generic_load_gen-->asym_load
        generic_load_gen-->asym_gen
      appliance-->source
      appliance-->shunt
    base-->sensor
      sensor-->generic_voltage_sensor
        generic_voltage_sensor-->sym_voltage_sensor
        generic_voltage_sensor-->asym_voltage_sensor
      sensor-->generic_power_sensor
        generic_power_sensor-->sym_power_sensor
        generic_power_sensor-->asym_power_sensor
     
   classDef green fill:#9f6,stroke:#333,stroke-width:2px
   class node,line,link,generic_branch,transformer,three_winding_transformer,source,shunt,sym_load,sym_gen,asym_load,asym_gen,sym_voltage_sensor,asym_voltage_sensor,sym_power_sensor,asym_power_sensor green
```

```{note}
The type names in the hierarchy are exactly the same as the component type names in
the {py:class}`power_grid_model.power_grid_meta_data`, see
[Native Data Interface](../advanced_documentation/native-data-interface.md).
```

There are four generic component types: `node`, `branch`, `branch3` and `appliance`.
A `node` is similar to a vertex in a graph, a `branch` is similar to an edge in a graph and a `branch3` connects three
nodes together.
An `appliance` is a component that is connected (coupled) to a node, and it is seen as a user of this node.

The figure below shows a simple example:

```txt
node_1 ---line_3 (branch)--- node_2 --------------three_winding_transformer_8 (branch3)------ node_6
 |                             |                                 |
source_5 (appliance)       sym_load_4 (appliance)             node_7
```

* There are four nodes (points/vertices) in the graph of this simple grid.
* `node_1` and `node_2` are connected by `line_3` which is a branch (edge).
* `node_2`, `node_6`, and `node_7` are connected by `three_winding_transformer_8`, which is a `branch3`.
* There are two appliances in the grid.
  `source_5` is coupled to `node_1` and `sym_load_4` is coupled to `node_2`.

## Symmetry of Components and Calculation

It should be emphasized that the symmetry of components and calculation are two independent concepts in the
power-grid-model.
For instance, a model can consist of loads of both `sym_load` and `asym_load` types, which is symmetry on component
level.
Meanwhile, both symmetric and asymmetric calculations can be run on the same model:

* In symmetric calculation, an asymmetric loads will be treated as a symmetric load by averaging the specified power
  through three phases.
* In asymmetric calculation, a symmetric load will be treated as an asymmetric load by dividing the total specified
  power equally into three phases.

## Reference Direction

The sign of active/reactive power of the {ref}`user_manual/components:Branch`, {ref}`user_manual/components:Branch3`,
{ref}`user_manual/components:Appliance` and {ref}`user_manual/components:Sensor` depends on the reference direction.

* For load reference direction, positive active/reactive power means the power flows *from the node to the*
  *appliance/sensor*.
* For generator reference direction, positive active/reactive power means the power flows *from the appliance/sensor to*
  *the node*.
* For `branch` and `branch3` type of components, positive active/reactive power means the power flows *from the node to*
  *the branch*.
