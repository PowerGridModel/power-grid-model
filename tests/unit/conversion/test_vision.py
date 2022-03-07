from typing import Optional, Tuple

import pytest
from power_grid_model.conversion.vision import COL_REF_RE


def test_cases():
    yield "OtherSheet!ValueColumn[IdColumn=RefColumn]", \
          ("OtherSheet", "ValueColumn", None, None, "IdColumn", None, None, "RefColumn")

    yield "OtherSheet!ValueColumn[OtherSheet!IdColumn=ThisSheet!RefColumn]", \
          ("OtherSheet", "ValueColumn", "OtherSheet!", "OtherSheet", "IdColumn", "ThisSheet!", "ThisSheet", "RefColumn")

    yield "OtherSheet.ValueColumn[IdColumn=RefColumn]", None
    yield "ValueColumn[IdColumn=RefColumn]", None
    yield "OtherSheet![IdColumn=RefColumn]", None


@pytest.mark.parametrize("value,groups", test_cases())
def test_col_ref_pattern(value: str, groups: Optional[Tuple[Optional[str]]]):
    match = COL_REF_RE.fullmatch(value)
    if groups is None:
        assert match is None
    else:
        assert match.groups() == groups
