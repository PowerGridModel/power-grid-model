# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import ctypes

import pytest

from power_grid_model._core import error_handling
from power_grid_model._core.error_codes import _ERROR_CODE_TO_EXCEPTION, _PgmCErrorCode
from power_grid_model._core.error_handling import find_error
from power_grid_model._core.errors import PowerGridBatchError, PowerGridError

_CODED_ERRORS = [code for code in _PgmCErrorCode if code not in (_PgmCErrorCode.NO_ERROR, _PgmCErrorCode.BATCH_ERROR)]


class _FakeCore:
    """Stands in for the C API handle: reports a fixed error state."""

    def __init__(self, error_code: int, message: str = "", scenario_codes: list[int] | None = None):
        self._error_code = error_code
        self._message = message
        self._scenario_codes = scenario_codes or []
        n_scenarios = len(self._scenario_codes)
        self._scenarios = (ctypes.c_int64 * n_scenarios)(*range(n_scenarios))
        self._codes = (ctypes.c_int64 * n_scenarios)(*self._scenario_codes)
        self._messages = [f"scenario {i}".encode() for i in range(n_scenarios)]

    def error_code(self):
        return self._error_code

    def error_message(self):
        return self._message

    def n_failed_scenarios(self):
        return len(self._scenario_codes)

    def failed_scenarios(self):
        return ctypes.cast(self._scenarios, ctypes.POINTER(ctypes.c_int64))

    def batch_error_codes(self):
        return ctypes.cast(self._codes, ctypes.POINTER(ctypes.c_int64))

    def batch_errors(self):
        return self._messages


def _set_core_state(monkeypatch: pytest.MonkeyPatch, core: _FakeCore):
    monkeypatch.setattr(error_handling, "get_pgc", lambda: core)


def test_find_error__no_error(monkeypatch):
    _set_core_state(monkeypatch, _FakeCore(_PgmCErrorCode.NO_ERROR))

    assert find_error() is None


@pytest.mark.parametrize("code", _CODED_ERRORS, ids=lambda code: code.name)
def test_find_error__every_error_code_is_decoded(monkeypatch, code):
    _set_core_state(monkeypatch, _FakeCore(code, "test message"))

    error = find_error()

    assert type(error) is _ERROR_CODE_TO_EXCEPTION[code]
    assert isinstance(error, PowerGridError)
    assert "test message" in str(error)


@pytest.mark.parametrize("code", _CODED_ERRORS, ids=lambda code: code.name)
def test_find_error__decode_error_disabled(monkeypatch, code):
    _set_core_state(monkeypatch, _FakeCore(code, "test message"))

    assert type(find_error(decode_error=False)) is PowerGridError


def test_find_error__unknown_error_code(monkeypatch):
    unknown_code = max(_PgmCErrorCode) + 1000
    _set_core_state(monkeypatch, _FakeCore(unknown_code))

    error = find_error()

    assert type(error) is RuntimeError
    assert "Unknown error!" in str(error)


def test_find_error__batch_error_decodes_every_scenario_by_its_code(monkeypatch):
    scenario_codes = [int(code) for code in _CODED_ERRORS]
    _set_core_state(monkeypatch, _FakeCore(_PgmCErrorCode.BATCH_ERROR, scenario_codes=scenario_codes))

    error = find_error(batch_size=len(scenario_codes) + 1)

    assert isinstance(error, PowerGridBatchError)
    assert list(error.error_codes) == scenario_codes
    assert [type(scenario_error) for scenario_error in error.errors] == [
        _ERROR_CODE_TO_EXCEPTION[code] for code in _CODED_ERRORS
    ]
