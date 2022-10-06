<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Python API


 ```{eval-rst}
.. autoclass:: power_grid_model.PowerGridModel
   :members:
   :special-members: __init__
   :show-inheritance:
```

## Helper functions

To improve the convenience of initializing an array with given shape and data format, one can use the helper
function {py:class}`power_grid_model.initialize_array`. An example of how to create components is
in [Input data](ex_input_data)

 ```{eval-rst}
.. autofunction:: power_grid_model.initialize_array
```

The code below initializes a symmetric load update array with a shape of `(5, 4)`.

```python
from power_grid_model import initialize_array

line_update = initialize_array('update', 'sym_load', (5, 4))
```

## Enumerations

Some attributes of components are enumerations.

```{eval-rst}
.. automodule:: power_grid_model.enum
   :members:
   :undoc-members:
   :member-order: bysource
   :show-inheritance:
```

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
