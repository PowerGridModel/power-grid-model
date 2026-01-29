<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0

-->

# How to Create the GitHub Issue

This document provides instructions for creating a GitHub issue from the feature request document.

## Quick Summary

A comprehensive feature request has been created for adding tap regulator control side validation to the Python data validator. This addresses the "long-term" remaining task mentioned in issue #1263.

## Background

### Original Issue: #1263
**Title**: [FEATURE] Power flow calculation. Case of incorrectly specified transformer tap_regulator control_side

**Problem**: When a tap regulator has an invalid control side configuration, the error message didn't show which transformer had the issue, making it difficult to debug in large models with many transformers.

**Solution Stages** (as discussed in comments):

1. ✅ **Short term** (Completed in PR #1279): Add transformer IDs to runtime error messages

2. 🔄 **Mid-longer term**: Accumulate all problematic transformers and output all at once
3. 📋 **Long term** (This Feature Request): Add validation to the Python data validator

## The Feature Request Document

The file `FEATURE_REQUEST_TAP_REGULATOR_VALIDATION.md` contains a comprehensive feature request with:

- **Clear problem statement**: Why the current runtime-only validation is insufficient
- **Proposed solution**: Add topology-dependent validation to the Python data validator
- **Implementation approaches**: Three options with pros/cons
- **Design considerations**: Performance, backward compatibility, testing strategy
- **Success criteria**: Measurable outcomes for successful implementation
- **Discussion questions**: Key decisions that need team input

## Creating the GitHub Issue

### Option 1: Use the Feature Request Document Directly

Copy the content of `FEATURE_REQUEST_TAP_REGULATOR_VALIDATION.md` into a new GitHub issue:

1. Go to: https://github.com/PowerGridModel/power-grid-model/issues/new

2. **Title**: `[FEATURE] Add tap regulator control side validation to data validator`

3. **Body**: Copy the entire content from `FEATURE_REQUEST_TAP_REGULATOR_VALIDATION.md`

4. **Labels**: Add `feature` label

5. **Assignees**: Optionally assign to team members for design discussion

6. **Projects**: Add to relevant project board if applicable

### Option 2: Create a Simplified Issue with Link

Create a shorter issue that references the detailed document:

```markdown
## Summary

Add validation to the Python data validator to detect invalid tap regulator control side configuration before running power flow calculations.

## Background

This addresses the "long-term" remaining task mentioned in issue #1263, where @mgovers noted:
> "In the long term, we can add it to the data validator, but that will still require some more design discussions"

## Problem

Currently, invalid tap regulator control side configurations are only detected during runtime calculation in C++. Users cannot validate their input data for this error using the Python data validator before constructing the PowerGridModel.

## Detailed Feature Request

A comprehensive feature request document has been created: `FEATURE_REQUEST_TAP_REGULATOR_VALIDATION.md`

This document includes:

- Detailed problem statement and user impact analysis
- Three implementation approaches with trade-offs
- Design considerations (performance, backward compatibility, testing)
- Success criteria and implementation estimate
- Discussion questions for the team

## Next Steps

1. Review the detailed feature request document

2. Discuss the implementation approach (especially topology validation framework)
3. Make architectural decisions on validation phases
4. Create implementation plan if approved

## Related
- Origin: #1263
- Short-term fix: PR #1279
```

## Recommended Approach

**Use Option 1** (full document) because:

- All context is in one place for team discussion
- The design questions need to be visible and addressable
- Implementation approaches need team input
- It's comprehensive enough for immediate planning if approved

## After Creating the Issue

1. **Link it to issue #1263**: Add a comment on #1263 referencing the new issue

2. **Tag relevant team members**: Mention @mgovers, @Jerry-Jinfeng-Guo, or other maintainers

3. **Add to backlog**: Include in project planning discussions

4. **Update documentation**: Once implemented, update the data validator documentation

## Key Points for Discussion

When the issue is created, these points will need team discussion:

1. **Topology validation framework**: Should this be a general framework or one-off?

2. **Performance trade-offs**: Is graph-based validation acceptable?

3. **API design**: Separate validation function or integrated into existing one?

4. **Opt-in vs opt-out**: Should topology validation be always enabled?

5. **Timeline**: When should this be prioritized relative to other features?

## Expected Timeline

Based on the document:

- **Design discussion**: 1-2 weeks
- **Implementation**: 2-3 weeks
- **Testing and documentation**: 1 week
- **Total**: ~4-6 weeks

This assumes approval and prioritization of the feature.

## Questions?

If you have questions about the feature request:

- Review the detailed document: `FEATURE_REQUEST_TAP_REGULATOR_VALIDATION.md`
- Check the original issue: #1263
- Review the short-term implementation: PR #1279
- Check the data validator documentation: https://power-grid-model.readthedocs.io/en/stable/user_manual/data-validator.html
