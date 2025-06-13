<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# power_grid_model (Python API)

This is the Python API reference for the `power-grid-model` library.
Due to the nature of how Python module works, we cannnot hide the implementation detail completely from the user.
As a general rule, any Python modules/functions/classes which are not documented in this API documentation,
are internal implementations.
**The user should not use any of them.**
**We do not guarantee the stability or even the existence of those modules.**

```{eval-rst}
.. py:module:: power_grid_model

.. autoclass:: PowerGridModel
   :show-inheritance:
.. autofunction:: initialize_array
.. autofunction:: attribute_dtype
.. autofunction:: attribute_empty_value

.. py:data:: power_grid_meta_data
   :type: typing.PowerGridMetaData

   The data types for all dataset types and components used by the Power Grid Model.
```

## enum

```{eval-rst}
.. automodule:: power_grid_model.enum
   :undoc-members:
   :imported-members:
   :member-order: bysource
   :show-inheritance:
```

## data types

```{eval-rst}
.. automodule:: power_grid_model.data_types
   :undoc-members:
   :imported-members:
   :member-order: bysource
   :show-inheritance:
   :exclude-members: BatchList,NominalValue,RealValue,AsymValue,AttributeValue,Component,ComponentList,SinglePythonDataset,BatchPythonDataset,PythonDataset
```

## error types

```{eval-rst}
.. automodule:: power_grid_model.errors
   :imported-members:
   :member-order: bysource
   :show-inheritance:
```

## typing

Type hints for the power-grid-model library.

This includes all miscellaneous type hints not under dataset or categories.

```{eval-rst}
.. py:module:: power_grid_model.typing
.. autoclass:: power_grid_model.typing.DatasetType
.. autoclass:: power_grid_model.typing.DatasetTypeVar
.. autoclass:: power_grid_model.typing.ComponentType
.. autoclass:: power_grid_model.typing.ComponentTypeVar
.. autoclass:: power_grid_model.typing.ComponentMetaData
.. autoclass:: power_grid_model.typing.DatasetMetaData
.. autoclass:: power_grid_model.typing.PowerGridMetaData
.. autodata:: power_grid_model.typing.ComponentAttributeMapping
   :annotation: ComponentAttributeMapping
```

## validation

```{eval-rst}
.. autofunction:: power_grid_model.validation.validate_input_data
.. autofunction:: power_grid_model.validation.validate_batch_data
.. autofunction:: power_grid_model.validation.assert_valid_input_data
.. autofunction:: power_grid_model.validation.assert_valid_batch_data
.. autofunction:: power_grid_model.validation.assert_valid_data_structure
.. autofunction:: power_grid_model.validation.errors_to_string
.. autoclass:: power_grid_model.validation.ValidationException
```

### validation errors

```{eval-rst}
.. automodule:: power_grid_model.validation.errors
   :undoc-members:
   :show-inheritance:
```

## utils

```{eval-rst}
.. autofunction:: power_grid_model.utils.get_dataset_type
.. autofunction:: power_grid_model.utils.get_dataset_scenario
.. autofunction:: power_grid_model.utils.get_dataset_batch_size
.. autofunction:: power_grid_model.utils.get_component_batch_size
.. autofunction:: power_grid_model.utils.is_columnar
.. autofunction:: power_grid_model.utils.is_sparse
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
.. autofunction:: power_grid_model.utils.self_test
```
