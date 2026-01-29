<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0

-->

# [FEATURE] Add tap regulator control side validation to data validator

## Summary

Add validation to the Python data validator to detect invalid tap regulator control side configuration
before running power flow calculations. This validation should catch cases where a transformer is being
controlled from the non-source side towards the source side, which is currently only detected during
runtime calculation.

## Related Issues and PRs

- **Origin**: Issue #1263 - [Power flow calculation. Case of incorrectly specified transformer
  tap_regulator control_side](https://github.com/PowerGridModel/power-grid-model/issues/1263)
- **Short-term solution**: PR #1279 - Report unique ID in transformer regulate side error message
- **Current status**: Runtime error messages now include transformer IDs, but validation is still only performed
 during calculation

## Problem Statement

### Current Behavior

Currently, when a tap regulator has an invalid control side configuration (controlling from non-source
side towards source side), the error is only detected during power flow calculation in the C++ core:

```text
AutomaticTapInputError: Automatic tap changer has invalid configuration. 
The transformer is being controlled from non source side towards source side.
Transformer IDs: [123, 456, 789]
```

### Why This Is a Problem

1. **Late error detection**: Errors are only discovered after the model is constructed and calculation
   begins, wasting computation time

2. **Poor development experience**: Developers must wait for calculation to start before discovering
   data issues
3. **Incomplete validation workflow**: The Python data validator doesn't catch this error, creating a
   gap in pre-calculation validation

4. **Inconsistent with best practices**: The documentation recommends validating data before
   construction, but this specific check is missing

### User Impact

For users with large models (e.g., 50+ transformers with tap regulators):

- Cannot validate all input data before construction
- Must rely on trial-and-error with calculations to find configuration issues
- Difficult to integrate into automated data validation pipelines

## Proposed Solution

Add a new validation check in the Python data validator
(`src/power_grid_model/validation/_validation.py`) to detect invalid tap regulator control side
configurations.

### Validation Logic

The validation should check for each `transformer_tap_regulator`:

1. **Identify the regulated transformer** (using `regulated_object` field)

2. **Determine the topology**:

   - For 2-winding transformers: Identify which node is closer to the source
   - For 3-winding transformers: Identify which nodes are closer to the source

3. **Validate control side**:

   - The `control_side` must point to a side that is closer to or at the source
   - If controlling from a side that is farther from the source, raise a validation error

### Implementation Approach

This is a **topology-dependent validation** that requires:

#### Option 1: Graph-based validation (similar to C++ implementation)

- Build a minimal topology graph from node and branch connections
- Perform breadth-first search from source nodes to determine distance from source
- Compare control side node distance with other side distances

#### Option 2: Deferred validation with topology context

- Add a new validation phase that requires topology information
- This could be a separate validation function: `validate_input_data_topology()`
- More complex but allows for other topology-dependent validations in the future

#### Option 3: Simplified heuristic validation

- Check basic configuration rules (e.g., HV side typically closer to source)
- Less accurate but provides some early detection
- Could still miss edge cases

### Recommended Approach

**Option 2** is recommended because:

- It's architecturally sound and extensible for future topology-dependent validations
- Provides complete accuracy by using actual topology
- Maintains separation between simple field validation and complex graph validation
- Aligns with the long-term vision mentioned in issue #1263

## Expected Outcome

After implementation:

1. **Early error detection**: Invalid tap regulator configurations detected during
   `validate_input_data()`

2. **Consistent validation**: Python data validator catches the same errors as C++ runtime

3. **Better error messages**: Clear validation errors with affected transformer IDs before calculation

4. **Improved workflow**: Users can validate all input data before constructing `PowerGridModel`

### Example Usage

```python
from power_grid_model import PowerGridModel
from power_grid_model.validation import validate_input_data, assert_valid_input_data

# Current behavior: validation passes, but calculation fails
errors = validate_input_data(input_data, symmetric=True)
# errors = [] (no topology validation)

model = PowerGridModel(input_data)  # Construction succeeds
model.calculate_power_flow()  # Runtime error here!
# AutomaticTapInputError: Automatic tap changer has invalid configuration...

# Desired behavior: validation catches the error early
errors = validate_input_data(input_data, symmetric=True)
# errors = [InvalidTapRegulatorControlSideError(...)]

assert_valid_input_data(input_data, symmetric=True)
# ValidationException: 1 error in input_data
#   InvalidTapRegulatorControlSideError: 
#     Transformer tap regulator has invalid control side configuration.
#     The regulator is controlling from non-source side towards source side.
#     Affected transformers: [123, 456, 789]
```

## Design Considerations

### Performance

- Graph-based validation may have performance implications for large networks
- Consider making topology validation optional via a parameter: `validate_topology=False`
- Could use caching if validation is called multiple times on the same data

### Backward Compatibility

- New validation should be additive only
- Existing validation should continue to work
- Consider adding a warning first, then making it an error in a future release

### Testing

The implementation should include:

1. Unit tests with simple 2-winding transformer cases

2. Unit tests with 3-winding transformer cases

3. Integration tests with complex networks

4. Performance benchmarks for large networks

5. Test cases matching the scenarios from issue #1263

## Success Criteria

- [ ] Python data validator detects invalid tap regulator control side configurations
- [ ] Error messages clearly identify affected transformer IDs
- [ ] Validation works for both 2-winding and 3-winding transformers
- [ ] Documentation updated to reflect new validation capability
- [ ] Performance impact is acceptable (< 10% overhead for typical networks)
- [ ] All existing tests continue to pass
- [ ] New tests cover the validation scenarios

## Priority

**Medium-High**

This was identified as a "long-term" task in issue #1263, indicating it requires design discussion but is valuable for improving the user experience and data validation completeness.

## Additional Context

### Current Data Validator Capabilities

The data validator (`src/power_grid_model/validation/`) currently validates:

- Field structure and types
- Required values (non-NaN)
- Unique IDs across components
- Field value constraints (ranges, enums, relationships)
- Cross-field relationships (e.g., tap_min ≤ tap_pos ≤ tap_max)

### What's Missing

The validator doesn't currently perform:

- Topology-dependent validation (node connectivity, distance from source)
- Graph-based validation (cycles, islands, source reachability)
- Cross-component configuration validation requiring topology context

This feature would be the first topology-dependent validation in the Python validator.

### References

- [Data Validator Documentation](https://power-grid-model.readthedocs.io/en/stable/user_manual/data-validator.html)
- [Validation Examples](https://power-grid-model.readthedocs.io/en/stable/examples/Validation%20Examples.html)
- C++ exception: `AutomaticTapInputError` in `exception.hpp`
- Python validation: `validate_transformer_tap_regulator()` in `_validation.py`

## Questions for Discussion

1. **Topology validation framework**: Should we create a general framework for topology-dependent validation, or implement this as a one-off check?

2. **Performance vs. accuracy trade-off**: Is it acceptable to build a topology graph during validation, or should we use a simpler heuristic approach?

3. **Validation phases**: Should we separate simple field validation from topology validation? (e.g., `validate_input_data()` vs. `validate_input_data_topology()`)

4. **Opt-in vs. opt-out**: Should topology validation be:

   - Always enabled (opt-out with `validate_topology=False`)
   - Optional (opt-in with `validate_topology=True`)
   - Automatic but with performance warning for large networks

5. **Error severity**: Should this be:

   - An error (prevents calculation)
   - A warning (allows calculation but warns user)
   - Configurable by user

## Implementation Estimate

- **Complexity**: Medium-High (requires graph algorithms and topology analysis)
- **Estimated effort**: 2-3 weeks including design, implementation, testing, and documentation
- **Dependencies**: Requires design discussion and agreement on topology validation approach
- **Risk**: Medium (new validation paradigm, potential performance impact)

---

**Note**: This feature request is generated based on the "long-term" task mentioned in issue #1263, comment by @mgovers:
> "In the long term, we can add it to the data validator, but that will still require some more design discussions"
