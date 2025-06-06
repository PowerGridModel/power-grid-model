<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Folder Structure of the Repository

The repository folder structure is as follows. The `docs` and `scripts` folders are self-explanatory.

- The C++ calculation core is inside
  {{ "[power_grid_model_c/power_grid_model/include/power_grid_model]({}/power_grid_model_c/power_grid_model/include/power_grid_model)".format(gh_link_head_tree) }}.  <!-- markdownlint-disable-line line-length -->
- The C API is inside
  {{ "[power_grid_model_c/power_grid_model_c]({}/power_grid_model_c/power_grid_model_c)".format(gh_link_head_tree) }}.
- The C program example to use the C API is inside
  {{ "[power_grid_model_c_example]({}/power_grid_model_c_example)".format(gh_link_head_tree) }}.
- The python interface code is in {{ "[src/power_grid_model]({}/src/power_grid_model)".format(gh_link_head_tree) }}
- The code for validation of input data is in
  {{ "[validation]({}/src/power_grid_model/validation)".format(gh_link_head_tree) }} folder.
- The {{ "[tests]({}/tests)".format(gh_link_head_tree) }} folder is divided in the following way:
  - `cpp_unit_tests` contains the tests for the C++ calculation core.
  - `cpp_integration_tests` contains more extensive tests for the C++ calculation core.
  - `cpp_validation_tests` contains the validation test cases for the C++ calculation core.
  - `c_api_tests` contains the tests for communication with the C API.
  - `benchmark_cpp` contains a benchmark test case generator in C++.
  - `package_tests` contains an integration test CMake project that tests whether the C API package is correctly
    installed.
  - `unit` folder contains tests for the python code.
  - `data` contains validation test cases designed for every component and algorithm.
    Some sample network types are also included.
    The validation is either against popular power system analysis software or hand calculation.
