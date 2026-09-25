# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import logging

import pytest

from power_grid_model import (
    AttributeType as AT,
    ComponentType as CT,
    DatasetType as DT,
    Logger,
    LoggerType,
    PowerGridModel,
    initialize_array,
)


@pytest.fixture(name="logger_test_network")
def make_logger_test_network():
    node = initialize_array(DT.input, CT.node, 1)
    node[AT.id] = 0
    node[AT.u_rated] = 100.0

    source = initialize_array(DT.input, CT.source, 1)
    source[AT.id] = 1
    source[AT.node] = 0
    source[AT.status] = 1
    source[AT.u_ref] = 1.0
    source[AT.sk] = 1000.0
    source[AT.rx_ratio] = 0.0

    sym_load = initialize_array(DT.input, CT.sym_load, 1)
    sym_load[AT.id] = 2
    sym_load[AT.node] = 0
    sym_load[AT.status] = 1
    sym_load[AT.type] = 2
    sym_load[AT.p_specified] = 0.0
    sym_load[AT.q_specified] = 500.0

    return PowerGridModel({CT.node: node, CT.source: source, CT.sym_load: sym_load})


def test_logger_captures_and_preserves_output_until_cleared(logger_test_network):
    logger = Logger(LoggerType.text)

    assert logger.output == ""
    with logger as entered_logger:
        assert entered_logger is logger
        logger_test_network.calculate_power_flow()
        captured_output = logger.output
        assert captured_output

    assert logger.output == captured_output
    logger_test_network.calculate_power_flow()
    assert logger.output == captured_output

    logger.clear()
    assert logger.output == ""

    with logger:
        logger_test_network.calculate_power_flow()
        assert logger.output


def test_nested_loggers_capture_only_while_registered(logger_test_network):
    outer_logger = Logger()
    inner_logger = Logger()

    with outer_logger:
        logger_test_network.calculate_power_flow()
        outer_output_before_inner = outer_logger.output
        assert outer_output_before_inner

        with inner_logger:
            logger_test_network.calculate_power_flow()
            inner_output = inner_logger.output
            assert inner_output
            assert outer_logger.output != outer_output_before_inner

        outer_output_after_inner = outer_logger.output
        logger_test_network.calculate_power_flow()
        assert outer_logger.output != outer_output_after_inner
        assert inner_logger.output == inner_output


def test_logger_unregisters_when_context_exits_with_exception(logger_test_network):
    logger = Logger()

    def calculate_then_raise():
        with logger:
            logger_test_network.calculate_power_flow()
            raise RuntimeError("calculation interrupted")

    with pytest.raises(RuntimeError, match="calculation interrupted"):
        calculate_then_raise()

    captured_output = logger.output
    assert captured_output
    logger_test_network.calculate_power_flow()
    assert logger.output == captured_output


def test_python_logger_flushes_each_line_at_configured_level(logger_test_network, caplog):
    python_logger = logging.getLogger("power_grid_model.logger_test")
    caplog.set_level(logging.DEBUG, logger=python_logger.name)
    logger = Logger(python_logger=python_logger, level=logging.WARNING)

    with logger:
        logger_test_network.calculate_power_flow()
        expected_messages = [line for line in logger.output.splitlines() if line]
        assert expected_messages

    records = [record for record in caplog.records if record.name == python_logger.name]
    assert [record.getMessage() for record in records] == expected_messages
    assert all(record.levelno == logging.WARNING for record in records)
    assert logger.output == ""


def test_flush_without_python_logger_leaves_output_available(logger_test_network):
    logger = Logger()

    with logger:
        logger_test_network.calculate_power_flow()
    captured_output = logger.output
    assert captured_output

    logger.flush_to_python_logger()
    assert logger.output == captured_output


def test_flushing_empty_output_emits_no_python_log_records(caplog):
    python_logger = logging.getLogger("power_grid_model.empty_logger_test")
    caplog.set_level(logging.DEBUG, logger=python_logger.name)
    logger = Logger(python_logger=python_logger)

    logger.flush_to_python_logger()

    assert not [record for record in caplog.records if record.name == python_logger.name]
