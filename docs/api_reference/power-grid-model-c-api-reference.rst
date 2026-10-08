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
Logger
-----

The header `power_grid_model_c/logger.h` contains functions for creating, registering, and reading loggers.

.. doxygenfile:: power_grid_model_c/logger.h


-----
Serialization
-----

The header `power_grid_model_c/serialization.h` contains functions for serializing and deserializing datasets.

.. doxygenfile:: power_grid_model_c/serialization.h


-----
Validation
-----

The header `power_grid_model_c/validation.h` contains the experimental input-validation interface. It is not included
by `power_grid_model_c.h`. The current prototype accepts a non-batch `input` dataset containing `node` and/or `line`
components. The `symmetric` argument must be 0 or 1 and controls whether line zero-sequence fields are required.
Unsupported dataset shapes are reported as handle errors. Invalid node or line data is returned as findings and does
not set a handle error.

`PGM_validation_result_get_json` returns the complete report as UTF-8 JSON bytes. Its `data` pointer is borrowed from
the result and remains valid until `PGM_destroy_validation_result`; `size` is a byte count and excludes the terminating
NUL. Copy the bytes before destroying the result if they need to outlive it. Check the handle after validation and
after reading the report. Both output pointers are required and any non-null output is reset on failure. The report is
serialized lazily on the first call and cached until result destruction; issue storage is released after serialization.
Call `PGM_validation_result_passed` to check for findings without generating or retaining the JSON report. It returns 1
when there are no findings and 0 otherwise; an invalid result also returns 0, so check the handle to distinguish errors.

.. doxygenfile:: power_grid_model_c/validation.h

The report envelope has `schema_version: 1` and an `issues` array, which is empty when there are no findings. A finding
contains a stable string `kind`, `component`, `field`, `ids`, `fields`, `objects`, `reference_component`, and an
extensible `details` object. For example:

.. code-block:: json

	 {
		 "schema_version": 1,
		 "issues": [
			 {
				 "kind": "not_greater_than",
				 "component": "node",
				 "field": "u_rated",
				 "ids": [4, 9],
				 "fields": [{"component": "node", "field": "u_rated"}],
				 "objects": [{"component": "node", "id": 4}, {"component": "node", "id": 9}],
				 "reference_component": null,
				 "details": {"ref_value": 0}
			 }
		 ]
	 }

The current kinds are `missing_value`, `infinity`, `not_unique`, `not_greater_than`, `not_boolean`, `two_values_zero`,
`invalid_id_reference`, and `cross_component_not_unique`. Components and fields are stable strings; `ids` is null when
no IDs apply, otherwise a non-empty array of signed 32-bit IDs. ID arrays preserve duplicate IDs and their order. The
`fields` and `objects` arrays contain component-tagged references to affected fields and objects;
`reference_component` names the target component for invalid ID references and is null otherwise. Consumers should ignore
unrecognized optional attributes, retain unknown kinds as generic findings, and reject unsupported schema versions. A new
finding kind does not itself require a schema-version change; incompatible shape or semantic changes do.

The :download:`v1 JSON Schema <validation-report.schema.json>` describes the current findings. Future kinds can carry
additional constructor data in `details`. `fields` uses `{ "component": name, "field": name }` objects and `objects`
uses `{ "component": name, "id": integer }` objects. Tuple-valued reference values are JSON arrays. Enum values use
`{ "type": "enum", "enum_type": stable_identifier, "value": member }`; each enum type gets a language-neutral stable
identifier. Non-finite floating-point values use
`{ "type": "float", "value": "nan" | "positive_infinity" | "negative_infinity" }`. These forms are reserved for
reference values where JSON has no native representation. JSON is the language-neutral contract; human-readable messages
are not intended to be parsed.

The private Python adapter for this prototype reconstructs existing Python error objects but is not wired into the
public `validate_input_data` function. A Java binding can use the same report: check handle errors, copy exactly `size`
bytes and decode them as UTF-8 while the result is alive, parse the JSON, map known `kind` strings to Java types, retain
unknown findings, and destroy the result in a `finally` block.


-----
Dataset Definitions
-----

The header `power_grid_model_c/dataset_definitions.h` contains extern global pointer variables of all datasets, compoments, and attributes. This header is not included in `power_grid_model_c.h`, you need to include it separately.

.. doxygenfile:: power_grid_model_c/dataset_definitions.h

