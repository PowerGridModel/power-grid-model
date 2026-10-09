# Validation Reporting Interface Options

The comparison below records the earlier goal of reconstructing Python validation errors from native findings.
That goal is superseded: the new validator will return JSON, not Python exception objects, and the existing Python
validator will be deprecated separately. See [the redesign proposal](#redesign-proposal) for the recommended architecture,
review findings, and implementation plan. This document proposes changes; it does not describe an implemented revamp.
The alternative interfaces below are historical design options, not a proposal to expose all of them simultaneously.
Export macros and C++ linkage boilerplate are omitted for readability.

The current experimental [validation header](../../power_grid_model_c/power_grid_model_c/include/power_grid_model_c/validation.h)
exposes input and batch validation, pass/fail, JSON retrieval, and result destruction.
Both validation functions accept a borrowed, non-NULL `PGM_Options const*`. Node and line validation currently uses
only its `symmetric` setting, which must be 0 or 1; the other calculation settings do not affect these validation rules.
Other interfaces shown here are hypothetical. With equivalent options, no reporting alternative changes which model
data is valid.

## Comparison

| Option | Main advantage | Main limitation |
| --- | --- | --- |
| Typed C getters | Explicit types; direct access to selected attributes | More attributes require more functions and binding changes |
| JSON report | Extensible, language-neutral, easy to inspect | Serialization and parsing costs; a schema is still required |
| Strings, consumer-defined regexes | Small interface; directly displayable messages | Fragile parsing; messages may omit required data |
| Strings with producer-supplied regexes | Producer supplies the matching pattern | Custom protocol; capture semantics, nested values, and portability remain difficult |
| Pass/fail with JSON logger reporting | Minimal validation return value; structured diagnostics | Report retrieval and call correlation depend on logger state |
| Generic typed property access | Fixed function family; extensible properties | Dynamic schema plus many small FFI calls |
| Structured finding callback | Findings can be delivered incrementally | Payload representation, callback lifetimes, and threading must be specified |
| MessagePack or CBOR report | Structured binary representation | Decoder dependency; less inspectable; performance must be measured |

## Shared Conventions

The result-based alternatives use these declarations:

```c
#include <stdint.h>
#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/logger.h"

typedef struct PGM_ValidationResult PGM_ValidationResult;

PGM_ValidationResult* PGM_validate_input_data(
    PGM_Handle* handle,
    PGM_ConstDataset const* dataset,
    PGM_Options const* options);

PGM_ValidationResult* PGM_validate_batch_data(
    PGM_Handle* handle,
    PGM_ConstDataset const* input_dataset,
    PGM_ConstDataset const* update_dataset,
    PGM_Options const* options);

PGM_Idx PGM_validation_result_passed(
    PGM_Handle* handle, PGM_ValidationResult const* result);

void PGM_destroy_validation_result(PGM_ValidationResult* result);
```

- The options pointer is borrowed for the duration of validation and must not be NULL. Its `symmetric` setting must be
    0 or 1; node and line validation does not currently use the other calculation settings.
- Invalid model data produces findings, not a handle error. Invalid arguments and internal failures set a handle error.
  Check `PGM_error_code` before interpreting a returned value; zero alone does not distinguish failed validation from
  an operational failure.
- The caller owns a returned result and destroys it once. Result-based borrowed pointers remain valid until destruction
  unless a section explicitly specifies a shorter lifetime.
- Byte lengths exclude a terminating NUL. Use the supplied length rather than assuming arbitrary buffers are terminated.
- Use a documented data contract for stable kinds, required attributes, optional additions, and unknown kinds.
  A smaller C function surface does not eliminate schema maintenance.

The running example is a `not_greater_than` finding for `node.u_rated`, IDs `[3, 14]`, and reference value `0`.

## 1. Typed C Getters

One function retrieves each attribute. This is the original getter-family approach.

```c
typedef struct PGM_ValidationIssue PGM_ValidationIssue;

enum PGM_ValidationIssueKind {
    PGM_validation_issue_unknown = 0,
    PGM_validation_issue_missing_value = 1,
    PGM_validation_issue_infinity = 2,
    PGM_validation_issue_not_unique = 3,
    PGM_validation_issue_not_greater_than = 4
};

PGM_Idx PGM_validation_result_issue_count(
    PGM_Handle* handle, PGM_ValidationResult const* result);
PGM_ValidationIssue const* PGM_validation_result_get_issue(
    PGM_Handle* handle, PGM_ValidationResult const* result, PGM_Idx index);

enum PGM_ValidationIssueKind PGM_validation_issue_get_kind(
    PGM_Handle* handle, PGM_ValidationIssue const* issue);
char const* PGM_validation_issue_get_component(
    PGM_Handle* handle, PGM_ValidationIssue const* issue);
char const* PGM_validation_issue_get_field(
    PGM_Handle* handle, PGM_ValidationIssue const* issue);
PGM_Idx PGM_validation_issue_get_id_count(
    PGM_Handle* handle, PGM_ValidationIssue const* issue);
PGM_ID const* PGM_validation_issue_get_ids(
    PGM_Handle* handle, PGM_ValidationIssue const* issue);
double PGM_validation_issue_get_numeric_ref_value(
    PGM_Handle* handle, PGM_ValidationIssue const* issue);
```

The binding retrieves the issue, reads its kind and common attributes, then invokes kind-specific getters.
Issue handles, strings, and ID arrays are borrowed from the result. Calling an inapplicable getter is an argument error.

The numeric reference getter handles this example only. String or tuple references, filters, enum types, and
cross-component fields need further functions or another representation. Adding getters can preserve existing ABI,
but each new function still needs language bindings.

## 2. JSON Report

Retrieve the complete structured report in one operation after validation.

```c
void PGM_validation_result_get_json(
    PGM_Handle* handle,
    PGM_ValidationResult const* result,
    char const** data,
    PGM_Idx* size);
```

This getter exists in the current experimental interface. The output is borrowed UTF-8 JSON owned by the result.
The consumer copies the bytes before destruction, parses JSON, and maps stable kind strings to its own error classes.
Operational getter failures reset the outputs and set the handle error.

```json
{
  "schema_version": 1,
  "issues": [
    {
      "kind": "not_greater_than",
      "component": "node",
      "field": "u_rated",
      "ids": [3, 14],
      "details": {"ref_value": 0}
    }
  ]
}
```

For a single-input report, an empty `issues` array means validation passed. The existing
[JSON Schema](../api_reference/validation-report.schema.json) describes the current report.
New attributes can be added without new C functions. Unknown kinds must be retained or explicitly rejected,
not silently discarded. Nonfinite values and enum identifiers require explicit representations rather than
nonstandard JSON tokens or Python-specific objects.

Batch validation uses a separate envelope. `data` preserves every zero-based scenario position, including passing
scenarios, and groups findings by component. An empty scenario is `{}`; a batch with zero scenarios has an empty
`data` array. The current batch validator supports node and line input/update components. Node updates check IDs but
do not change values; line updates change only `from_status` and `to_status`. Each scenario is merged independently
with the unchanged input dataset.

```json
{
    "schema_version": 1,
    "is_batch": true,
    "data": [
        {
            "line": [
                {
                    "kind": "not_boolean",
                    "component": "line",
                    "field": "from_status",
                    "ids": [12],
                    "details": {}
                }
            ]
        },
        {
            "line": [
                {
                    "kind": "id_not_in_dataset",
                    "component": "line",
                    "field": "id",
                    "ids": [99],
                    "details": {"ref_dataset": "update"}
                }
            ]
        },
        {}
    ]
}
```

`invalid_update_id` findings use `ids: null` when the update component is absent from the input or positional IDs
cannot be matched by row count. `id_not_in_dataset` reports unknown update IDs in `ids`. Both kinds can be reconstructed
as the corresponding Python batch validation errors.

## 3. Strings With Consumer-Defined Regexes

Return one human-readable message per finding; the consumer supplies its own parsing rules.

```c
PGM_Idx PGM_validation_result_issue_count(
    PGM_Handle* handle, PGM_ValidationResult const* result);

void PGM_validation_result_get_message(
    PGM_Handle* handle,
    PGM_ValidationResult const* result,
    PGM_Idx index,
    char const** message,
    PGM_Idx* size);
```

Example borrowed UTF-8 message:

```text
node.u_rated is not greater than 0; ids=[3,14]
```

The consumer can display this directly or match it with a locally maintained regex.
The library provides no machine-readable capture contract. Changes in wording, escaping, localization, or list syntax
can break reconstruction. Some human messages contain counts rather than IDs, making reconstruction impossible.
If the text is instead given a stable, lossless grammar, this becomes a custom serialization format rather than
ordinary message parsing.

## 4. Strings With Producer-Supplied Regexes

Return a message, its rule identifier, and a corresponding matching pattern.

```c
PGM_Idx PGM_validation_result_issue_count(
    PGM_Handle* handle, PGM_ValidationResult const* result);

void PGM_validation_result_get_message_and_regex(
    PGM_Handle* handle,
    PGM_ValidationResult const* result,
    PGM_Idx index,
    char const** rule,
    char const** message,
    PGM_Idx* message_size,
    char const** regex,
    PGM_Idx* regex_size);
```

All strings are borrowed from the result; `rule` is NUL-terminated. For the example message above:

```text
rule: not_greater_than
regex: ^([a-z_]+)\.([a-z_]+) is not greater than (-?[0-9]+); ids=\[([0-9,-]*)\]$
```

The rule contract must identify the captures: group 1 is component, group 2 is field, group 3 is reference value,
and group 4 is the encoded ID list. Matching still leaves type conversion and ID-list parsing to the consumer.
This pattern handles the example only, not arbitrary reference expressions or richer findings.

A real interface needs a versioned capture contract, escaping rules, match-cost limits, and a specified regex dialect.
Python and Java regex engines differ; supplying a pattern does not automatically make it portable.
This is a custom serialization protocol with regex as one decoding step.

## 5. Pass/Fail With JSON Logger Reporting

Return only the validation outcome and accumulate structured findings in a registered validation logger.
Its output operation delivers a complete UTF-8 JSON report to the callback, rather than human-readable log lines.

```c
PGM_Idx PGM_validate_input_data_logged(
    PGM_Handle* handle,
    PGM_ConstDataset const* dataset,
    PGM_Options const* options);

void consume_validation_json(char const* json, PGM_Idx size, void* user_data);

PGM_Logger* PGM_create_logger(PGM_Handle* handle, PGM_Idx logger_type);
void PGM_register_logger(PGM_Handle* handle, PGM_Logger* logger);
void PGM_logger_clear_content(PGM_Handle* handle, PGM_Logger* logger);
void PGM_logger_get_output(
    PGM_Handle* handle,
    PGM_Logger* logger,
    PGM_LogOutputCallback callback,
    void* user_data);
void PGM_unregister_logger(PGM_Handle* handle, PGM_Logger* logger);
void PGM_destroy_logger(PGM_Logger* logger);
```

The logger lifecycle and output operations already exist. The logged validation call, its options argument, and a
dedicated validation-JSON logger type are proposed additions; existing text loggers do not supply this report format.
The existing `PGM_validation_result_passed` operation also exists, but a standalone logged validation call would
avoid requiring a result handle solely to read pass/fail. Return 1 for valid data and 0 for invalid data; check the
handle to distinguish an operational failure. This alternative does not acquire a validation result.

The caller creates/registers a logger, clears or isolates its contents, validates, checks the handle, retrieves the
report with `consume_validation_json` as the callback, then unregisters/destroys the logger.
`PGM_logger_get_output` invokes the callback once, synchronously, with one JSON document using the envelope and schema
from option 2. A successful validation with no findings produces an empty `issues` array, not an empty byte buffer.
The consumer checks the output operation's handle error as well as the validation call's error.

Output bytes supplied to `PGM_LogOutputCallback` are valid only during that callback. Copy them there for later parsing,
or parse them during the callback without retaining borrowed storage. Catch Python/Java exceptions inside the callback
and propagate them only after control returns to the binding.

This supports reconstruction of typed error objects just like option 2; the difference is report ownership and delivery
through logger state rather than a validation result. The logger must isolate each validation report from prior calls
and unrelated diagnostics. Complete reporting must not depend on log level, filtering, or truncation.
Obey the logger's existing concurrency restrictions. Accumulating a report in the logger still requires storage and
JSON serialization; eliminating the result handle does not eliminate that work.

## 6. Generic Typed Property Access

Expose a tree of opaque values. Functions depend on value types, not individual validation attributes.
The tree need not be materialized as JSON or converted to text.

```c
typedef struct PGM_Value PGM_Value;

enum PGM_ValueKind {
    PGM_value_invalid = 0,
    PGM_value_null,
    PGM_value_object,
    PGM_value_array,
    PGM_value_integer,
    PGM_value_number,
    PGM_value_string,
    PGM_value_boolean
};

PGM_Value const* PGM_validation_result_get_value(
    PGM_Handle* handle, PGM_ValidationResult const* result);
enum PGM_ValueKind PGM_value_get_kind(PGM_Handle* handle, PGM_Value const* value);

PGM_Value const* PGM_value_get_property(
    PGM_Handle* handle, PGM_Value const* object, char const* name);
PGM_Idx PGM_value_get_property_count(PGM_Handle* handle, PGM_Value const* object);
char const* PGM_value_get_property_name(
    PGM_Handle* handle, PGM_Value const* object, PGM_Idx index);

PGM_Idx PGM_value_get_array_size(PGM_Handle* handle, PGM_Value const* array);
PGM_Value const* PGM_value_get_array_item(
    PGM_Handle* handle, PGM_Value const* array, PGM_Idx index);

int64_t PGM_value_get_integer(PGM_Handle* handle, PGM_Value const* value);
double PGM_value_get_number(PGM_Handle* handle, PGM_Value const* value);
PGM_Idx PGM_value_get_boolean(PGM_Handle* handle, PGM_Value const* value);
void PGM_value_get_string(
    PGM_Handle* handle, PGM_Value const* value, char const** data, PGM_Idx* size);
```

For single-input validation, the root exposes the same logical envelope as the JSON example: `schema_version` and
`issues`. Batch validation uses `schema_version`, `is_batch`, and the scenario-indexed `data` array described above.
Reading the reference value conceptually looks like this:

```text
root      = get_value(result)
issues    = get_property(root, "issues")
issue     = get_array_item(issues, 0)
details   = get_property(issue, "details")
reference = get_property(details, "ref_value")
value     = get_integer(reference)
```

All values, property names, and string buffers are borrowed from the result. A missing property returns NULL without
an error; an explicitly null property returns a non-NULL value of kind `PGM_value_null`.
Wrong types, invalid indices, and invalid handles set a handle error. Scalar fallback values are not authoritative
until the caller checks that error. Integer and floating-point accessors have distinct, documented type semantics.

Adding `filters`, `ref_components`, or nested field/ID references needs no new function. Bindings can recursively
enumerate and preserve unknown properties. However, the property schema still evolves, and reconstructing a whole
report requires many foreign-function calls. This is attractive for selective native inspection, less so when Python
and Java normally read every finding.

## 7. Structured Finding Callback

Deliver each finding to a callback instead of exposing a retained result. This example reuses the generic value
representation from option 6; callbacks are a delivery mechanism, not a substitute for a payload format.

```c
typedef struct PGM_Value PGM_Value;
typedef void (*PGM_ValidationIssueCallback)(PGM_Value const* issue, void* user_data);

PGM_Idx PGM_validate_input_data_with_callback(
    PGM_Handle* handle,
    PGM_Options const* options,
    PGM_ConstDataset const* dataset,
    PGM_ValidationIssueCallback callback,
    void* user_data);
```

Invoke the non-NULL callback synchronously on the calling thread, once per finding, before the call returns.
Each `issue` is the finding object itself, not a complete report envelope; its kind and property schema are documented
for this interface version. The consumer may use the generic accessors only during the callback and must copy values
it wants to retain. The callback and `user_data` remain alive for the entire validation call.

Return 1 if there are no findings and 0 otherwise; operational failures set the handle error. If such a failure occurs
after callbacks have fired, consumers must treat the collected findings as incomplete.
Python/Java exceptions must be caught inside their callback and must not unwind through C; defer re-raising until
control returns to the binding. This illustration has no cancellation or reentrant validation support.

The consumer can construct error objects as findings arrive, avoiding a complete serialized report.
Callbacks do not guarantee constant-memory validation: rules may still require aggregation to identify findings.
Native callback registration, threading, and exception containment are additional binding complexity.

## 8. MessagePack or CBOR Report

Use the same logical report as JSON, encoded as bytes rather than text.

```c
enum PGM_ValidationReportFormat {
    PGM_validation_report_json = 0,
    PGM_validation_report_msgpack = 1,
    PGM_validation_report_cbor = 2
};

void PGM_validation_result_get_report(
    PGM_Handle* handle,
    PGM_ValidationResult const* result,
    enum PGM_ValidationReportFormat format,
    char const** data,
    PGM_Idx* size);
```

The data is borrowed from the result, and the size is a byte count. Specify that retrieving another format does not
invalidate earlier borrowed buffers; an implementation can maintain separate cached encodings.
Unsupported formats set a handle error rather than returning a different encoding silently.

MessagePack already has serialization support in this repository; a validation-report encoder would still be needed.
CBOR is another possible format, not an existing validation capability implied by this example.
Python and Java need suitable decoders. Retain the version, stable kinds, and extension policy from the JSON contract,
and define consistent enum/nonfinite representations across encodings. Binary encoding does not remove schema work.
Measure complete producer/consumer workloads before claiming smaller buffers or lower latency.

## Redesign Proposal

### Scope and History

This review compares the working tree with `80d5482d1`, immediately before line validation. It includes:

- `037ae3111`: line rules, node references, cross-component uniqueness, and richer issue payloads.
- `048772686`: JSON reporting, replacement of typed C issue getters, schema documentation, and the private Python adapter.
- Uncommitted changes: batch validation, update-ID matching, scenario merging, batch JSON, and corresponding tests.

The uncommitted changes are part of the review, not changes introduced by this proposal.
The native validator still supports only node and line. The Python validator supports a much wider model and
calculation-dependent rules. Deprecation does not imply that the native validator is already a drop-in replacement.

### Current Architecture

The current flow is:

```text
ConstDataset -> input orchestration -> node/line composers -> reusable rules -> Issues
Issues or BatchIssues -> PGM_ValidationResult -> lazy JSON serialization
JSON -> private Python decoder -> existing ValidationError objects
```

Keep the useful separation between reusable rules, component composers, dataset traversal, and C error handling.
The C wrapper already distinguishes data findings from operational failures through `call_with_catch`.
The row/columnar traversal and scenario extraction already exist in `ConstDataset`; they do not need replacement.

### Review: Standards and Architectural Risks

These are design judgements, not demonstrated violations of a documented repository coding standard.

1. **Component growth requires coordinated edits in unrelated layers.** `validate_input_dataset` and
     `get_supported_components` repeat the node/line presence logic. `cross_unique` names both components directly.
     Batch collection and merging repeat the same list, while the batch schema and Python decoder also hardcode it.
     A third component therefore requires orchestration, update, reporting, schema, and decoding edits. Removing only
     `has_node` and `has_line` would not address the underlying ownership problem.
2. **The result handle owns a state machine rather than a finished result.** It stores `passed_`, `is_batch_`, two
     alternative issue containers, a JSON string, and a mutable `once_flag`. Reading JSON serializes it and clears the
     original issues. `call_once` makes that lazy operation safe against concurrent initialization; it is not itself
     evidence of a race. However, JSON-only consumers do not need this lifecycle, delayed serialization failures, or
     synchronization. Eager serialization trades some work on passing datasets for a much simpler contract.
3. **The issue representation repeats the same information.** Most findings hold both `component`/`field`/`ids`
     and `fields`/`objects`. Multi-field findings select a first field, and cross-component findings select a first
     component, to fill singleton properties that are not authoritative. Batch serialization then groups cross-component
     findings under that selected component. Keep one canonical representation instead of parallel projections.
4. **Batch orchestration mixes multiple responsibilities.** Dataset eligibility, typed record collection, ID lookup,
     update application, temporary dataset construction, and rule execution live in the input orchestrator. ID maps are
     rebuilt per scenario and records copied per scenario. Copies are an acceptable initial implementation, but update
     semantics belong with each component, not in an ever-growing central function.

### Review: Requirements

1. **The Python adapter solves the abandoned goal.** `_validation_report.py` validates and decodes the JSON schema,
     converts component and attribute enums, reconstructs exception objects, handles unknown exception kinds, and
     maintains separate single/batch decoders. This duplicates the wire contract in Python. Its exception reconstruction
     and parity tests should not be carried into the JSON-only interface.
2. **The JSON contract unnecessarily branches by validation mode.** Input reports use `issues`; batches use
     `is_batch` and component-grouped `data`. The batch schema is closed to components other than node and line.
     Use one envelope and one issue shape. Grouping by component is a consumer presentation choice, not a wire format.
3. **There is no implemented public deprecation transition.** The native helpers are private, and the existing
     public Python validator remains independent. Deprecation needs an explicit replacement, coverage statement,
     release policy, documentation changes, and tests; it should not silently change old return types.
4. **Batch completeness must be explicit.** Invalid update IDs intentionally stop merged value validation, and
     zero-scenario batches intentionally skip input-value validation. Existing Python batch validation also skips
     merged value checks after an ID error. Do not report this as a new bug or accidentally promise complete input
     validation. The new contract should distinguish a blocked scenario from a scenario that was fully checked.

### Recommended Ownership

Use this flow:

```text
ConstDataset + validation options
    -> supported component dispatch + shared validation context
    -> reusable rules + component composers + independent scenario validation
    -> typed ValidationReport
    -> one JSON serializer
    -> immutable C-owned JSON bytes
    -> Python bytes, copied before destroying the C result
```

- **Rules:** retain typed checks and issue aggregation. Do not make every rule manipulate JSON.
- **Component dispatch:** one small compile-time list of supported component validators, initially node and line.
    Each entry owns its input composer and update application. Reuse component names from existing component metadata.
    Validate dataset coverage against this list and dispatch over it. Absent components produce empty ranges through
    existing traversal. Do not add a plugin registry, callbacks, or presence flags for every component.
- **Shared context:** build an ID index once per input, with component identity and row identity. It supports
    reference membership, update lookup, and global uniqueness for any registered component. Within-component duplicate
    multiplicity and cross-component duplicate reporting remain separate, intentional rule semantics. Do not silently
    resolve an ambiguous update target using the first row inserted into a map.
- **Scenario orchestration:** separate input validation from batch validation. Validate invariant identities once;
    validate update addressing; apply each scenario independently to the unchanged input; then reuse input composers.
    Initially retain simple typed copies. Reuse existing update machinery only if its behavior on invalid data is safe;
    model construction/update logic must not be a prerequisite for validating an invalid model.
- **Report:** use one `ValidationReport` containing scope results. Each scope has an optional scenario index, an
    explicit outcome, and issues. This is a private typed representation, not another public getter interface.
- **C result:** own only a completed `std::string` of JSON. Serialize inside the validation call before publishing
    the handle. Remove the mutex, mutable state, raw issues, batch issue storage, and mode flag. Retain the opaque handle
    solely for allocation ownership and borrowed-byte lifetime. Do not return pointers into the input or C error handle.
- **Python:** a new JSON entry point returns UTF-8 `bytes` for both passing and failing validation. It prepares
    datasets/options, checks native operational errors, copies the report bytes, and destroys the C result in `finally`.
    Consumers can call `json.loads` themselves. No kind mapping, schema reimplementation, or `ValidationError` imports.

The experimental C `passed` getter can be removed because the JSON envelope carries validity. A separate pass/fail
operation would need a demonstrated consumer/performance requirement, not speculative retention. Keep validate,
get-JSON, and destroy operations; the result handle still earns its place by providing clear memory ownership.

"No exception objects" refers to data findings. Invalid pointers, unsupported datasets/options, malformed buffers,
allocation failures, and internal failures remain operational errors on the C handle and ordinary binding exceptions.
An operational failure must not produce a success-shaped or partial validation report.

### Proposed JSON Contract

This is a **proposed version 2**, not the currently implemented version 1. Changing the envelope and removing singleton
issue properties are incompatible wire changes, even though the C interface is still experimental.

- The envelope always contains `schema_version`, `validation_type`, `valid`, and `results`.
- `validation_type` is `input` or `batch`; it records the requested operation, not a second internal storage mode.
- Each result has `scenario` (`null` for the input scope, otherwise a zero-based index), `status`, and `issues`.
- Status is `passed`, `failed`, or `blocked`. A passed scope has no issues. A failed scope completed its scheduled
    checks and found issues. A blocked scope did not run merged value checks because update addressing or input
    identity checks failed. Its issues may be empty if the causal findings are already recorded in the input scope.
- `valid` is true exactly when every result is passed. It is a summary derived during serialization, not stored
    separately in the C result.
- Input validation has exactly one `scenario: null` scope and runs the supported input rules.
- Batch validation has an input scope for invariant ID checks, followed by one scope for every update scenario,
    including passing or blocked scenarios. Do not copy invariant findings into every scenario.
- In a zero-scenario batch the input scope still checks invariant IDs; no merged value checks run. Thus a passing
    zero-scenario batch does not claim that all input values are valid. Use input validation to make that claim.
- Issues contain only `kind`, canonical `fields`, canonical `objects`, and `details`. Cross-component findings use
    the same shape and need no arbitrary primary component. Reference targets and comparison values live in `details`.
- Object IDs can be null for missing IDs; include a zero-based component row when IDs are missing or ambiguous.
    A row denotes the effective input row, or the update row for an update-addressing finding. Sentinel integers do not
    need to escape as object identities. Component and field names remain strings, not language-specific enums.
- Preserve unknown kinds and additive attributes. JSON remains strict UTF-8; use tagged values if a rule needs to
    report nonfinite numeric values, never bare `NaN` or `Infinity` tokens. Do not include Python class names/messages.
- Keep issue order deterministic. Sort references where their order has no meaning, but retain intentional duplicate
    multiplicity and do not reorder update-ID findings without documenting that decision.

Proposed input example:

```json
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
                    "objects": [{"component": "node", "id": 3}],
                    "details": {"ref_value": 0}
                }
            ]
        }
    ]
}
```

Proposed batch example, with an unchanged valid input, an invalid line status, an unknown update ID, and a passing
scenario:

```json
{
    "schema_version": 2,
    "validation_type": "batch",
    "valid": false,
    "results": [
        {"scenario": null, "status": "passed", "issues": []},
        {
            "scenario": 0,
            "status": "failed",
            "issues": [
                {
                    "kind": "not_boolean",
                    "fields": [{"component": "line", "field": "from_status"}],
                    "objects": [{"component": "line", "id": 3}],
                    "details": {}
                }
            ]
        },
        {
            "scenario": 1,
            "status": "blocked",
            "issues": [
                {
                    "kind": "id_not_in_dataset",
                    "fields": [{"component": "line", "field": "id"}],
                    "objects": [{"component": "line", "id": 99}],
                    "details": {"reference": {"dataset": "input", "component": "line", "field": "id"}}
                }
            ]
        },
        {"scenario": 2, "status": "passed", "issues": []}
    ]
}
```

For comparison, [the standalone current-output sample](../api_reference/validation-report.example.json) illustrates
the complete **implemented version 1** batch shape, including the redundant singleton and reference properties
emitted by today's serializer. It is an illustrative fixture, not output captured from a live validation call.

### Implementation Plan

1. **Agree the JSON contract first.** Establish scope/status semantics, zero-scenario behavior, canonical issue
     references, extension rules, and the version change. Add a version 2 schema and fixtures before changing the wire
     producer. Keep the current version 1 schema accurate until that producer changes.
2. **Simplify report ownership.** Introduce the typed report and one serializer; eagerly build the JSON in the C
     validation call; make `PGM_ValidationResult` a string owner. Replace lazy-serialization tests with immutable buffer,
     repeated read, dataset/result lifetime, passing-report, and operational-failure checks.
3. **Centralize supported component dispatch and identity checks.** Remove presence flags and pair-specific global
     ID rules. Verify that registering a third composer needs no report, schema, or binding change. Keep unsupported
     components explicitly rejected rather than returning a partially checked success.
4. **Separate and tighten batch validation.** Move component update semantics out of the orchestrator, reuse input
     indexes, preserve scenario isolation, and emit blocked outcomes when addressing is invalid or ambiguous. Verify
     positional IDs, missing IDs, mixed IDs, unknown IDs, duplicate input IDs, empty components, empty scenarios, zero
     scenarios, row/columnar storage, and update/input non-mutation.
5. **Replace the private Python adapter with a thin JSON binding.** Choose and publish distinct JSON-returning
     function names. Remove private exception decoders and their reconstruction tests; replace them with byte ownership,
     JSON parsing, passing/failing reports, schema fixtures, and native operational-error checks. Existing public Python
     validation behavior remains unchanged at this step.
6. **Deprecate the legacy Python validator explicitly.** Once the replacement is usable and its limited coverage is
     documented, add warnings and a stated removal release. Retain old functions and exception classes for that policy's
     compatibility period; do not route them through the partial native validator. Define coverage requirements before
     removal, or explicitly accept and document lost capabilities. No migration adapter or exception reconstruction is
     required for the new interface.

Do not combine the whole redesign into one rewrite. Each step must preserve native rule tests, pass focused C API
checks, and validate produced JSON against its schema. Python tests for the legacy validator remain until its behavior
is intentionally retired. Performance work, logging variants, binary formats, and runtime registries are not part of
this redesign.