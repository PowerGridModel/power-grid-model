# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import inspect
import io
import warnings
from collections.abc import Callable
from pathlib import Path
from typing import Any

import numpy as np
import pytest

from power_grid_model import (
    ComponentType,
    DatasetType,
    PowerGridModel,
    attribute_dtype,
    attribute_empty_value,
    initialize_array,
)
from power_grid_model._core._compatibility import deprecate_positional_arguments
from power_grid_model.utils import (
    json_deserialize,
    json_deserialize_from_file,
    json_serialize,
    json_serialize_to_file,
    msgpack_deserialize,
    msgpack_deserialize_from_file,
    msgpack_deserialize_from_stream,
    msgpack_serialize,
    msgpack_serialize_to_file,
    msgpack_serialize_to_stream,
    self_test,
)
from power_grid_model.validation import (
    assert_valid_batch_data,
    assert_valid_data_structure,
    assert_valid_input_data,
    validate_batch_data,
    validate_input_data,
)


@deprecate_positional_arguments
def _function(shared: str, *, first: str = "default-first", second: str = "default-second") -> tuple[str, str, str]:
    return shared, first, second


class _Class:
    @deprecate_positional_arguments
    def __init__(self, shared: str, *, first: str = "default-first") -> None:
        self.values = (shared, first)

    @deprecate_positional_arguments
    def method(self, *, first: str, second: str = "default-second") -> tuple[str, str]:
        return first, second


def test_v2_call_does_not_warn():
    with warnings.catch_warnings():
        warnings.simplefilter("error")
        assert _function("a", first="b") == ("a", "b", "default-second")
        assert _function(shared="a", second="c") == ("a", "default-first", "c")
        assert _Class("a", first="b").values == ("a", "b")
        assert _Class("a").method(first="b") == ("b", "default-second")


@pytest.mark.parametrize(
    ("args", "kwargs", "expected", "expected_names"),
    [
        (("a", "b"), {}, ("a", "b", "default-second"), "'first'"),
        (("a", "b", "c"), {}, ("a", "b", "c"), "'first', 'second'"),
        (("a", "b"), {"second": "c"}, ("a", "b", "c"), "'first'"),
    ],
)
def test_v1_call_warns_at_callsite(args, kwargs, expected, expected_names):
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always")
        callsite_line = inspect.currentframe().f_lineno + 1
        result = _function(*args, **kwargs)

    assert result == expected
    assert len(caught) == 1
    assert caught[0].category is DeprecationWarning
    assert Path(caught[0].filename).resolve() == Path(__file__).resolve()
    assert caught[0].lineno == callsite_line
    assert f"Passing {expected_names} positionally to _function()" in str(caught[0].message)


def test_v1_call_on_methods():
    with pytest.warns(DeprecationWarning, match=r"'first' positionally to _Class.__init__\(\)"):
        instance = _Class("a", "b")
    assert instance.values == ("a", "b")

    with pytest.warns(DeprecationWarning, match=r"'first', 'second' positionally to _Class.method\(\)"):
        assert instance.method("b", "c") == ("b", "c")


def test_invalid_calls_raise_type_error():
    with pytest.raises(TypeError, match="multiple values for argument 'first'"):
        _function("a", "b", first="c")
    with pytest.raises(TypeError, match="positional argument"):
        _function("a", "b", "c", "d")
    with pytest.raises(TypeError, match="unexpected keyword argument 'unknown'"):
        _function("a", unknown="b")


def test_runtime_signature_is_v1_compatible():
    assert str(inspect.signature(_function)) == (
        "(shared: str, first: str = 'default-first', second: str = 'default-second') -> tuple[str, str, str]"
    )


def test_decorator_rejects_unsupported_callables():
    with pytest.raises(TypeError, match=r"\*args"):
        deprecate_positional_arguments(lambda *args, first=None: None)  # noqa: ARG005
    with pytest.raises(TypeError, match="keyword-only"):
        deprecate_positional_arguments(lambda first: None)  # noqa: ARG005


@pytest.fixture
def input_data():
    node = initialize_array(data_type=DatasetType.input, component_type=ComponentType.node, shape=1)
    node["id"] = 1
    node["u_rated"] = 10.0e3
    source = initialize_array(data_type=DatasetType.input, component_type=ComponentType.source, shape=1)
    source["id"] = 2
    source["node"] = 1
    source["status"] = 1
    source["u_ref"] = 1.0
    return {ComponentType.node: node, ComponentType.source: source}


