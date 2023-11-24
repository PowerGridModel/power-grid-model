<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# power_grid_model


 ```{eval-rst}
.. autoclass:: power_grid_model.PowerGridModel
   :show-inheritance:
.. autofunction:: power_grid_model.initialize_array
```

## enum

```{eval-rst}
.. automodule:: power_grid_model.enum
   :undoc-members:
   :show-inheritance:
```

## validation

```{eval-rst}
.. autofunction:: power_grid_model.validation.validate_input_data
.. autofunction:: power_grid_model.validation.validate_batch_data
.. autofunction:: power_grid_model.validation.assert_valid_input_data
.. autofunction:: power_grid_model.validation.assert_valid_batch_data  
.. autofunction:: power_grid_model.validation.errors_to_string
```

### errors

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
.. autofunction:: power_grid_model._utils.get_and_verify_batch_sizes
.. autofunction:: power_grid_model._utils.get_batch_size
```
