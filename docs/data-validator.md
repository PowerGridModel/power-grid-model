<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Validation

For performance reasons, the input/update data is not automatically validated. There are validation functions available
in the `power_grid_model.validation` module:

```python
# Manual validation
#   validate_input_data() assumes that you won't be using update data in your calculation.
#   validate_batch_data() validates input_data in combination with batch/update data.
validate_input_data(input_data, calculation_type, symmetric) -> List[ValidationError]
validate_batch_data(input_data, update_data, calculation_type, symmetric) -> Dict[int, List[ValidationError]]

# Assertions
#   assert_valid_input_data() and assert_valid_batch_data() raise a ValidationException,
#   containing the list/dict of errors, when the data is invalid.
assert_valid_input_data(input_data, calculation_type, symmetric)
raises
ValidationException
assert_valid_batch_data(input_data, calculation_type, update_data, symmetric)
raises
ValidationException

# Utilities
#   errors_to_string() converts a set of errors to a human readable (multi-line) string representation
errors_to_string(errors, name, details)
```

Have a look at the Jupyter Notebook "[Validation Examples](../examples/Validation%20Examples.ipynb)" for more
information on how to apply these functions.
