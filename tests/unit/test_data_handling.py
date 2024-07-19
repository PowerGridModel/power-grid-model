# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import numpy as np
import pytest

from power_grid_model.core.data_handling import OutputType, create_output_data, process_output_component_types
from power_grid_model.core.dataset_definitions import ComponentType as CT, DatasetType as DT
from power_grid_model.core.power_grid_meta import initialize_array


@pytest.mark.parametrize(
    ("output_component_types", "is_batch", "expected"),
    [
        (
            None,
            False,
            {
                CT.node: initialize_array(DT.sym_output, CT.node, 4),
                CT.sym_load: initialize_array(DT.sym_output, CT.sym_load, 3),
                CT.source: initialize_array(DT.sym_output, CT.source, 1),
            },
        ),
        (
            [CT.node, CT.sym_load],
            False,
            {
                CT.node: initialize_array(DT.sym_output, CT.node, 4),
                CT.sym_load: initialize_array(DT.sym_output, CT.sym_load, 3),
            },
        ),
        (
            {CT.node, CT.sym_load},
            False,
            {
                CT.node: initialize_array(DT.sym_output, CT.node, 4),
                CT.sym_load: initialize_array(DT.sym_output, CT.sym_load, 3),
            },
        ),
        pytest.param(
            {CT.node: [], CT.sym_load: []}, True, {CT.node: dict(), CT.sym_load: dict()}, marks=pytest.mark.xfail
        ),
        pytest.param({CT.node: [], CT.sym_load: ["p"]}, True, {}, marks=pytest.mark.xfail),
        pytest.param({CT.node: ["u"], CT.sym_load: ["p"]}, True, {}, marks=pytest.mark.xfail),
        pytest.param({CT.node: None, CT.sym_load: ["p"]}, True, {}, marks=pytest.mark.xfail),
    ],
)
def test_create_output_data(output_component_types, is_batch, expected):
    # TODO use is_batch and shorten parameterization after columnar data implementation
    all_component_count = {CT.node: 4, CT.sym_load: 3, CT.source: 1}
    batch_size = 15 if is_batch else 1
    actual = create_output_data(
        output_component_types=output_component_types,
        output_type=OutputType.SYM_OUTPUT,
        all_component_count=all_component_count,
        is_batch=is_batch,
        batch_size=batch_size,
    )

    assert actual.keys() == expected.keys()
    for comp in expected:
        if isinstance(expected[comp], np.ndarray):
            # Row based
            assert actual[comp].dtype == expected[comp].dtype
        elif expected[comp] == dict():
            # Empty atrtibutes
            assert actual[comp] == expected[comp]
        else:
            # Columnar data
            assert actual[comp].keys() == expected[comp].keys()
            assert all(actual[comp][attr].dtype == expected[comp][attr].dtype for attr in expected[comp])


@pytest.mark.parametrize(
    ("output_component_types", "error", "match"),
    [
        ({"abc": 3, "def": None}, ValueError, "Invalid output_component_types"),
        ({"abc": None, "def": None}, KeyError, "unknown component"),
        ({"abc": None, CT.sym_load: None}, KeyError, "unknown component"),
        ({"abc": ["xyz"], CT.sym_load: None}, KeyError, "unknown component"),
        ({CT.node: ["xyz"], CT.sym_load: None}, KeyError, "unknown attributes"),
        ({CT.node: ["xyz1"], CT.sym_load: ["xyz2"]}, KeyError, "unknown attributes"),
    ],
)
def test_create_output_data__errors(output_component_types, error, match):
    available_components = [CT.node, CT.sym_load, CT.source]
    with pytest.raises(error, match=match):
        process_output_component_types(
            output_type=OutputType.SYM_OUTPUT,
            output_component_types=output_component_types,
            available_components=available_components,
        )
