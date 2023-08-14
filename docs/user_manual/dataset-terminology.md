<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Dataset Terminology

Some terms regarding the data structures are explained here, including the definition of dataset, component, and attribute.

## Data structures

- **SingleDataset:** A data type storing input data (ie. all elements of all components) for a single scenario

- **BatchDataset:** A data type storing update and or output data for one or more scenarios

- **Dataset:** Either a single or a batch dataset

### Type of Dataset 

The type of Dataset. i.e. `input`, `update`, `sym_output`, `asym_output`, `sc_output`.
The examples in brackets are given in context of a dataset of a `line` component.

- **input:** Contains attributes relevant to configuration of grid.(eg. `id`, `from_node`, `from_status`, ...)
- **update:** Contains attributes relevant to multiple scenarios. (eg. `from_status`,`to_status`)
- **sym_output:** Contains attributes relevant to symmetrical steady state output of power flow or state estimation calculation. (eg. `p_from`, `p_to`, ...) 
- **asym_output:** Contains attributes relevant to asymmetrical steady state output of power flow or state estimation calculation. (eg. `p_from`, `p_to`, ...). Attributes are similar to `sym_output` except some values of the asymmetrical dataset will contain detailed data for all 3 phases individually.
- **sc_output:** Contains attributes relevant to symmetrical short circuit calculation output. (eg. `i_from`, `i_from_angle`, ...). Like for the `asym_output`, detailed data for all 3 phases will be provided where relevant.

## Terms regarding data structures

- **Component:** The definition of a part of a grid: e.g. `node`, `source`, `line`, etc. Check highlighted section of graph in [Component Hierarchy](./data-model.md#component-type-hierarchy-and-graph-data-model)

- **Element:** A single instance of a node, source, line etc.

- **Attribute:** The definition of `id`, `energized`, `p`, etc. of any component.

- **Value:** The value under an attribute, ie. id, energized, p, etc.

- **Array:** All elements of one specific component, for one or more scenarios. I.e. a node array or line array. An array can be one dimensional (containing all elements of a single component for one scenario), two-dimensional (containing all elements of a single component for multiple scenarios), or it can be a sparse array, which is essentially a dictionary with a data buffer and an index pointer.

The Power Grid Model can process many scenarios (i.e. time steps, switch states, etc.) at once, which we call a batch. The batch size is the number of scenarios.

- **Scenario:** A single time step / switch state topology.

- **Batch:** The total set of scenarios. (there is only one batch)

- **Batch size:** The total number of scenarios in the batch.

- **n_scenarios:** The total number of scenarios in the batch. (Same as Batch Size)

- **n_component_elements_per_scenario:** The number of elements of a specific component for each scenario. This can be an integer (for dense batches), or a list of integers for sparse batches, where each integer in the list represents the number of elements of a specific component for the scenario corresponding to the index of the integer.

- **Sub-batch:** Only used internally, in the C++ code, when all scenarios in a batch calculation are distributed over multiple threads, so that each thread can handle a sub-batch, to utilize the calculation power of multi-core processors.

## Attributes of Components

| Attribute characteristic | Description                                                                                                                                                                                                                                                                                                  |
| ------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| name                     | Name of the attribute. It is exactly the same as the attribute name in `power_grid_model.power_grid_meta_data`.                                                                                                                                                                                              |
| data type                | Data type of the attribute. It is either a type from the table in [Native Data Interface](../advanced_documentation/native-data-interface.md#basic-data-types). Or it can be an enumeration as above defined. There are two special data types `RealValueInput` and `RealValueOutput` which are independent. |
|                          | `RealValueInput` is used for some input attributes. It is a `double` for a symmetric class (e.g. `sym_load`) and `double[3]` an asymmetric class (e.g. `asym_load`). It is explained in detail in the corresponding types.                                                                                   |
|                          | `RealValueOutput` is used for many output attributes. It is a `double` in symmetric calculation and `double[3]` for asymmetric and short circuit calculations.                                                                                                                                               |
| unit                     | Unit of the attribute, if it is applicable. As a general rule, only standard SI units without any prefix are used.                                                                                                                                                                                           |
| description              | Description of the attribute.                                                                                                                                                                                                                                                                                |
| required                 | Whether the attribute is required. If not, then it is optional. Note if you choose not to specify an optional attribute, it should have the null value as defined in [](../advanced_documentation/native-data-interface.md#basic-data-types).                                                                |
| update                   | Whether the attribute can be mutated by the update call `PowerGridModel.update` on an existing instance, only applicable when this attribute is part of an input dataset.                                                                                                                                    |
| valid values             | Whether applicable, an indication which values are valid for the input data.                                                                                                                                                                                                                                 |
