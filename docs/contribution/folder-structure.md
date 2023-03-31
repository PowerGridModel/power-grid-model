<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->


# Folder Structure of the Repository

The repository folder structure is as follows. The `docs` and `scripts` folders are self-explanatory.

- The C++ calculation core is inside {{ "[include/power-grid-model]({}/include/power-grid-model)".format(gh_link_head_tree) }}.
- The C-API is inside {{ "[power_grid_model_c]({}/power_grid_model_c)".format(gh_link_head_tree) }}.
- The C program example to use the C-API is inside {{ "[power_grid_model_c_example]({}/power_grid_model_c_example)".format(gh_link_head_tree) }}.
- The python interface code is in {{ "[src/power_grid_model]({}/src/power_grid_model)".format(gh_link_head_tree) }}
- The code for validation of input data is in {{ "[validation]({}/src/power_grid_model/validation)".format(gh_link_head_tree) }} folder.
- The [tests]({{ gh_link_head}}tests) folder is divided in the following way:
  - `cpp_unit_tests` contains the tests for the C++ calculation core.
  - `benchmark_cpp` contains a benchmark test case generator in C++.
  - `unit` folder contains tests for the python code.
  - `data` contains validation test cases designed for every component and algorithm. Some sample network types are also included. 
  The validation is either against popular power system analysis software or hand calculation.
