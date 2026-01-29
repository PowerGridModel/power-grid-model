# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Tests for transformer tap regulator control side topology validation.
"""

import numpy as np
import pytest

from power_grid_model import initialize_array
from power_grid_model._core.dataset_definitions import ComponentType, DatasetType
from power_grid_model.enum import BranchSide, Branch3Side
from power_grid_model.validation import validate_input_data
from power_grid_model.validation._validation import validate_tap_regulator_control_side_topology
from power_grid_model.validation.errors import InvalidTapRegulatorControlSideError


def test_valid_tap_regulator_control_side_simple():
    """Test valid control side configuration: source -> transformer with control on to_side"""
    # Setup: source at node 0, transformer from 0 to 1, control from to_side (1) - VALID
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input["id"] = [0, 1]
    node_input["u_rated"] = [150e3, 10e3]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input["id"] = [2]
    source_input["node"] = [0]
    source_input["status"] = [1]
    source_input["u_ref"] = [1.0]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer_input["id"] = [3]
    transformer_input["from_node"] = [0]
    transformer_input["to_node"] = [1]
    transformer_input["from_status"] = [1]
    transformer_input["to_status"] = [1]
    transformer_input["u1"] = [150e3]
    transformer_input["u2"] = [10e3]
    transformer_input["sn"] = [1e5]
    transformer_input["uk"] = [0.1]
    transformer_input["pk"] = [1e3]
    transformer_input["i0"] = [1.0e-6]
    transformer_input["p0"] = [0.1]
    transformer_input["winding_from"] = [1]
    transformer_input["winding_to"] = [1]
    transformer_input["clock"] = [0]
    transformer_input["tap_side"] = [BranchSide.from_side]
    transformer_input["tap_pos"] = [0]
    transformer_input["tap_nom"] = [0]
    transformer_input["tap_min"] = [-1]
    transformer_input["tap_max"] = [1]
    transformer_input["tap_size"] = [100]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 1)
    transformer_tap_regulator_input["id"] = [4]
    transformer_tap_regulator_input["regulated_object"] = [3]
    transformer_tap_regulator_input["status"] = [1]
    transformer_tap_regulator_input["control_side"] = [BranchSide.to_side]  # Control to_side (downstream) - VALID
    transformer_tap_regulator_input["u_set"] = [10e3]
    transformer_tap_regulator_input["u_band"] = [200]

    input_data = {
        ComponentType.node: node_input,
        ComponentType.source: source_input,
        ComponentType.transformer: transformer_input,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
    }

    errors = validate_tap_regulator_control_side_topology(input_data)
    assert len(errors) == 0


def test_invalid_tap_regulator_control_side_from_non_source():
    """Test invalid control side: controlling from non-source side (to_side) towards source side (from_side)"""
    # Setup: source at node 0, transformer from 0 to 1, but control from to_side (1) - INVALID
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input["id"] = [0, 1]
    node_input["u_rated"] = [150e3, 10e3]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input["id"] = [2]
    source_input["node"] = [0]
    source_input["status"] = [1]
    source_input["u_ref"] = [1.0]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer_input["id"] = [3]
    transformer_input["from_node"] = [0]
    transformer_input["to_node"] = [1]
    transformer_input["from_status"] = [1]
    transformer_input["to_status"] = [1]
    transformer_input["u1"] = [150e3]
    transformer_input["u2"] = [10e3]
    transformer_input["sn"] = [1e5]
    transformer_input["uk"] = [0.1]
    transformer_input["pk"] = [1e3]
    transformer_input["i0"] = [1.0e-6]
    transformer_input["p0"] = [0.1]
    transformer_input["winding_from"] = [1]
    transformer_input["winding_to"] = [1]
    transformer_input["clock"] = [0]
    transformer_input["tap_side"] = [BranchSide.from_side]
    transformer_input["tap_pos"] = [0]
    transformer_input["tap_nom"] = [0]
    transformer_input["tap_min"] = [-1]
    transformer_input["tap_max"] = [1]
    transformer_input["tap_size"] = [100]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 1)
    transformer_tap_regulator_input["id"] = [4]
    transformer_tap_regulator_input["regulated_object"] = [3]
    transformer_tap_regulator_input["status"] = [1]
    transformer_tap_regulator_input["control_side"] = [BranchSide.from_side]  # Controlling from source side
    transformer_tap_regulator_input["u_set"] = [10e3]
    transformer_tap_regulator_input["u_band"] = [200]

    input_data = {
        ComponentType.node: node_input,
        ComponentType.source: source_input,
        ComponentType.transformer: transformer_input,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
    }

    errors = validate_tap_regulator_control_side_topology(input_data)
    assert len(errors) == 1
    assert isinstance(errors[0], InvalidTapRegulatorControlSideError)
    assert errors[0].component == ComponentType.transformer_tap_regulator
    assert errors[0].field == "control_side"
    assert errors[0].ids == [4]


def test_valid_tap_regulator_cascaded_transformers():
    """Test valid control side in cascaded transformers: source -> T1 -> T2"""
    # Setup: source at node 0, T1 from 0 to 1, T2 from 1 to 2
    node_input = initialize_array(DatasetType.input, ComponentType.node, 3)
    node_input["id"] = [0, 1, 2]
    node_input["u_rated"] = [150e3, 50e3, 10e3]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input["id"] = [10]
    source_input["node"] = [0]
    source_input["status"] = [1]
    source_input["u_ref"] = [1.0]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 2)
    transformer_input["id"] = [3, 4]
    transformer_input["from_node"] = [0, 1]
    transformer_input["to_node"] = [1, 2]
    transformer_input["from_status"] = [1, 1]
    transformer_input["to_status"] = [1, 1]
    transformer_input["u1"] = [150e3, 50e3]
    transformer_input["u2"] = [50e3, 10e3]
    transformer_input["sn"] = [1e5, 1e5]
    transformer_input["uk"] = [0.1, 0.1]
    transformer_input["pk"] = [1e3, 1e3]
    transformer_input["i0"] = [1.0e-6, 1.0e-6]
    transformer_input["p0"] = [0.1, 0.1]
    transformer_input["winding_from"] = [1, 1]
    transformer_input["winding_to"] = [1, 1]
    transformer_input["clock"] = [0, 0]
    transformer_input["tap_side"] = [BranchSide.from_side, BranchSide.from_side]
    transformer_input["tap_pos"] = [0, 0]
    transformer_input["tap_nom"] = [0, 0]
    transformer_input["tap_min"] = [-1, -1]
    transformer_input["tap_max"] = [1, 1]
    transformer_input["tap_size"] = [100, 100]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 2)
    transformer_tap_regulator_input["id"] = [5, 6]
    transformer_tap_regulator_input["regulated_object"] = [3, 4]
    transformer_tap_regulator_input["status"] = [1, 1]
    transformer_tap_regulator_input["control_side"] = [BranchSide.to_side, BranchSide.to_side]  # Both control downstream
    transformer_tap_regulator_input["u_set"] = [50e3, 10e3]
    transformer_tap_regulator_input["u_band"] = [200, 200]

    input_data = {
        ComponentType.node: node_input,
        ComponentType.source: source_input,
        ComponentType.transformer: transformer_input,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
    }

    errors = validate_tap_regulator_control_side_topology(input_data)
    assert len(errors) == 0


def test_invalid_tap_regulator_second_transformer_wrong_control():
    """Test invalid control in cascaded transformers: T2 controlling from downstream towards upstream"""
    # Setup: source at node 0, T1 from 0 to 1, T2 from 1 to 2, but T2 controls from to_side (wrong direction)
    node_input = initialize_array(DatasetType.input, ComponentType.node, 3)
    node_input["id"] = [0, 1, 2]
    node_input["u_rated"] = [150e3, 50e3, 10e3]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input["id"] = [10]
    source_input["node"] = [0]
    source_input["status"] = [1]
    source_input["u_ref"] = [1.0]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 2)
    transformer_input["id"] = [3, 4]
    transformer_input["from_node"] = [0, 1]
    transformer_input["to_node"] = [1, 2]
    transformer_input["from_status"] = [1, 1]
    transformer_input["to_status"] = [1, 1]
    transformer_input["u1"] = [150e3, 50e3]
    transformer_input["u2"] = [50e3, 10e3]
    transformer_input["sn"] = [1e5, 1e5]
    transformer_input["uk"] = [0.1, 0.1]
    transformer_input["pk"] = [1e3, 1e3]
    transformer_input["i0"] = [1.0e-6, 1.0e-6]
    transformer_input["p0"] = [0.1, 0.1]
    transformer_input["winding_from"] = [1, 1]
    transformer_input["winding_to"] = [1, 1]
    transformer_input["clock"] = [0, 0]
    transformer_input["tap_side"] = [BranchSide.from_side, BranchSide.from_side]
    transformer_input["tap_pos"] = [0, 0]
    transformer_input["tap_nom"] = [0, 0]
    transformer_input["tap_min"] = [-1, -1]
    transformer_input["tap_max"] = [1, 1]
    transformer_input["tap_size"] = [100, 100]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 2)
    transformer_tap_regulator_input["id"] = [5, 6]
    transformer_tap_regulator_input["regulated_object"] = [3, 4]
    transformer_tap_regulator_input["status"] = [1, 1]
    transformer_tap_regulator_input["control_side"] = [BranchSide.to_side, BranchSide.from_side]  # T2 controls from_side (wrong!)
    transformer_tap_regulator_input["u_set"] = [50e3, 10e3]
    transformer_tap_regulator_input["u_band"] = [200, 200]

    input_data = {
        ComponentType.node: node_input,
        ComponentType.source: source_input,
        ComponentType.transformer: transformer_input,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
    }

    errors = validate_tap_regulator_control_side_topology(input_data)
    assert len(errors) == 1
    assert isinstance(errors[0], InvalidTapRegulatorControlSideError)
    assert errors[0].ids == [6]  # Only second regulator is invalid


def test_valid_three_winding_transformer_control_side():
    """Test valid control side for three-winding transformer"""
    node_input = initialize_array(DatasetType.input, ComponentType.node, 3)
    node_input["id"] = [0, 1, 2]
    node_input["u_rated"] = [150e3, 50e3, 10e3]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input["id"] = [10]
    source_input["node"] = [0]
    source_input["status"] = [1]
    source_input["u_ref"] = [1.0]

    transformer3w_input = initialize_array(DatasetType.input, ComponentType.three_winding_transformer, 1)
    transformer3w_input["id"] = [3]
    transformer3w_input["node_1"] = [0]
    transformer3w_input["node_2"] = [1]
    transformer3w_input["node_3"] = [2]
    transformer3w_input["status_1"] = [1]
    transformer3w_input["status_2"] = [1]
    transformer3w_input["status_3"] = [1]
    transformer3w_input["u1"] = [150e3]
    transformer3w_input["u2"] = [50e3]
    transformer3w_input["u3"] = [10e3]
    transformer3w_input["sn_1"] = [1e5]
    transformer3w_input["sn_2"] = [1e5]
    transformer3w_input["sn_3"] = [1e5]
    transformer3w_input["uk_12"] = [0.1]
    transformer3w_input["uk_13"] = [0.1]
    transformer3w_input["uk_23"] = [0.1]
    transformer3w_input["pk_12"] = [1e3]
    transformer3w_input["pk_13"] = [1e3]
    transformer3w_input["pk_23"] = [1e3]
    transformer3w_input["i0"] = [1.0e-6]
    transformer3w_input["p0"] = [0.1]
    transformer3w_input["winding_1"] = [1]
    transformer3w_input["winding_2"] = [1]
    transformer3w_input["winding_3"] = [1]
    transformer3w_input["clock_12"] = [0]
    transformer3w_input["clock_13"] = [0]
    transformer3w_input["tap_side"] = [Branch3Side.side_1]
    transformer3w_input["tap_pos"] = [0]
    transformer3w_input["tap_nom"] = [0]
    transformer3w_input["tap_min"] = [-1]
    transformer3w_input["tap_max"] = [1]
    transformer3w_input["tap_size"] = [100]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 1)
    transformer_tap_regulator_input["id"] = [4]
    transformer_tap_regulator_input["regulated_object"] = [3]
    transformer_tap_regulator_input["status"] = [1]
    transformer_tap_regulator_input["control_side"] = [Branch3Side.side_2]  # Control on side_2 (valid)
    transformer_tap_regulator_input["u_set"] = [50e3]
    transformer_tap_regulator_input["u_band"] = [200]

    input_data = {
        ComponentType.node: node_input,
        ComponentType.source: source_input,
        ComponentType.three_winding_transformer: transformer3w_input,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
    }

    errors = validate_tap_regulator_control_side_topology(input_data)
    assert len(errors) == 0


def test_disabled_regulator_not_validated():
    """Test that disabled regulators are not validated"""
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input["id"] = [0, 1]
    node_input["u_rated"] = [150e3, 10e3]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input["id"] = [2]
    source_input["node"] = [0]
    source_input["status"] = [1]
    source_input["u_ref"] = [1.0]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer_input["id"] = [3]
    transformer_input["from_node"] = [0]
    transformer_input["to_node"] = [1]
    transformer_input["from_status"] = [1]
    transformer_input["to_status"] = [1]
    transformer_input["u1"] = [150e3]
    transformer_input["u2"] = [10e3]
    transformer_input["sn"] = [1e5]
    transformer_input["uk"] = [0.1]
    transformer_input["pk"] = [1e3]
    transformer_input["i0"] = [1.0e-6]
    transformer_input["p0"] = [0.1]
    transformer_input["winding_from"] = [1]
    transformer_input["winding_to"] = [1]
    transformer_input["clock"] = [0]
    transformer_input["tap_side"] = [BranchSide.from_side]
    transformer_input["tap_pos"] = [0]
    transformer_input["tap_nom"] = [0]
    transformer_input["tap_min"] = [-1]
    transformer_input["tap_max"] = [1]
    transformer_input["tap_size"] = [100]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 1)
    transformer_tap_regulator_input["id"] = [4]
    transformer_tap_regulator_input["regulated_object"] = [3]
    transformer_tap_regulator_input["status"] = [0]  # DISABLED
    transformer_tap_regulator_input["control_side"] = [BranchSide.from_side]  # Would be invalid if enabled
    transformer_tap_regulator_input["u_set"] = [10e3]
    transformer_tap_regulator_input["u_band"] = [200]

    input_data = {
        ComponentType.node: node_input,
        ComponentType.source: source_input,
        ComponentType.transformer: transformer_input,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
    }

    errors = validate_tap_regulator_control_side_topology(input_data)
    assert len(errors) == 0  # Disabled regulator should not trigger error


def test_integration_with_validate_input_data():
    """Test that the new validation is integrated into validate_input_data"""
    # Setup invalid control side configuration
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input["id"] = [0, 1]
    node_input["u_rated"] = [150e3, 10e3]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input["id"] = [2]
    source_input["node"] = [0]
    source_input["status"] = [1]
    source_input["u_ref"] = [1.0]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer_input["id"] = [3]
    transformer_input["from_node"] = [0]
    transformer_input["to_node"] = [1]
    transformer_input["from_status"] = [1]
    transformer_input["to_status"] = [1]
    transformer_input["u1"] = [150e3]
    transformer_input["u2"] = [10e3]
    transformer_input["sn"] = [1e5]
    transformer_input["uk"] = [0.1]
    transformer_input["pk"] = [1e3]
    transformer_input["i0"] = [1.0e-6]
    transformer_input["p0"] = [0.1]
    transformer_input["winding_from"] = [1]
    transformer_input["winding_to"] = [1]
    transformer_input["clock"] = [0]
    transformer_input["tap_side"] = [BranchSide.from_side]
    transformer_input["tap_pos"] = [0]
    transformer_input["tap_nom"] = [0]
    transformer_input["tap_min"] = [-1]
    transformer_input["tap_max"] = [1]
    transformer_input["tap_size"] = [100]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 1)
    transformer_tap_regulator_input["id"] = [4]
    transformer_tap_regulator_input["regulated_object"] = [3]
    transformer_tap_regulator_input["status"] = [1]
    transformer_tap_regulator_input["control_side"] = [BranchSide.from_side]  # Controlling from source side - INVALID
    transformer_tap_regulator_input["u_set"] = [10e3]
    transformer_tap_regulator_input["u_band"] = [200]

    input_data = {
        ComponentType.node: node_input,
        ComponentType.source: source_input,
        ComponentType.transformer: transformer_input,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
    }

    errors = validate_input_data(input_data)
    assert errors is not None
    assert len(errors) > 0
    # Check that at least one error is InvalidTapRegulatorControlSideError
    assert any(isinstance(err, InvalidTapRegulatorControlSideError) for err in errors)


def test_no_source_no_error():
    """Test that if there's no source, no error is raised (all nodes unreachable)"""
    # Setup: No source, transformer from 0 to 1
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input["id"] = [0, 1]
    node_input["u_rated"] = [150e3, 10e3]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer_input["id"] = [3]
    transformer_input["from_node"] = [0]
    transformer_input["to_node"] = [1]
    transformer_input["from_status"] = [1]
    transformer_input["to_status"] = [1]
    transformer_input["u1"] = [150e3]
    transformer_input["u2"] = [10e3]
    transformer_input["sn"] = [1e5]
    transformer_input["uk"] = [0.1]
    transformer_input["pk"] = [1e3]
    transformer_input["i0"] = [1.0e-6]
    transformer_input["p0"] = [0.1]
    transformer_input["winding_from"] = [1]
    transformer_input["winding_to"] = [1]
    transformer_input["clock"] = [0]
    transformer_input["tap_side"] = [BranchSide.from_side]
    transformer_input["tap_pos"] = [0]
    transformer_input["tap_nom"] = [0]
    transformer_input["tap_min"] = [-1]
    transformer_input["tap_max"] = [1]
    transformer_input["tap_size"] = [100]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 1)
    transformer_tap_regulator_input["id"] = [4]
    transformer_tap_regulator_input["regulated_object"] = [3]
    transformer_tap_regulator_input["status"] = [1]
    transformer_tap_regulator_input["control_side"] = [BranchSide.from_side]
    transformer_tap_regulator_input["u_set"] = [10e3]
    transformer_tap_regulator_input["u_band"] = [200]

    input_data = {
        ComponentType.node: node_input,
        ComponentType.transformer: transformer_input,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
    }

    errors = validate_tap_regulator_control_side_topology(input_data)
    assert len(errors) == 0  # No source means all nodes unreachable, no partial reachability issue


