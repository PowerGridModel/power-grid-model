<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Data Validator

The power-grid-model repository provides an input/update data validator.  For performance reasons, the input/update data is not automatically validated.
However, it would be advisable to validate your data before constructing a PowerGridModel instance. 
An alternative approach would be to validate only when an exception is raised, but be aware that not all data errors will raise exceptions, most of them wil just yield invalid results without warning.
The main validation functions and classes can be included from `power_grid_model.validation`.

Two helper type definitions are used throughout the validation functions, `InputData` and `UpdateData`. They are not 
special types or classes, but merely type hinting aliases:

```python
InputData = Dict[str, np.ndarray]
UpdateData = Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]
```

```{seealso}
Check the [example](examples/Validation%20Examples.ipynb) for an example of function applications.
```


## Validation Errors
Each validation error is an object which can be converted to a compact human-readable message using `str(error)`. 
It contains three member variables `component`, `field` and `ids`, which can be used to gather more specific information about the validation error, 
e.g. which object IDs are involved.

```
class ValidationError:
    
    # Component(s): e.g. "node" or ["node", "line"]
    component: Union[str, List[str]]
    
    # Field(s): e.g. "id" or ["line_from", "line_to"] or [("node", "id"), ("line", "id")]
    field: Union[str, List[str], List[Tuple[str, str]]]

    # IDs: e.g. [1, 2, 3] or [("node", 1), ("line", 1)]
    ids: Union[List[int], List[Tuple[str, int]]] = []    
```

## Validation functions
#### Manual validation
The validation functions below can be used to validate input/batch data manually. The functions require `input_data: InputData `, which is power-grid-model input data,
and `symmetric: bool`, stating if the data will be used for symmetric or asymmetric calculations. `calculation_type: CalculationType` is optional and can be supplied to 
allow missing values for unused fields; see the [API reference](../api_reference/python-api-reference.md#enum) for more information. To validate update/batch data `update_data: UpdateData`,
power-grid-model update data, should also be supplied.
- `validate_input_data(input_data, calculation_type, symmetric) -> List[ValidationError]` validates input_data.
- `validate_batch_data(input_data, update_data, calculation_type, symmetric) -> Dict[int, List[ValidationError]]` validates input_data in combination with batch/update data.

#### Assertions

Instead of manual validation it is possible to use the assertion functions below to assert valid data. In the case of invalid data a `ValidationException` will be raised, 
which is an exception storing the name of the validated data, a list/dict of errors and a convenient conversion to string
to display a summary of all the errors when printing the exception.
- `assert_valid_input_data(input_data, calculation_type, symmetric)` raises `ValidationException`
- `assert_valid_batch_data(input_data, calculation_type, update_data, symmetric)` raises `ValidationException`

#### Utilities
- `errors_to_string(errors, name, details)` converts a set of errors to a human-readable (multi-line) string representation

```{seealso}
For more information on the validation functions please see the  [API reference](../api_reference/python-api-reference.md#validation).
```