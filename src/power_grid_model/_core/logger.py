# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Opt-in diagnostic loggers for Power Grid Model calculations.

Loggers capture non-conclusive hints produced during calculations (e.g. sparse-matrix
debug text or per-phase benchmark timings). They are opt-in: no logger is active by
default, so there is zero performance cost unless you register one.

Basic usage::

    with Logger() as log:
        model.calculate(...)
    print(log.output)

Multiple loggers simultaneously::

    with Logger(LoggerType.text) as text_log, Logger(LoggerType.benchmark) as bench_log:
        model.calculate(...)
    print(text_log.output)
    print(bench_log.output)

Python logging module integration::

    import logging
    py_logger = logging.getLogger("power_grid_model")

    with Logger(python_logger=py_logger):
        model.calculate(...)
    # output is flushed to py_logger automatically on exit

Using the same logger from multiple user threads simultaneously is UB (internal batch
threads spawned by the C core are safe). Destroying a logger while it is still
inside a ``with`` block triggers a :exc:`ResourceWarning`.
"""

import logging as _logging
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
    cleared) automatically on ``__exit__``.

    Args:
        logger_type: The type of logger to create. Defaults to :attr:`LoggerType.text`.
        python_logger: An optional :class:`logging.Logger` to route output to on exit.
        level: The log level used when routing to *python_logger*. Defaults to
            :data:`logging.DEBUG`.
    """

    _logger_ptr: LoggerPtr

    def __init__(
        self,
        logger_type: LoggerType = LoggerType.text,
        *,
        python_logger: _logging.Logger | None = None,
        level: int = _logging.DEBUG,
    ) -> None:
        self._logger_ptr = get_pgc().create_logger(int(logger_type))
        assert_no_error()
        self._python_logger = python_logger
        self._level = level
        self._active: bool = False

    def __del__(self) -> None:
        if self._active:
            _warnings.warn(
                f"{self!r} is being destroyed inside an active 'with' block. "
                "Ensure the 'with Logger()' block has exited before the logger is garbage-collected.",
                ResourceWarning,
                stacklevel=2,
            )
        if self._logger_ptr:
            get_pgc().destroy_logger(self._logger_ptr)

    def __enter__(self) -> "Logger":
        get_pgc().register_logger(self._logger_ptr)
        assert_no_error()
        self._active = True
        return self

    def __exit__(self, *_: object) -> None:
        self._active = False
        get_pgc().unregister_logger(self._logger_ptr)
        assert_no_error()
        self.flush_to_python_logger()

    @property
    def output(self) -> str:
        """Current accumulated output of this logger.

        For :attr:`LoggerType.text`: timestamped log lines, one per logged event or message.
        For :attr:`LoggerType.benchmark`: one line per logged event,
        format ``EVENT_CODE\\tVALUE``.
        For :attr:`LoggerType.do_nothing`: always empty string.

        Accessible both inside and after the ``with`` block. The value is copied
        into Python on each access, so the returned string is independent of the
        logger's internal buffer.
        """
        result = get_pgc().logger_get_output(self._logger_ptr)
        assert_no_error()
        return result

    def clear(self) -> None:
        """Clear the accumulated output.

        For :attr:`LoggerType.do_nothing` this is a no-op.
        """
        get_pgc().logger_clear(self._logger_ptr)
        assert_no_error()

    def flush_to_python_logger(self) -> None:
        """Route accumulated output to the Python logger set at construction, then clear.

        Each non-empty line of :attr:`output` is emitted as a single log record at
        the configured *level*. Does nothing if no Python logger was supplied.
        Called automatically by ``__exit__``.
        """
        if self._python_logger is None:
            return
        get_pgc().logger_get_output_lines(
            self._logger_ptr,
            lambda line: self._python_logger.log(self._level, line),  # type: ignore[union-attr]
        )
        self.clear()
