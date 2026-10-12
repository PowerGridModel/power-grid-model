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

The header `power_grid_model_c/validation.h` contains the experimental validation interface. It is not included by
`power_grid_model_c.h`. The current prototype accepts non-batch `input` data and batch `update` data. It validates
`node` and `line`; other components are silently ignored, so `valid: true` covers only the components that were
validated. The `symmetric` argument must be 0 or 1 and controls whether line zero-sequence fields are required. Invalid
dataset types and modes are reported as handle errors. Invalid model data is returned as findings and does not set a
handle error.

`PGM_validation_result_get_json` returns the complete report as UTF-8 JSON bytes. The report is serialized during
validation and owned by the result. Its `data` pointer is borrowed and remains valid until
`PGM_destroy_validation_result`; `size` is a byte count and excludes the terminating NUL. Copy the bytes before
destroying the result if they need to outlive it. Check the handle after validation and after reading the report. Both
output pointers are required and any non-null output is reset on failure. There is no separate pass/fail getter; the
report's `valid` property summarizes its scope statuses.

.. doxygenfile:: power_grid_model_c/validation.h

The report uses schema version 2. Its envelope contains `validation_type`, `valid`, and `results`; each result contains a
`scenario` (`null` for input scope), `status`, and an `issues` array. A finding contains a stable string `kind`, canonical
`fields` and `objects` arrays, and an extensible `details` object. For example:

.. code-block:: json

	 {
	   "schema_version": 2,
	   "validation_type": "input",
	   "valid": false,
	   "results": [
	     {
	       "scenario": null,
	       "status": "failed",
	       "issues": [
	         {
	           "kind": "not_greater_than",
	           "fields": [{"component": "node", "field": "u_rated"}],
	           "objects": [{"component": "node", "id": 4}],
	           "details": {"ref_value": 0}
	         }
	       ]
	     }
	   ]
	 }

The current kinds are `missing_value`, `infinity`, `not_unique`, `not_greater_than`, `not_boolean`, `two_values_zero`,
`invalid_id_reference`, `id_not_in_dataset`, `invalid_update_id`, and `cross_component_not_unique`. Components and fields
are stable strings. Object IDs may be `null` for missing IDs; object references can include a zero-based `row` when the
row is needed to identify a missing, ambiguous, or invalid update target. Reference targets and comparison values are
reported in `details`. Consumers should preserve unknown kinds and additive attributes and reject unsupported schema
versions.

Input validation returns one input scope. Batch validation returns an input scope containing base ID and value findings,
followed by one scope per update scenario. A `passed` scope has no issues; `failed` records findings after checks
complete; `blocked` means scenario value checks did not run because base ID checks or update addressing failed. Base ID
failures block scenarios without addressing updates; base value findings may repeat in scenarios that reach value
validation. A zero-scenario batch returns an empty `results` array and performs no input checks, so its `valid: true`
does not certify input values.

The :download:`v2 JSON Schema <validation-report.schema.json>` describes the report. Unknown properties remain allowed
for forward-compatible additions. Non-finite reference values use tagged objects rather than nonstandard JSON tokens;
human-readable messages are not intended to be parsed.

Python exposes experimental `validate_input_data_json` and `validate_batch_data_json` entry points from
`power_grid_model.validation`. They return UTF-8 `bytes` for both passing and failing reports. The existing
`validate_input_data` and `validate_batch_data` functions retain their exception-based behavior.


-----
Dataset Definitions
-----

The header `power_grid_model_c/dataset_definitions.h` contains extern global pointer variables of all datasets, compoments, and attributes. This header is not included in `power_grid_model_c.h`, you need to include it separately.

.. doxygenfile:: power_grid_model_c/dataset_definitions.h

