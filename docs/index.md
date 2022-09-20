<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

% power-grid-model documentation master file, created by
% sphinx-quickstart on Fri Sep 16 12:10:59 2022.
% You can adapt this file completely to your liking, but it should at least
% contain the root `toctree` directive.

# power-grid-model

```{warning}
The documentation is under heavy development
```

```{include} ../README.md
```


```{toctree}
:caption: 'Graph Data Model'
:maxdepth: 2
graph-data-model
```
```{toctree}
:caption: 'Native Data Interface'
:maxdepth: 2
native-data-interface
```
```{toctree}
:caption: 'Python API reference (Existing)'
:maxdepth: 2
python-api-reference
```
```{toctree}
:caption: 'Auto API from docstrings'
:maxdepth: 2
power_grid_model
```
## Functions used in examples

```
Extra code
.. autofunction:: power_grid_model.validation.assert_valid_input_data
.. autofunction:: power_grid_model.validation.assert_valid_batch_data
.. autofunction:: power_grid_model.initialize_array
.. autofunction:: power_grid_model.PowerGridModel
.. autofunction:: power_grid_model.PowerGridModel.update
.. autofunction:: power_grid_model.PowerGridModel.copy
.. autofunction:: power_grid_model.PowerGridModel.calculate_power_flow
.. autofunction:: power_grid_model.PowerGridModel.calculate_state_estimation
```


# Indices and tables

- {ref}`genindex`
- {ref}`modindex`
- {ref}`search`
