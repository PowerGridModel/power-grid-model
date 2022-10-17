<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

To represent the physical grid components, and the calculation results, this library uses a graph data model. In this
document, the graph data model is presented with the list of all components types, and their relevant input/output
attributes.

# Component Type Hierarchy and Graph Data Model

The components types are organized in an inheritance-like hierarchy. A sub-type has all the attributes from its parent
type. The hierarchy of the component types is shown below.

```{mermaid}
graph LR
    base-->node
    base-->branch
      branch-->line
      branch-->link
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
   class node,line,link,transformer,three_winding_transformer,source,shunt,sym_load,sym_gen,asym_load,asym_gen,sym_voltage_sensor,asym_voltage_sensor,sym_power_sensor,asym_power_sensor green
```

```{note}
The type names in the hierarchy are exactly the same as the component type names in
the {py:class}`power_grid_model.power_grid_meta_data`, see [Native Data Interface](../advanced_documentation/native-data-interface.md)
```

This library uses a graph data model with three generic component types: `node`, `branch`, `branch3` and `appliance`. A
node is similar to a vertex in the graph, a branch is similar to an edge in the graph and a branch3 connects three nodes
together. An appliance is a component which is connected (coupled) to a node, it is seen as a user of this node.

The figure below shows a simple example:

```
node_1 ---line_3 (branch)--- node_2 --------------three_winding_transformer_8 (branch3)------ node_6
 |                             |                                 |
source_5 (appliance)       sym_load_4 (appliance)             node_7
```

* There are four nodes (points/vertices) in the graph of this simple grid.
* The `node_1` and `node_2` are connected by `line_3` which is a branch (edge).
* The `node_2`, `node_6`, and `node_7` are connected by `three_winding_transformer_8` which is a branch3.
* There are two appliances in the grid. The `source_5` is coupled to `node_1` and the `sym_load_4` is coupled
  to `node_2`.

## Symmetry of Components and Calculation

It should be emphasized that the symmetry of components and calculation are two independent concepts in the model. For
example, a power grid model can consist of both `sym_load` and `asym_load`. They are symmetric or asymmetric load
components. On the other hand, the same model can execute symmetric or asymmetric calculations.

* In case of symmetric calculation, the `asym_load` will be treated as a symmetric load by averaging the specified power
  through three phases.
* In case of asymmetric calculation, the `sym_load` will be treated as an asymmetric load by dividing the total
  specified power equally into three phases.

## Attributes of Components

| Attribute    | Description                                                                                                                                                                                                                                                        |
|--------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| name         |  Name   of the attribute. It is exactly the same as the attribute name in   `power_grid_model.power_grid_meta_data`.                                                                                                                                               |
| data type    |  Data type of the attribute. It is either a type from the table in [Native Data Interface](../advanced_documentation/native-data-interface.md#basic-data-types). Or it can be an enumeration as above defined. There are two special data types `RealValueInput` and `RealValueOutput` which are independent. |
|              |     `RealValueInput` is used for some input   attributes. It is a `double` for a symmetric class (e.g. `sym_load`)  and `double[3]` an asymmetric class (e.g.   `asym_load`). It is explained in detail in the corresponding types.                                |
|              |     `RealValueOutput` is used for many output   attributes. It is a `double` in symmetric calculation and `double[3]` for   asymmetric calculation.                                                                                                                |
| unit         |  Unit of the attribute, if it is   applicable. As a general rule, only standard SI units without any prefix are   used.                                                                                                                                            |
| description  |  Description of the attribute.                                                                                                                                                                                                                                     |
| required     |  If the attribute is required. If   not, then it is optional. Note if you choose not to specify an optional   attribute, it should have the null value as defined in [](../advanced_documentation/native-data-interface.md#basic-data-types).                                          |
| input        |  If the attribute is part of an   input dataset.                                                                                                                                                                                                                   |
| update       |  If the attribute can be mutated by   the update call `PowerGridModel.update` on an existing instance, only   applicable when this attribute is part of an input dataset.                                                                                          |
| output       |  If the attribute is part of an   output dataset.                                                                                                                                                                                                                  |
| valid values |  If applicable, an indication which   values are valid for the input data                                                                                                                                                                                          |

## Reference Direction

The sign of active/reactive power of the {ref}`user_manual/components:Appliance` and
{ref}`user_manual/components:Sensor` depends on the reference direction.

* For load reference direction, positive active power means the power flows *from the node to the appliance/sensor*.
* For generator reference direction, positive active power means the power flows *from the appliance/sensor to the node*
  .