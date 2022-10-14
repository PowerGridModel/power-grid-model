<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Data Validator

```{warning}
[Issue 79](https://github.com/alliander-opensource/power-grid-model/issues/79)
```

For performance reasons, the input/update data is not automatically validated. The main validation functions and classes can be included from `power_grid_model.validation`.

Two helper type definitions are used throughout the validation functions, `InputData` and `UpdateData`. They are not 
special types or classes, but merely type hinting aliases:

```python
InputData = Dict[str, np.ndarray]
UpdateData = Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]
```

```{seealso}
Check [](examples/Validation%20Examples.ipynb) for an example of function applications.
```


## Validation Errors

## Validation functions