<!--
SPDX-FileCopyrightText: Contributors to `power-grid-model` project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Dataset Terminology

Some terms regarding the data structures are explained here, including the definition of dataset, component, and attribute. For detailed data types used throughout `power-grid-model`, please refer to [Python API Reference](../api_reference/python-api-reference.md).

## Data structures

- **Dataset:** Either a single or a batch dataset.
    - **SingleDataset:** A data type storing input data (i.e. all elements of all components) for a single scenario.
    - **BatchDataset:** A data type storing update and or output data for one or more scenarios. A batch dataset can contain sparse or dense data, depending on the component.
- **DataArray** A data array can be a single or a batch array. It is a numpy structured array.     
    - **SingleArray** A dictionary where the keys are the component types and the values are one-dimensional structured numpy arrays.
    - **BatchArray:** An array of dictionaries where the keys are the component types and the values are two-dimensional structured numpy arrays.
        - **DenseBatchArray:** A two-dimensional structured numpy array containing a list of components of the same type for each scenario.
        - **SparseBatchArray:** A dictionary with a one-dimensional numpy int64 array and a one-dimensional structured numpy arrays. 

### Type of Dataset 

The types of `Dataset` include the following: `input`, `update`, `sym_output`, `asym_output`, and `sc_output`:
Exemplery datasets attributes are given in a dataset containing a `line` component.

- **input:** Contains attributes relevant to configuration of grid. 
  - Example: `id`, `from_node`, `from_status`
- **update:** Contains attributes relevant to multiple scenarios. 
  - Example: `from_status`,`to_status`
- **sym_output:** Contains attributes relevant to symmetrical steady state output of power flow or state estimation calculation.
  - Example: `p_from`, `p_to`
- **asym_output:** Contains attributes relevant to asymmetrical steady state output of power flow or state estimation calculation. Attributes are similar to `sym_output` except some values of the asymmetrical dataset will contain detailed data for all 3 phases individually.
  - Example: `p_from`, `p_to`
- **sc_output:** Contains attributes relevant to symmetrical short circuit calculation output. Like for the `asym_output`, detailed data for all 3 phases will be provided where relevant.
  - Example: `i_from`, `i_from_angle`

## Terminologies related to data structures

- **Component:** The definition of a part of a grid: e.g., `node`, `source`, `line`, etc. Check highlighted section of graph in [Component Hierarchy](./data-model.md#component-type-hierarchy-and-graph-data-model)

- **Element:** A single instance of a node, source, line etc.

- **Attribute:** The definition of `id`, `energized`, `p`, etc. of any component.

- **Value:** The value under an attribute, ie. id, energized, p, etc.

- **Array:** All elements of one specific component, for one or more scenarios. I.e. a node array or line array. An array can be one dimensional (containing all elements of a single component for one scenario), two-dimensional (containing all elements of a single component for multiple scenarios), or it can be a sparse array, which is essentially a dictionary with a data buffer and an index pointer.

`power-grid-model` can process many scenarios (e.g. time steps, switch states, etc.) at once, which we call a batch. The batch size is the number of scenarios.

- **Scenario:** A single time step / switch state topology.

- **Batch:** The set of scenarios. (there is only one batch)

- **Batch size:** The total number of scenarios in the batch.

- **n_scenarios:** The total number of scenarios in the batch. (Same as Batch Size)

- **n_component_elements_per_scenario:** The number of elements of a specific component for each scenario. This can be an integer (for dense batches), or a list of integers for sparse batches, where each integer in the list represents the number of elements of a specific component for the scenario corresponding to the index of the integer.

- **Sub-batch:** When computing in parallel, all scenarios in batch calculation are distributed over threads. Each thread handles a subset of the `Batch`, called a `Sub-batch`. This is only used internally in the C++ code.

## Attributes of Components

| Attribute |  Description  |
| ------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| name                     | Name of the attribute. It is exactly the same as the attribute name in `power_grid_model.power_grid_meta_data`.                                                                                                                                                                                              |
| data type                | Data type of the attribute. It is either a type from the table in [Native Data Interface](../advanced_documentation/native-data-interface.md#basic-data-types), or an enumeration as defined above. There are two special data types that are independent from one another; `RealValueInput` and `RealValueOutput`. |
|                          | `RealValueInput` is used for some input attributes. It is a `double` for a symmetric class (e.g. `sym_load`) and `double[3]` an asymmetric class (e.g. `asym_load`). It is explained in detail in the corresponding types.                                                                                   |
|                          | `RealValueOutput` is used for many output attributes. It is a `double` in symmetric calculation and `double[3]` for asymmetric and short circuit calculations.                                                                                                                                               |
| unit                     | Unit of the attribute, if applicable. As a general rule, only standard SI units without any prefix are used.                                                                                                                                                                                           |
| description              | Description of the attribute.                                                                                                                                                                                                                                                                                |
| required                 | Whether the attribute is required. If not, then it is optional. Note if you choose not to specify an optional attribute, it should have the null value as defined in [](../advanced_documentation/native-data-interface.md#basic-data-types).                                                                |
| update                   | Whether the attribute can be mutated by the update call `PowerGridModel.update` on an existing instance, only applicable when this attribute is part of an input dataset.                                                                                                                                    |
| valid values             | Whether applicable or not; an indication of value validity for the input data.   |
