<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Getting Started

(Explain how to create input data, instantiate model)

To improve the convenience of initializing an array with given shape and data format, one can use the helper
function {py:class}`power_grid_model.initialize_array`. An example of how to create components is
in [Input data](ex_input_data)

The code below initializes a symmetric load update array with a shape of `(5, 4)`.

```python
from power_grid_model import initialize_array

line_update = initialize_array('update', 'sym_load', (5, 4))
```
