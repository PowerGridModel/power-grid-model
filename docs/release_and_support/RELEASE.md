<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Release strategy

This project uses a rolling release strategy on `main` branch.
A new push(merge) to the `main` branch will trigger GitHub Actions to automatically 
build and upload a new version to PyPI with a unique version number.

**All the bug fixes will be committed directly into the `main` branch and published in the latest release. 
No effort will be spent on backporting bug fixes to previous versions!**

Sometimes a `release/` branch will be created temporarily for 
a big new version with new features and/or breaking changes.
A push(merge) to the release branch will tigger GitHub Actions to automatically 
build and upload a new version to PyPI with a unique release-candidate version number (suffix `rc`).

**NOTE: Release candidate versions are for testing only. You should not use them in production!**
