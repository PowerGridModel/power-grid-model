# Summary: Feature Request Creation Complete ✅

## What Was Requested

> "Based on the remaining task mentioned in this issue, i.e., the check of the error in the data 
> validator, please help me create a feature request (issue) for it."

**Source**: Issue #1263, comment by @mgovers:
> "In the long term, we can add it to the data validator, but that will still require some more 
> design discussions"

## What Was Delivered

✅ **Complete feature request documentation** ready to be converted into a GitHub issue

### Files Created

1. **`FEATURE_REQUEST_TAP_REGULATOR_VALIDATION.md`** (254 lines)
   - The complete feature request with all details
   - **This is the main document to copy into a GitHub issue**

2. **`GITHUB_ISSUE_INSTRUCTIONS.md`** (153 lines)
   - Step-by-step instructions for creating the GitHub issue
   - Two approaches: full document or simplified version

3. **`README_FEATURE_REQUEST.md`** (103 lines)
   - Quick overview and summary
   - Background context and next steps

## The Feature Request

### Problem

Currently, when a tap regulator has invalid control side configuration, the error is only detected 
during runtime calculation. Users cannot validate this before constructing the PowerGridModel.

### Solution

Add topology-dependent validation to the Python data validator to detect these errors early, providing:
- Early error detection before calculations
- Clear error messages with affected transformer IDs  
- Consistent validation between Python and C++
- Better development workflow for large models

### Implementation Options

Three approaches are documented:
1. Graph-based validation (similar to C++ implementation)
2. Deferred validation with topology context (recommended)
3. Simplified heuristic validation

## How to Use This

### Step 1: Create the GitHub Issue

Go to: https://github.com/PowerGridModel/power-grid-model/issues/new

- **Title**: `[FEATURE] Add tap regulator control side validation to data validator`
- **Body**: Copy the content from `FEATURE_REQUEST_TAP_REGULATOR_VALIDATION.md`
- **Labels**: Add `feature`
- **Link**: Reference issue #1263 in the description

### Step 2: Team Discussion

The feature request includes discussion questions for:
- Topology validation framework design
- Performance trade-offs
- API design decisions
- Opt-in vs opt-out approach

### Step 3: Implementation

Once the team agrees on the approach:
- Estimated effort: 4-6 weeks
- Complexity: Medium-High
- Requires design discussion before implementation

## Key Insights from Research

### Current Data Validator
- Located in `src/power_grid_model/validation/`
- Validates: structure, required values, IDs, value constraints
- Does NOT validate: topology-dependent configuration

### The Missing Validation
- Error: "Automatic tap changer has invalid configuration. The transformer is being controlled from 
  non source side towards source side."
- Currently: Only caught during C++ calculation
- Desired: Caught during Python `validate_input_data()`

### Why This Matters
For users with large models (50+ transformers):
- Cannot validate all data before construction
- Must rely on trial-and-error with calculations
- Difficult to integrate into automated pipelines

## Related Information

- **Origin**: [Issue #1263](https://github.com/PowerGridModel/power-grid-model/issues/1263)
- **Short-term fix**: [PR #1279](https://github.com/PowerGridModel/power-grid-model/pull/1279)
- **Documentation**: [Data Validator](https://power-grid-model.readthedocs.io/en/stable/user_manual/data-validator.html)

## Success Criteria

The task is complete when:
- [x] Comprehensive feature request document created
- [x] Clear problem statement and motivation documented
- [x] Multiple implementation approaches provided
- [x] Design considerations and questions outlined
- [x] Instructions for creating GitHub issue provided
- [ ] GitHub issue created (next step for maintainers)

---

**Status**: ✅ **COMPLETE** - Ready for GitHub issue creation

All documentation is complete and ready. The next step is for a team member to create the GitHub 
issue using the provided documentation.
