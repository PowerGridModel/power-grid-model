# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path


def test_data_validator_documents_duplicate_update_ids() -> None:
    documentation = (Path(__file__).parents[2] / "docs" / "user_manual" / "data-validator.md").read_text(
        encoding="utf-8"
    )

    assert "will just yield invalid results without warning" in documentation
    assert "{warning}" in documentation
    assert "issue #1552" in documentation
    assert "C++ validator migration" in documentation
