.. SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
..
.. SPDX-License-Identifier: MPL-2.0

=====
power_grid_model_c (C API)
=====

This is a C API reference. In the following sections the content of all the headers are shown.

You can include `power_grid_model_c.h` if you want to use all the symbols (except for the global definition pointers of all datasets, components, attributes).

You can also include one or more of the individual headers to use a subset of the functionality.

-----
Main Header
-----

The main header is `power_grid_model_c.h` with all the core functionality.

.. doxygenfile:: power_grid_model_c.h


-----
Basics
-----

The header `power_grid_model_c/basics.h` contains definitions of opaque structs and enumerations.

.. doxygenfile:: power_grid_model_c/basics.h


-----
Handle
-----

The header `power_grid_model_c/handle.h` contains error handling functions.

.. doxygenfile:: power_grid_model_c/handle.h


-----
Meta Data
-----

The header `power_grid_model_c/meta_data.h` contains functions to retrieve meta data.


.. doxygenfile:: power_grid_model_c/meta_data.h


-----
Buffer
-----

The header `power_grid_model_c/buffer.h` contains functions for buffer control.


.. doxygenfile:: power_grid_model_c/buffer.h


-----
Dataset
-----

The header `power_grid_model_c/dataset.h` contains functions for dataset control.

.. doxygenfile:: power_grid_model_c/dataset.h


-----
Options
-----

The header `power_grid_model_c/options.h` contains functions for creating and setting calculation options.

.. doxygenfile:: power_grid_model_c/options.h


-----
Model
-----

The header `power_grid_model_c/model.h` contains functions to create and calculate the main model: Power Grid Model.

.. doxygenfile:: power_grid_model_c/model.h


-----
Serialization
-----

The header `power_grid_model_c/serialization.h` contains functions for serializing and deserializing datasets.

.. doxygenfile:: power_grid_model_c/serialization.h


-----
Dataset Definitions
-----

The header `power_grid_model_c/dataset_definitions.h` contains extern global pointer variables of all datasets, compoments, and attributes. This header is not included in `power_grid_model_c.h`, you need to include it separately.

.. doxygenfile:: power_grid_model_c/dataset_definitions.h

