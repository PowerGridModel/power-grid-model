# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Opt-in diagnostic loggers for Power Grid Model calculations.

Loggers capture non-conclusive hints produced during calculations (e.g. sparse-matrix
debug text or per-phase benchmark timings). They are opt-in: no logger is active by
default, so there is zero performance cost unless you register one.

Basic usage::

    >>> with Logger() as log:
    ...     model.calculate(...)
    >>> print(log.output)

Multiple loggers simultaneously::

    >>> with Logger() as first_log, Logger() as second_log:
    ...     model.calculate(...)
    >>> print(first_log.output)
    >>> print(second_log.output)

Python logging module integration::

    >>> import logging
    >>> py_logger = logging.getLogger("power_grid_model")

    >>> with Logger(python_logger=py_logger):
    ...     model.calculate(...)

Output is flushed to ``py_logger`` automatically on exit.

Using the same logger from multiple user threads simultaneously is UB (internal batch
threads spawned by the C core are safe). Destroying a logger while it is still
inside a ``with`` block triggers a :exc:`ResourceWarning`.
"""

import logging as _logging
import threading as _threading
import warnings as _warnings

from power_grid_model._core.enum import LoggerType
from power_grid_model._core.error_handling import assert_no_error
from power_grid_model._core.power_grid_core import LoggerPtr, get_power_grid_core as get_pgc

__all__ = ["Logger", "LoggerType"]


class Logger:
    """Wrapper around an opaque PGM_Logger object.

    Use as a context manager: the logger is registered on ``__enter__`` and
    unregistered on ``__exit__``. The output buffer is preserved after the
    ``with`` block and is accessible via :attr:`output`.

    If *python_logger* is supplied, accumulated output is flushed to it (and
    cleared) automatically when the outermost context exits.

    Args:
        logger_type: The type of logger to create. Defaults to :attr:`LoggerType.info`.
        python_logger: An optional :class:`logging.Logger` to route output to on exit.
        level: The log level used when routing to *python_logger*. Defaults to
            :data:`logging.DEBUG`.
    """

    _logger_ptr: LoggerPtr

    def __init__(
        self,
        logger_type: LoggerType = LoggerType.info,
        *,
        python_logger: _logging.Logger | None = None,
        python_logging_level: int = _logging.DEBUG,
    ) -> None:
        self._logger_ptr = get_pgc().create_logger(int(logger_type))
        assert_no_error()
        self._python_logger = python_logger
        self._level = python_logging_level
        self._active_count: int = 0
        self._active_lock = _threading.Lock()

    def __del__(self) -> None:
        with self._active_lock:
            is_active = self._active_count > 0
        if is_active:
            _warnings.warn(
                f"{self!r} is being destroyed inside an active 'with' block. "
                "Ensure the 'with Logger()' block has exited before the logger is garbage-collected.",
                ResourceWarning,
                stacklevel=2,
            )
        if self._logger_ptr:
            get_pgc().destroy_logger(self._logger_ptr)

    def __enter__(self) -> "Logger":
        with self._active_lock:
            if self._active_count == 0:
                get_pgc().register_logger(self._logger_ptr)
                assert_no_error()
            self._active_count += 1
        return self

    def __exit__(self, *_: object) -> None:
        should_flush = False
        with self._active_lock:
            if self._active_count > 0:
                self._active_count -= 1
                if self._active_count == 0:
                    get_pgc().unregister_logger(self._logger_ptr)
                    assert_no_error()
                    should_flush = True
        if should_flush:
            self._flush_to_python_logger()

    @property
    def output(self) -> str:
        """Current accumulated output of this logger.

        Accessible both inside and after the ``with`` block. The value is copied
        into Python on each access, so the returned string is independent of the
        logger's internal buffer.
        """
        result = get_pgc().logger_get_output(self._logger_ptr)
        assert_no_error()
        return result

    def clear_content(self) -> None:
        """Clear the accumulated output."""
        get_pgc().logger_clear_content(self._logger_ptr)
        assert_no_error()

    def _flush_to_python_logger(self) -> None:
        """Route accumulated output to the Python logger set at construction, then clear.

        Each non-empty line of :attr:`output` is emitted as a single log record at
        the configured *level*. Does nothing if no Python logger was supplied.
        Called automatically when the outermost context exits.
        """
        if self._python_logger is None:
            return
        get_pgc().logger_get_output_lines(
            self._logger_ptr,
            lambda line: self._python_logger.log(self._level, line),  # type: ignore[union-attr]
        )
        self.clear_content()
