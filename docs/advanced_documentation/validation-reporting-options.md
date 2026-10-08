# Validation Reporting Interface Options

This design comparison considers how C validation findings can be consumed by Python and Java, including reconstruction
of the error objects defined in `src/power_grid_model/validation/errors.py`.
The interfaces below are illustrative alternatives, not a proposal to expose all of them simultaneously.
Export macros and C++ linkage boilerplate are omitted for readability.

The current experimental [validation header](../../power_grid_model_c/power_grid_model_c/include/power_grid_model_c/validation.h)
already exposes validation, pass/fail, JSON retrieval, and result destruction.
The options argument shown below is planned and is not yet present in that header.
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

PGM_Idx PGM_validation_result_passed(
    PGM_Handle* handle, PGM_ValidationResult const* result);

void PGM_destroy_validation_result(PGM_ValidationResult* result);
```

- The options pointer is borrowed for the duration of validation. Its settings and whether NULL is accepted are future
    interface decisions, not specified by this reporting comparison.
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

An empty `issues` array means validation passed. The existing
[JSON Schema](../api_reference/validation-report.schema.json) describes the current report.
New attributes can be added without new C functions. Unknown kinds must be retained or explicitly rejected,
not silently discarded. Nonfinite values and enum identifiers require explicit representations rather than
nonstandard JSON tokens or Python-specific objects.

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

The root exposes the same logical envelope as the JSON example: `schema_version` and `issues`.
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

## Recommendation

For reconstructing existing Python error objects and corresponding Java types, prefer the versioned JSON report.
Keep pass/fail available for callers that need no diagnostics. A validation-JSON logger can deliver the same structured
report when logger-based retrieval is preferred, but requires explicit call isolation and callback lifetime handling.
Use ordinary human-readable logging as supplementary output.

Generic typed access is the main alternative when callers need selective native inspection.
Choose callbacks when incremental delivery matters, or a binary report when measurements justify the extra decoder.
Neither ordinary message parsing nor producer-supplied regexes is a strong default for a stable cross-language contract.