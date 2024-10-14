<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Dataset Terminology

Some terms regarding the data structures are explained here, including the definition of dataset, component, and attribute. For detailed data types used throughout `power-grid-model`, please refer to [Python API Reference](../api_reference/python-api-reference.md).

## Data structures

```{mermaid}
graph TD
    subgraph Datasets
    Dataset --> SingleDataset
    Dataset --> BatchDataset
    end

    subgraph Dataset values
    ComponentData --> DataArray
    ComponentData --> ColumnarData

    DataArray --> SingleArray
    DataArray --> BatchArray

    BatchArray --> DenseBatchArray
    BatchArray --> SparseBatchArray

    ColumnarData --> SingleColumnarData
    ColumnarData --> BatchColumnarData

    BatchColumnarData --> DenseColumnarData
    BatchColumnarData --> SparseColumnarData
    end

    subgraph Other numpy arrays
    Indptr
    SingleColumn
    BatchColumn
    end
```

- **Dataset:** Either a single or a batch dataset. it is a dictionary with keys as the component types (eg. `line`, `node`, etc) and values as **ComponnentData**
  - **SingleDataset:** A data type storing input data (i.e. all elements of all components) for a single scenario.
  - **BatchDataset:** A data type storing update and or output data for one or more scenarios. A batch dataset can contain sparse or dense data, depending on the component.

- **ComponentData** The data corresponding to the component.
  - **DataArray** A data array can be a single or a batch array. It is a numpy structured array.
    - **SingleArray** A 1D numpy structured array corresponding to a single dataset.
    - **BatchArray:** Multiple batches of data can be represend in sparse or dense forms.
      - **DenseBatchArray:** A 2D structured numpy array containing a list of components of the same type for each scenario.
      - **SparseBatchArray:** A typed dictionary with a 1D numpy array of `Indptr` type under `indptr` key and `SingleArray` under `data` key  which is all components flattened over all batches.
  - **ColumnarData** A dictionary of attributes as keys and individiual numpy arrays as values.
    - **SingleColumnarData** A dictionary of attributes as keys and `SingleColumn` as values in a single dataset.
    - **BatchColumnarData:** Multiple batches of data can be represend in sparse or dense forms.
      - **DenseColumnarData:** A dictionary of attributes as keys and 2D/3D numpy array of `BatchColumn` type as values in a single dataset.
      - **SparseColumnarData:** A typed dictionary with a 1D numpy array of `Indptr` type under `indptr` key and `SingleColumn` under `data` which is all components flattened over all batches.

- **Indptr** A 1D numpy array of int64 type used to specify sparse batches. It indicates the range of components within a scenario. For eg. and indptr of [0, 1, 3, 3] indicates 4 batches with element indexed with 0 in 1st batch, [1, 2, 3] in 2nd batch and no elements in 3rd batch.
- **SingleColumn** A 1D/2D numpy array of values corresponding to a specific attribute.
- **BatchColumn** A 2D/3D numpy array of values corresponding to a specific attribute.

### Dimensions of numpy arrays

The dimensions of numpy arrays and the interpretation of each dimension is as follows.

| **Data Type**            | **1D**                                                                                       | **2D**                                                                                                           | **3D**                                                                                                           |
|--------------------------|----------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------|
| **SingleArray**          | Corresponds to a single dataset.                                                             | ❌                                                                                                               | ❌                                                                                                               |
| **DenseBatchArray**      | ❌                                                                                            | Batch number $\times$ Component within that batch                                                                       | ❌                                                                                                               |
| **SingleColumn**   | Component within that batch.                                                                 | Component within that batch $\times$ Phases &#10024                                                                            | ❌                                                                                                               |
| **BatchColumn**    | ❌                                                                                            | Batch number $\times$ Component within that batch                                                                       | Batch number $\times$ Component within that batch $\times$ Phases &#10024                                                            |

```{note}
&#10024: The "Phases" dimension is optional and is available only when the attributes are asymmetric.
```

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
| data type                | Data type of the attribute. It is either a type from the table in [Native Data Interface](../advanced_documentation/native-data-interface.md#basic-data-types), or an enumeration as defined above. There are two special data types that are independent from one another, namely, `RealValueInput` and `RealValueOutput`. |
|                          | `RealValueInput` is used for some input attributes. It is a `double` for a symmetric class (e.g. `sym_load`) and `double[3]` an asymmetric class (e.g. `asym_load`). It is explained in detail in the corresponding types.                                                                                   |
|                          | `RealValueOutput` is used for many output attributes. It is a `double` in symmetric calculation and `double[3]` for asymmetric and short circuit calculations.                                                                                                                                               |
| unit                     | Unit of the attribute, if applicable. As a general rule, only standard SI units without any prefix are used.                                                                                                                                                                                           |
| description              | Description of the attribute.                                                                                                                                                                                                                                                                                |
| required                 | Whether the attribute is required. If not, then it is optional. Note if you choose not to specify an optional attribute, it should have the null value as defined in [](../advanced_documentation/native-data-interface.md#basic-data-types).                                                                |
| update                   | Whether the attribute can be mutated by the update call `PowerGridModel.update` on an existing instance, only applicable when this attribute is part of an input dataset.                                                                                                                                    |
| valid values             | Whether applicable or not; an indication of value validity for the input data.   |