def test_multiple_invalid_regulators():
    """Test multiple transformers with invalid control side configuration"""
    node_input = initialize_array(DatasetType.input, ComponentType.node, 4)
    node_input["id"] = [0, 1, 2, 3]
    node_input["u_rated"] = [150e3, 50e3, 10e3, 5e3]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input["id"] = [10]
    source_input["node"] = [0]
    source_input["status"] = [1]
    source_input["u_ref"] = [1.0]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 3)
    transformer_input["id"] = [20, 21, 22]
    transformer_input["from_node"] = [0, 1, 2]
    transformer_input["to_node"] = [1, 2, 3]
    transformer_input["from_status"] = [1, 1, 1]
    transformer_input["to_status"] = [1, 1, 1]
    transformer_input["u1"] = [150e3, 50e3, 10e3]
    transformer_input["u2"] = [50e3, 10e3, 5e3]
    transformer_input["sn"] = [1e5, 1e5, 1e5]
    transformer_input["uk"] = [0.1, 0.1, 0.1]
    transformer_input["pk"] = [1e3, 1e3, 1e3]
    transformer_input["i0"] = [1.0e-6, 1.0e-6, 1.0e-6]
    transformer_input["p0"] = [0.1, 0.1, 0.1]
    transformer_input["winding_from"] = [1, 1, 1]
    transformer_input["winding_to"] = [1, 1, 1]
    transformer_input["clock"] = [0, 0, 0]
    transformer_input["tap_side"] = [BranchSide.from_side, BranchSide.from_side, BranchSide.from_side]
    transformer_input["tap_pos"] = [0, 0, 0]
    transformer_input["tap_nom"] = [0, 0, 0]
    transformer_input["tap_min"] = [-1, -1, -1]
    transformer_input["tap_max"] = [1, 1, 1]
    transformer_input["tap_size"] = [100, 100, 100]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 3)
    transformer_tap_regulator_input["id"] = [30, 31, 32]
    transformer_tap_regulator_input["regulated_object"] = [20, 21, 22]
    transformer_tap_regulator_input["status"] = [1, 1, 1]
    transformer_tap_regulator_input["control_side"] = [
        BranchSide.to_side,  # Valid
        BranchSide.from_side,  # Invalid - controlling from downstream towards upstream
        BranchSide.from_side,  # Invalid - controlling from downstream towards upstream
    ]
    transformer_tap_regulator_input["u_set"] = [50e3, 10e3, 5e3]
    transformer_tap_regulator_input["u_band"] = [200, 200, 200]

    input_data = {
        ComponentType.node: node_input,
        ComponentType.source: source_input,
        ComponentType.transformer: transformer_input,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
    }

    errors = validate_tap_regulator_control_side_topology(input_data)
    assert len(errors) == 1
    assert isinstance(errors[0], InvalidTapRegulatorControlSideError)
    assert set(errors[0].ids) == {31, 32}  # Both second and third regulators are invalid
