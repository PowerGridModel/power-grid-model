# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Opt-in diagnostic loggers for Power Grid Model calculations.

Loggers capture non-conclusive hints produced during calculations (e.g. sparse-matrix
debug text or per-phase benchmark timings). They are opt-in: no logger is active by
default, so there is zero performance cost unless you register one.

Lifecycle::

    logger = Logger(LoggerType.text)
    model.attach_logger(logger)
    model.calculate(...)
    print(logger.output)
    logger.clear()
    model.detach_logger(logger)

Python logging module integration::

    import logging
    py_logger = logging.getLogger("power_grid_model")

    # Option 1: context manager — flushes to py_logger automatically on exit
    with Logger(python_logger=py_logger) as logger:
        model.attach_logger(logger)
        model.calculate(...)
        model.detach_logger(logger)

    # Option 2: manual flush
    logger = Logger(python_logger=py_logger)
    model.attach_logger(logger)
    model.calculate(...)
    logger.flush_to_python_logger()
    model.detach_logger(logger)

Using the same logger from multiple user threads simultaneously is UB (internal batch
threads spawned by the C core are safe). Registering the same logger to the same model
more than once, or destroying a logger while it is still attached, is also UB.
"""

import logging as _logging

from power_grid_model._core.enum import LoggerType
from power_grid_model._core.error_handling import assert_no_error
from power_grid_model._core.power_grid_core import LoggerPtr, get_power_grid_core as get_pgc

__all__ = ["Logger", "LoggerType"]


class Logger:
    """Wrapper around an opaque PGM_Logger object.

    Create a logger with the desired :class:`LoggerType`, attach it to a
    :class:`~power_grid_model.PowerGridModel` via ``model.attach_logger()``, run
    calculations, then read :attr:`output` or call :meth:`clear`.

    When *python_logger* is supplied, :meth:`flush_to_python_logger` routes all
    accumulated lines to it (and clears the buffer). Use the instance as a context
    manager to have this happen automatically on exit.

    Args:
        logger_type: The type of logger to create. Defaults to :attr:`LoggerType.text`.
        python_logger: An optional :class:`logging.Logger` to route output to.
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

    def __del__(self) -> None:
        if self._logger_ptr:
            get_pgc().destroy_logger(self._logger_ptr)

    def __enter__(self) -> "Logger":
        return self

    def __exit__(self, *_: object) -> None:
        self.flush_to_python_logger()

    @property
    def output(self) -> str:
        """Current accumulated output of this logger.

        For :attr:`LoggerType.text`: timestamped log lines, one per logged event or message.
        For :attr:`LoggerType.benchmark`: one line per logged event,
        format ``EVENT_CODE\\tVALUE``.
        For :attr:`LoggerType.do_nothing`: always empty string.

        The value is copied into Python on each access, so the returned string is
        independent of the logger's internal buffer.
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
        """
        if self._python_logger is None:
            return
        for line in self.output.splitlines():
            if line:
                self._python_logger.log(self._level, line)
        self.clear()

    @property
    def _ptr(self) -> LoggerPtr:
        """Internal pointer used by PowerGridModel to register/unregister."""
        return self._logger_ptr