@pytest.fixture
def update_data():
    source = initialize_array(data_type=DatasetType.update, component_type=ComponentType.source, shape=(2, 1))
    source["id"] = 2
    source["u_ref"] = [[1.0], [1.1]]
    return {ComponentType.source: source}


def _v1_calls(input_data, update_data, tmp_path: Path) -> list[tuple[Callable[..., Any], tuple[Any, ...]]]:
    json_data = json_serialize(data=input_data)
    msgpack_data = msgpack_serialize(data=input_data)
    (tmp_path / "input.json").write_text(json_data, encoding="utf-8")
    (tmp_path / "input.msgpack").write_bytes(msgpack_data)
    model = PowerGridModel(input_data=input_data)
    return [
        (initialize_array, (DatasetType.input, ComponentType.node, 3, True)),
        (initialize_array, (DatasetType.update, ComponentType.node, (2, 3), False)),
        (attribute_dtype, (DatasetType.input, ComponentType.node, "id")),
        (attribute_empty_value, (DatasetType.input, ComponentType.node, "id")),
        (PowerGridModel, (input_data, 60.0)),
        (model.get_indexer, (ComponentType.node, np.array([1]))),
        (json_deserialize, (json_data, None)),
        (json_serialize, (input_data, DatasetType.input, True, 4)),
        (msgpack_deserialize, (msgpack_data, None)),
        (msgpack_serialize, (input_data, DatasetType.input, True)),
        (json_deserialize_from_file, (tmp_path / "input.json", None)),
        (json_serialize_to_file, (tmp_path / "output.json", input_data, DatasetType.input, True, 4)),
        (msgpack_deserialize_from_file, (tmp_path / "input.msgpack", None)),
        (msgpack_serialize_to_file, (tmp_path / "output.msgpack", input_data, DatasetType.input, True)),
        (msgpack_deserialize_from_stream, (io.BytesIO(msgpack_data), None)),
        (msgpack_serialize_to_stream, (io.BytesIO(), input_data, DatasetType.input, True)),
        (validate_input_data, (input_data, None, True)),
        (validate_batch_data, (input_data, update_data, None, True)),
        (assert_valid_input_data, (input_data, None, True)),
        (assert_valid_batch_data, (input_data, update_data, None, True)),
        (assert_valid_data_structure, (input_data, DatasetType.input)),
    ]


def test_public_api_accepts_v1_positional_arguments_with_deprecation_warning(input_data, update_data, tmp_path):
    for function, args in _v1_calls(input_data, update_data, tmp_path):
        with pytest.warns(DeprecationWarning, match="positionally to") as caught:
            function(*args)
        assert len(caught) == 1, function
        assert Path(caught[0].filename).resolve() == Path(__file__).resolve(), function


def test_msgpack_serialize_rejects_json_only_arguments(input_data):
    with pytest.raises(TypeError, match="indent"):
        msgpack_serialize(data=input_data, indent=4)  # type: ignore[call-overload]
    with pytest.raises(TypeError):
        msgpack_serialize(input_data, DatasetType.input, True, 4)  # type: ignore[call-overload]  # noqa: FBT003


def test_self_test_does_not_use_deprecated_calls():
    with warnings.catch_warnings():
        warnings.simplefilter("error", DeprecationWarning)
        self_test()


def test_v1_calls_still_type_check(input_data, tmp_path: Path) -> None:
    """Statically checked by mypy: v1 call styles and v1 return-type usage must keep type-checking."""
    with warnings.catch_warnings():
        warnings.simplefilter("ignore", DeprecationWarning)
        single = initialize_array(DatasetType.input, ComponentType.node, 1, True)  # noqa: FBT003
        batch = initialize_array(DatasetType.update, ComponentType.node, (2, 1), True)  # noqa: FBT003
        assert single.shape == (1,)
        assert batch.shape == (2, 1)
        model = PowerGridModel(input_data, 50.0)
        indexer: list[int] = model.get_indexer(ComponentType.node, np.array([1]))  # was Any in v1
        assert len(indexer) == 1
        serialized: str = json_serialize(input_data, DatasetType.input)
        assert serialized
        writer: Callable[[Path, Any], None] = json_serialize_to_file  # was Any-returning in v1
        writer(tmp_path / "output.json", input_data)
        asserter: Callable[[Any], None] = assert_valid_input_data  # was Any-returning in v1
        asserter(input_data)
