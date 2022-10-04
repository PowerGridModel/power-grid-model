<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Data Validation

For performance reasons, the input/update data is not automatically validated. The main validation functions and classes can be included from `power_grid_model.validation`.

Two helper type definitions are used throughout the validation functions, `InputData` and `UpdateData`. They are not 
special types or classes, but merely type hinting aliases:

```python
InputData = Dict[str, np.ndarray]
UpdateData = Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]
```

```{see-also}
Check {ref}`Validation-Examples.ipynb` for an example of function applications.
```

## Manual Validation


```{eval-rst}
.. autoclass:: power_grid_model.validation.errors.ValidationError
   :members:
   :undoc-members:
   :show-inheritance:
.. autofunction:: power_grid_model.validation.validate_input_data

```
```{note}
`validate_input_data()` assumes that you won't be using update data in your calculation.
`validate_batch_data()` validates input_data in combination with batch/update data.
```
```{eval-rst}
.. autofunction:: power_grid_model.validation.validate_batch_data
```

## Assertions

`assert_valid_input_data()` and `assert_valid_batch_data()` raise a ValidationException containing the list/dict of
errors, when the data is invalid.

```{eval-rst}
.. autofunction:: power_grid_model.validation.assert_valid_input_data
.. autofunction:: power_grid_model.validation.assert_valid_batch_data  
```

## Validation utilites

`errors_to_string()` converts a set of errors to a human readable (multi-line) string representation

```{eval-rst}
.. autofunction:: power_grid_model.validation.errors_to_string

```
