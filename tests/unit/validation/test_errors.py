# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from enum import IntEnum

import pytest

from power_grid_model import ComponentType
from power_grid_model.validation.errors import (
    ComparisonError,
    InvalidAssociatedEnumValueError,
    InvalidEnumValueError,
    InvalidIdError,
    MultiComponentValidationError,
    MultiFieldValidationError,
    SingleFieldValidationError,
    ValidationError,
)


def test_validation_error():
    error = ValidationError()
    assert str(error) == "An unknown validation error occurred."
    assert error.component_str == str(None)
    assert error.field_str == str(None)
    assert not error.ids


def test_single_field_validation_error():
    error = SingleFieldValidationError(component=ComponentType.node, field="bravo", ids=[0, 1, 1, 2, 3, 5])
    assert error.component_str == "node"
    assert error.field_str == "'bravo'"
    assert str(error) == "Field 'bravo' is not valid for 6 nodes."


def test_multi_field_validation_error():
    error = MultiFieldValidationError(component=ComponentType.node, fields=["delta", "echo"], ids=[0, 1, 1, 2, 3, 5])
    assert error.component_str == "node"
    assert error.field_str == "'delta' and 'echo'"
    assert str(error) == "Combination of fields 'delta' and 'echo' is not valid for 6 nodes."

    with pytest.raises(ValueError, match="at least two fields"):
        MultiFieldValidationError(component=ComponentType.node, fields=["delta"], ids=[])


def test_multi_component_validation_error():
    error = MultiComponentValidationError(
        fields=[(ComponentType.node, "golf"), (ComponentType.line, "india")],
        ids=[
            (ComponentType.node, 0),
            (ComponentType.node, 1),
            (ComponentType.node, 1),
            (ComponentType.line, 2),
            (ComponentType.line, 3),
            (ComponentType.line, 5),
        ],
    )
    assert error.component_str == "line/node"
    assert error.field_str == "line.india and node.golf"
    assert str(error) == "Fields line.india and node.golf are not valid for 6 lines/nodes."

    with pytest.raises(ValueError, match="at least two fields"):
        MultiComponentValidationError(fields=[(ComponentType.node, "golf")], ids=[])

    with pytest.raises(ValueError, match="at least two components"):
        MultiComponentValidationError(fields=[(ComponentType.node, "golf"), (ComponentType.node, "india")], ids=[])


def test_invalid_enum_value_error():
    class CustomType(IntEnum):
        pass

    error = InvalidEnumValueError(component=ComponentType.node, field="lima", ids=[1, 2, 3], enum=CustomType)
    assert error.component == "node"
    assert error.field == "lima"
    assert error.ids == [1, 2, 3]
    assert error.enum is CustomType
    assert str(error) == "Field 'lima' contains invalid CustomType values for 3 nodes."


def test_invalid_id_error():
    error = InvalidIdError(ComponentType.node, field="november", ids=[1, 2, 3], ref_components=["oscar", "papa"])
    assert error.component == "node"
    assert error.field == "november"
    assert error.ids == [1, 2, 3]
    assert error.ref_components == ["oscar", "papa"]
    assert str(error) == "Field 'november' does not contain a valid oscar/papa id for 3 nodes."


def test_invalid_id_error_with_filters():
    error = InvalidIdError(
        ComponentType.node,
        field="november",
        ids=[1, 2, 3],
        ref_components=["oscar", "papa"],
        filters={"foo": "bar", "baz": ComponentType.node},
    )
    assert error.component == "node"
    assert error.field == "november"
    assert error.ids == [1, 2, 3]
    assert error.ref_components == ["oscar", "papa"]
    assert error.filters_str == "(foo=bar, baz=node)"
    assert str(error) == "Field 'november' does not contain a valid oscar/papa id for 3 nodes. (foo=bar, baz=node)"


def test_comparison_error():
    error = ComparisonError(component=ComponentType.node, field="romeo", ids=[1, 2, 3], ref_value=0)
    assert error.component == "node"
    assert error.field == "romeo"
    assert error.ids == [1, 2, 3]
    assert error.ref_value == 0
    assert ComparisonError(component="", field="", ids=[], ref_value=123).ref_value_str == "123"
    assert ComparisonError(component="", field="", ids=[], ref_value=123.456).ref_value_str == "123.456"
    assert ComparisonError(component="", field="", ids=[], ref_value=0).ref_value_str == "zero"
    assert ComparisonError(component="", field="", ids=[], ref_value=0.0).ref_value_str == "zero"
    assert ComparisonError(component="", field="", ids=[], ref_value=1).ref_value_str == "one"
    assert ComparisonError(component="", field="", ids=[], ref_value=1.0).ref_value_str == "one"
    assert ComparisonError(component="", field="", ids=[], ref_value="sierra").ref_value_str == "sierra"


def test_error_context():
    error = ComparisonError(component=ComponentType.node, field="tango", ids=[1, 2, 3], ref_value=0)
    context = error.get_context()
    assert len(context) == 4
    assert context["component"] == "node"
    assert context["field"] == "'tango'"
    assert context["ids"] == [1, 2, 3]
    assert context["ref_value"] == "zero"

    context = error.get_context(id_lookup=["Uniform", "Victor"])  # Note that id 0 does not exist in the error ids
    assert context["ids"] == {1: "Victor", 2: None, 3: None}

    context = error.get_context(id_lookup={1: "Victor", 3: "Whiskey"})
    assert context["ids"] == {1: "Victor", 2: None, 3: "Whiskey"}


def test_error_context_tuple_ids():
    error = MultiComponentValidationError(
        fields=[(ComponentType.node, "x"), ("b", "y")], ids=[(ComponentType.node, 1), ("b", 2), (ComponentType.node, 3)]
    )
    context = error.get_context(id_lookup={1: "Victor", 3: "Whiskey"})
    assert context["ids"] == {("node", 1): "Victor", ("b", 2): None, ("node", 3): "Whiskey"}


def test_invalid_associated_enum_value_error():
    class CustomType(IntEnum):
        pass

    error = InvalidAssociatedEnumValueError(
        component=ComponentType.node, fields=["bar", "baz"], ids=[1, 2], enum=[CustomType]
    )
    assert error.component == "node"
    assert error.field == ["bar", "baz"]
    assert error.ids == [1, 2]
    assert len(error.enum) == 1
    for actual, expected in zip(error.enum, [CustomType]):
        assert actual is expected
    assert str(error) == "The combination of fields 'bar' and 'baz' results in invalid CustomType values for 2 nodes."
