# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from enum import IntEnum

import pytest

from power_grid_model.validation.errors import (
    ComparisonError,
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


def test_single_field_validation_error():
    error = SingleFieldValidationError(component="alpha", field="bravo", ids=[0, 1, 1, 2, 3, 5])
    assert error.component_str == "alpha"
    assert error.field_str == "'bravo'"
    assert str(error) == "Field 'bravo' is not valid for 6 alphas."


def test_multi_field_validation_error():
    error = MultiFieldValidationError(component="charlie", fields=["delta", "echo"], ids=[0, 1, 1, 2, 3, 5])
    assert error.component_str == "charlie"
    assert error.field_str == "'delta' and 'echo'"
    assert str(error) == "Combination of fields 'delta' and 'echo' is not valid for 6 charlies."

    with pytest.raises(ValueError, match="at least two fields"):
        MultiFieldValidationError(component="charlie", fields=["delta"], ids=[])


def test_multi_component_validation_error():
    error = MultiComponentValidationError(
        fields=[("foxtrot", "golf"), ("hotel", "india")],
        ids=[("foxtrot", 0), ("foxtrot", 1), ("foxtrot", 1), ("hotel", 2), ("hotel", 3), ("hotel", 5)],
    )
    assert error.component_str == "foxtrot/hotel"
    assert error.field_str == "foxtrot.golf and hotel.india"
    assert str(error) == "Fields foxtrot.golf and hotel.india are not valid for 6 foxtrots/hotels."

    with pytest.raises(ValueError, match="at least two fields"):
        MultiComponentValidationError(fields=[("foxtrot", "golf")], ids=[])

    with pytest.raises(ValueError, match="at least two components"):
        MultiComponentValidationError(fields=[("foxtrot", "golf"), ("foxtrot", "india")], ids=[])


def test_invalid_enum_value_error():
    class CustomType(IntEnum):
        pass

    error = InvalidEnumValueError(component="kilo", field="lima", ids=[1, 2, 3], enum=CustomType)
    assert error.component == "kilo"
    assert error.field == "lima"
    assert error.ids == [1, 2, 3]
    assert error.enum is CustomType
    assert str(error) == "Field 'lima' contains invalid CustomType values for 3 kilos."


def test_invalid_id_error():
    error = InvalidIdError(component="mike", field="november", ids=[1, 2, 3], ref_components=["oscar", "papa"])
    assert error.component == "mike"
    assert error.field == "november"
    assert error.ids == [1, 2, 3]
    assert error.ref_components == ["oscar", "papa"]
    assert str(error) == "Field 'november' does not contain a valid oscar/papa id for 3 mikes."


def test_comparison_error():
    error = ComparisonError(component="quebec", field="romeo", ids=[1, 2, 3], ref_value=0)
    assert error.component == "quebec"
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
    error = ComparisonError(component="sierra", field="tango", ids=[1, 2, 3], ref_value=0)
    context = error.get_context()
    assert len(context) == 4
    assert context["component"] == "sierra"
    assert context["field"] == "'tango'"
    assert context["ids"] == [1, 2, 3]
    assert context["ref_value"] == "zero"

    context = error.get_context(id_lookup=["Uniform", "Victor"])  # Note that id 0 does not exist in the error ids
    assert context["ids"] == {1: "Victor", 2: None, 3: None}

    context = error.get_context(id_lookup={1: "Victor", 3: "Whiskey"})
    assert context["ids"] == {1: "Victor", 2: None, 3: "Whiskey"}


def test_error_context_tuple_ids():
    error = MultiComponentValidationError(fields=[("a", "x"), ("b", "y")], ids=[("a", 1), ("b", 2), ("a", 3)])
    context = error.get_context(id_lookup={1: "Victor", 3: "Whiskey"})
    assert context["ids"] == {("a", 1): "Victor", ("b", 2): None, ("a", 3): "Whiskey"}
