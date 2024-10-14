<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# power_grid_model (Python API)

This is the Python API reference for the `power-grid-model` library

 ```{eval-rst}
.. autoclass:: power_grid_model.PowerGridModel
   :show-inheritance:
.. autofunction:: power_grid_model.initialize_array
.. autoclass:: power_grid_model.DatasetType
.. autoclass:: power_grid_model.ComponentType
.. autodata:: power_grid_model.power_grid_meta_data
   :no-index:
   :annotation: power_grid_model.core.power_grid_meta.power_grid_meta_data
.. autodata:: power_grid_model.ComponentAttributeMapping
   :annotation: power_grid_model.typing.ComponentAttributeMapping
```

## enum

```{eval-rst}
.. automodule:: power_grid_model.enum
   :undoc-members:
   :show-inheritance:
```

## data types

```{eval-rst}
.. autoclass:: power_grid_model.data_types.Dataset
.. autoclass:: power_grid_model.data_types.SingleDataset
.. autoclass:: power_grid_model.data_types.BatchDataset
.. autoclass:: power_grid_model.data_types.ComponentData
.. autoclass:: power_grid_model.data_types.SingleComponentData
.. autoclass:: power_grid_model.data_types.BatchComponentData
.. autoclass:: power_grid_model.data_types.DataArray
.. autoclass:: power_grid_model.data_types.ColumnarData
.. autoclass:: power_grid_model.data_types.SingleArray
.. autoclass:: power_grid_model.data_types.SingleColumnarData
.. autoclass:: power_grid_model.data_types.BatchArray
.. autoclass:: power_grid_model.data_types.BatchColumnarData
.. autoclass:: power_grid_model.data_types.DenseBatchArray
.. autoclass:: power_grid_model.data_types.SparseBatchArray
```

## error types

```{eval-rst}
.. automodule:: power_grid_model.errors
```

## validation

```{eval-rst}
.. autofunction:: power_grid_model.validation.validate_input_data
.. autofunction:: power_grid_model.validation.validate_batch_data
.. autofunction:: power_grid_model.validation.assert_valid_input_data
.. autofunction:: power_grid_model.validation.assert_valid_batch_data  
.. autofunction:: power_grid_model.validation.errors_to_string
```

### validation errors

```{eval-rst}
.. autoclass:: power_grid_model.validation.errors.ValidationError
```

## utils

```{eval-rst}
.. autofunction:: power_grid_model.utils.get_dataset_type
.. autofunction:: power_grid_model.utils.get_dataset_scenario
.. autofunction:: power_grid_model.utils.json_deserialize
.. autofunction:: power_grid_model.utils.json_serialize
.. autofunction:: power_grid_model.utils.msgpack_deserialize
.. autofunction:: power_grid_model.utils.msgpack_serialize
.. autofunction:: power_grid_model.utils.json_deserialize_from_file
.. autofunction:: power_grid_model.utils.json_serialize_to_file
.. autofunction:: power_grid_model.utils.msgpack_deserialize_from_file
.. autofunction:: power_grid_model.utils.msgpack_serialize_to_file
.. autofunction:: power_grid_model.utils.import_json_data
.. autofunction:: power_grid_model.utils.export_json_data
.. autofunction:: power_grid_model.utils.get_and_verify_batch_sizes
.. autofunction:: power_grid_model.utils.get_batch_size
```
