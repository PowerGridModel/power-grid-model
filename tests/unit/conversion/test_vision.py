# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from typing import Optional, Tuple

import pytest
from power_grid_model.conversion.vision import COL_REF_RE


def cases():
    yield "OtherSheet!ValueColumn[IdColumn=RefColumn]", (
        "OtherSheet",
        "ValueColumn",
        None,
        None,
        "IdColumn",
        None,
        None,
        "RefColumn",
    )

    yield "OtherSheet!ValueColumn[OtherSheet!IdColumn=ThisSheet!RefColumn]", (
        "OtherSheet",
        "ValueColumn",
        "OtherSheet!",
        "OtherSheet",
        "IdColumn",
        "ThisSheet!",
        "ThisSheet",
        "RefColumn",
    )

    yield "OtherSheet.ValueColumn[IdColumn=RefColumn]", None
    yield "ValueColumn[IdColumn=RefColumn]", None
    yield "OtherSheet![IdColumn=RefColumn]", None


@pytest.mark.parametrize("value,groups", cases())
def test_col_ref_pattern(value: str, groups: Optional[Tuple[Optional[str]]]):
    match = COL_REF_RE.fullmatch(value)
    if groups is None:
        assert match is None
    else:
        assert match.groups() == groups
