# Validation Reports

The experimental native validator returns schema-version-2 JSON through the C API and the Python
`validate_input_data_json` and `validate_batch_data_json` functions. The JSON schema is available at
[`validation-report.schema.json`](../api_reference/validation-report.schema.json).

## Scope and Status

Input validation returns one result with `scenario: null`. A batch with one or more scenarios returns a base-input
result followed by one result for each scenario. The base result includes both ID checks and value checks. Scenario
value checks run against the base data merged with that scenario's updates, so base value findings can repeat in
scenarios that reach value validation.

ID checks require each supported record to have an `id`, require IDs to be unique within each component, and require
IDs to be unique across components. If a base ID check fails, every scenario is `blocked` with no issues; update
addressing is skipped. Otherwise, unknown, missing, or mismatched update IDs block only the affected scenario and produce
addressing findings. Repeated updates to one ID are applied in order, with the last update taking effect.

A component's updates are positional when every update ID is missing and the record count equals the base component's
record count. Otherwise, updates are matched by ID; missing or unknown IDs are reported. A zero-scenario batch returns
an empty `results` array and performs no input checks; its `valid: true` does not certify the input data.

## Component Coverage

The default native validator checks `node` and `line`. Other components in input or update datasets are silently
ignored. Consequently, `valid: true` means no issues were found in the components the invoked validator checks; it
does not certify ignored components. The C++ component-list template can extend the checked component set.

## JSON Contract

The envelope contains `schema_version`, `validation_type`, `valid`, and `results`. Each result contains `scenario`,
`status`, and `issues`; statuses are `passed`, `failed`, or `blocked`. A passed result has no issues. `valid` is true
only when every returned result passed. A blocked scenario may have no issues when a base ID failure already explains
why it was not checked.

Issues use stable string `kind` values and canonical `fields`, `objects`, and `details`. Object IDs may be `null` when
missing; a zero-based `row` is included when it is needed to identify an object. Consumers should preserve unknown
kinds and additive attributes and reject unsupported schema versions.

The C API reports invalid model data as findings. Invalid arguments and internal failures return `NULL` and set the
handle error. Options are borrowed, must be non-NULL, and currently use only the `symmetric` setting for line rules.
The experimental Python JSON functions return UTF-8 `bytes`; the existing exception-based Python validators are
unchanged.

## Known Follow-Up

Addressing parity with the model is not yet enforced across scenarios. The validator accepts a positional update in
one scenario even if other scenarios use explicit IDs or ragged counts; the model rejects such a dataset through
`validate_update_data_independence`. A follow-up can use
`independence::check_component_independence<C>(update, n_base)` and report `ids_part_na` and
`ids_all_na && !qualify_for_optional_id()` as addressing findings.