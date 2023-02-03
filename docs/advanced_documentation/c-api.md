<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Use Power Grid Model through C-API

While many users use Python API for Power Grid Model.
This library also provides a C-API with a dynamic library (`.so` or `.dll`). 
The main use case of C-API is to integrate Power Grid Model into a non-Python application/library, namely, C, C++, JAVA, C#, etc.

You can refer to the [C-API Reference](../api_reference/power-grid-model-c-api-reference.rst)
for a detailed documentation of the API. 
You can also refer to the 
{{ "[header file]({}/include/power_grid_model_c.h)".format(gh_link_head_blob) }}
to explore the declarations of all C-API functions.
Please also have a look at an 
{{ "[example]({}/power_grid_model_c_example/main.c)".format(gh_link_head_blob) }}
of C program to use this C-API.

In this documentation, however, the main design choices and concepts of the C-API are presented.

