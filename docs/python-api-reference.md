<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# power_grid_model


 ```{eval-rst}
.. autoclass:: power_grid_model.PowerGridModel
   :members:
   :special-members: __init__
   :show-inheritance:
```

 ```{eval-rst}
.. autofunction:: power_grid_model.initialize_array
```

## Enum

```{eval-rst}
.. automodule:: power_grid_model.enum
   :members:
   :undoc-members:
   :member-order: bysource
   :show-inheritance:
```
```{eval-rst}
.. automodule:: power_grid_model.validation.
   :members:
```

## Validation

```{eval-rst}
.. autoclass:: power_grid_model.validation.errors.ValidationError
   :members:
.. autofunction:: power_grid_model.validation.validate_input_data
```
```{note}
`validate_input_data()` assumes that you won't be using update data in your calculation.
`validate_batch_data()` validates input_data in combination with batch/update data.
```
```{eval-rst}
.. autofunction:: power_grid_model.validation.validate_batch_data
```
```{eval-rst}
.. autofunction:: power_grid_model.validation.assert_valid_input_data
.. autofunction:: power_grid_model.validation.assert_valid_batch_data  
```
```{eval-rst}
.. autofunction:: power_grid_model.validation.errors_to_string

```
