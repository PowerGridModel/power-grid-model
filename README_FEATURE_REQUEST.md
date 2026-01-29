<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Feature Request: Tap Regulator Validation in Data Validator

## What Was Done

This PR creates a comprehensive feature request document for adding tap regulator control side validation
to the Python data validator, as mentioned in issue #1263.

## Background

In issue #1263, a user reported difficulty identifying which transformer had an incorrect tap regulator
control side configuration in large models. The team implemented a three-stage solution:

1. **Short term** (✅ Completed): Add transformer IDs to runtime error messages (PR #1279)
2. **Mid-longer term** (Pending): Accumulate all problematic transformers and report all at once
3. **Long term** (This PR): Add validation to the Python data validator

## What's Included

### 1. Feature Request Document (`FEATURE_REQUEST_TAP_REGULATOR_VALIDATION.md`)

A comprehensive 230-line document covering:

- **Problem Statement**: Why runtime-only validation is insufficient
- **Proposed Solution**: Three implementation approaches with trade-offs
- **Design Considerations**: Performance, backward compatibility, testing
- **Success Criteria**: Measurable outcomes for implementation
- **Discussion Questions**: Key architectural decisions needed

### 2. GitHub Issue Instructions (`GITHUB_ISSUE_INSTRUCTIONS.md`)

A guide for creating the GitHub issue, including:

- Quick summary of the background
- Two approaches for creating the issue (full or simplified)
- Post-creation checklist
- Timeline estimates

### 3. Markdown Linting Configuration

Updated `.markdownlintignore` to exclude feature request documents from linting since they're meant
to be converted to GitHub issues where different formatting may be preferred.

## The Core Problem

Currently, when a tap regulator is configured to control from the non-source side towards the source
side, this error is only detected during calculation:

```text
AutomaticTapInputError: Automatic tap changer has invalid configuration. 
The transformer is being controlled from non source side towards source side.
Transformer IDs: [123, 456, 789]
```

Users cannot detect this error using `validate_input_data()` before constructing the `PowerGridModel`.

## Proposed Solution

Add topology-dependent validation to the Python data validator that:

1. Builds a minimal topology graph from node and branch connections
2. Determines distance from source nodes using breadth-first search
3. Validates that tap regulators control from a side closer to the source
4. Reports errors early with clear messages and affected transformer IDs

## Why This Matters

For users working with large power grid models:

- **Early error detection**: Find configuration issues before running calculations
- **Better development workflow**: Validate all input data upfront
- **Improved debugging**: Clear error messages with specific transformer IDs
- **Automation-friendly**: Can be integrated into data validation pipelines

## Next Steps

1. **Review** the detailed feature request document
2. **Create GitHub issue** using the provided instructions
3. **Discuss** the implementation approach with the team
4. **Design** the topology validation framework
5. **Implement** once approach is agreed upon

## Related Links

- **Origin**: [Issue #1263](https://github.com/PowerGridModel/power-grid-model/issues/1263)
- **Short-term fix**: [PR #1279](https://github.com/PowerGridModel/power-grid-model/pull/1279)
- **Data Validator Docs**: <https://power-grid-model.readthedocs.io/en/stable/user_manual/data-validator.html>

## Files Changed

- `FEATURE_REQUEST_TAP_REGULATOR_VALIDATION.md` - New comprehensive feature request
- `GITHUB_ISSUE_INSTRUCTIONS.md` - New GitHub issue creation guide
- `.markdownlintignore` - Updated to exclude feature request documents

## Questions or Feedback?

Please review the detailed feature request document for comprehensive information. For any questions
or suggestions, feel free to comment on the PR or reach out to the maintainers.
