# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Error classes
"""
import re
from abc import ABC
from enum import Enum
from typing import Any, Dict, Iterable, List, Optional, Set, Tuple, Type, Union


class ValidationError(ABC):
    """
    The Validation Error is an abstract base class which should be extended by all validation errors. It supplies
    three public member variables: component, field and ids; storing information about the origin of the validation
    error. Error classes can extend the public members. For example:

        NotBetweenError(ValidationError):
            component = 'vehicle'
            field = 'direction'
            id = [3, 14, 15, 92, 65, 35]
            ref_value = (-3.1416, 3.1416)

    For convenience, a human readable representation of the error is supplied using the str() function.
    I.e. print(str(error)) will print a human readable error message like:

        Field `direction` is not between -3.1416 and 3.1416 for 6 vehicles

    """

    component: Optional[Union[str, List[str]]] = None
    """
    The component, or components, to which the error applies.
    """

    field: Optional[Union[str, List[str], List[Tuple[str, str]]]] = None
    """
    The field, or fields, to which the error applies. A field can also be a tuple (component, field) when multiple
    components are being addressed.
    """

    ids: Optional[Union[List[int], List[Tuple[str, int]]]] = None
    """
    The object identifiers to which the error applies. A field object identifier can also be a tuple (component, id)
    when multiple components are being addressed.
    """

    _message: str = "An unknown validation error occurred."

    _delimiter: str = " and "

    @property
    def component_str(self) -> str:
        """
        A string representation of the component to which this error applies
        """
        return str(self.component)

    @property
    def field_str(self) -> str:
        """
        A string representation of the field to which this error applies
        """
        return f"'{self.field}'"

    def get_context(self, id_lookup: Optional[Union[List[str], Dict[int, str]]] = None) -> Dict[str, Any]:
        """
        Returns a dictionary that supplies (human readable) information about this error. Each member variable is
        included in the dictionary. If a function {field_name}_str() exists, the value is overwritten by that function.

        Args:
            id_lookup: A list or dict (int->str) containing textual object ids
        """
        context = self.__dict__.copy()
        if id_lookup:
            if isinstance(id_lookup, list):
                id_lookup = dict(enumerate(id_lookup))
            context["ids"] = (
                {i: id_lookup.get(i[1] if isinstance(i, tuple) else i) for i in self.ids} if self.ids else set()
            )
        for key in context:
            if hasattr(self, key + "_str"):
                context[key] = str(getattr(self, key + "_str"))
        return context

    def __str__(self) -> str:
        n_objects = len(self.ids) if self.ids else 0
        context = self.get_context()
        context["n"] = n_objects
        context["objects"] = context.get("component", "object")
        if n_objects != 1:
            context["objects"] = re.sub(r"([a-z_]+)", r"\1s", context["objects"])
        return self._message.format(**context).strip()

    def __repr__(self) -> str:
        context = " ".join(f"{key}={value}" for key, value in self.get_context().items())
        return f"<{type(self).__name__}: {context}>"

    def __eq__(self, other):
        return (
            type(self) is type(other)
            and self.component == other.component
            and self.field == other.field
            and self.ids == other.ids
        )


class SingleFieldValidationError(ValidationError):
    """
    Base class for an error that applies to a single field in a single component
    """

    _message = "Field {field} is not valid for {n} {objects}."
    component: str
    field: str
    ids: List[int]

    def __init__(self, component: str, field: str, ids: Iterable[int]):
        """
        Args:
            component: Component name
            field: Field name
            ids: List of component IDs (not row indices)
        """
        self.component = component
        self.field = field
        self.ids = sorted(ids)


class MultiFieldValidationError(ValidationError):
    """
    Base class for an error that applies to multiple fields in a single component
    """

    _message = "Combination of fields {field} is not valid for {n} {objects}."
    component: str
    field: List[str]
    ids: List[int]

    def __init__(self, component: str, fields: List[str], ids: List[int]):
        """
        Args:
            component: Component name
            fields: List of field names
            ids: List of component IDs (not row indices)
        """
        self.component = component
        self.field = sorted(fields)
        self.ids = sorted(ids)

        if len(self.field) < 2:
            raise ValueError(f"{type(self).__name__} expects at least two fields: {self.field}")

    @property
    def field_str(self) -> str:
        return self._delimiter.join(f"'{field}'" for field in self.field)


class MultiComponentValidationError(ValidationError):
    """
    Base class for an error that applies to multiple component, and as a consequence also to multiple fields.
    Even if both fields have the same name, they are considered to be different fields and notated as such.
    E.g. the two fields `id` fields of the `node` and `line` component: [('node', 'id'), ('line', 'id')].
    """

    component: List[str]
    field: List[Tuple[str, str]]
    ids: List[Tuple[str, int]]
    _message = "Fields {field} are not valid for {n} {objects}."

    def __init__(self, fields: List[Tuple[str, str]], ids: List[Tuple[str, int]]):
        """
        Args:
            fields: List of field names, formatted as tuples (component, field)
            ids: List of component IDs (not row indices), formatted as tuples (component, id)
        """
        self.component = sorted(set(component for component, _ in fields))
        self.field = sorted(fields)
        self.ids = sorted(ids)

        if len(self.field) < 2:
            raise ValueError(f"{type(self).__name__} expects at least two fields: {self.field}")
        if len(self.component) < 2:
            raise ValueError(f"{type(self).__name__} expects at least two components: {self.component}")

    @property
    def component_str(self) -> str:
        return "/".join(self.component)

    @property
    def field_str(self) -> str:
        return self._delimiter.join(f"{component}.{field}" for component, field in self.field)


class NotIdenticalError(SingleFieldValidationError):
    """
    The value is not unique within a single column in a dataset
    E.g. When two nodes share the same id.
    """

    _message = "Field {field} is not unique for {n} {objects}: {num_unique} different values."
    values: List[Any]
    unique: Set[Any]
    num_unique: int

    def __init__(self, component: str, field: str, ids: Iterable[int], values: List[Any]):
        super().__init__(component, field, ids)
        self.values = values
        self.unique = set(self.values)
        self.num_unique = len(self.unique)


class NotUniqueError(SingleFieldValidationError):
    """
    The value is not unique within a single column in a dataset
    E.g. When two nodes share the same id.
    """

    _message = "Field {field} is not unique for {n} {objects}."


class MultiComponentNotUniqueError(MultiComponentValidationError):
    """
    The value is not unique between multiple columns in multiple components
    E.g. When a node and a line share the same id.
    """

    _message = "Fields {field} are not unique for {n} {objects}."


class InvalidEnumValueError(SingleFieldValidationError):
    """
    The value is not a valid value in the supplied enumeration type.
    E.g. a sym_load has a non existing LoadGenType.
    """

    _message = "Field {field} contains invalid {enum} values for {n} {objects}."
    enum: Type[Enum]

    def __init__(self, component: str, field: str, ids: List[int], enum: Type[Enum]):
        super().__init__(component, field, ids)
        self.enum = enum

    @property
    def enum_str(self) -> str:
        """
        A string representation of the field to which this error applies.
        """
        return self.enum.__name__

    def __eq__(self, other):
        return super().__eq__(other) and self.enum == other.enum


class SameValueError(MultiFieldValidationError):
    """
    The value of two fields is equal.
    E.g. A line has the same from_node as to_node.
    """

    _message = "Same value for {field} for {n} {objects}."


class NotBooleanError(SingleFieldValidationError):
    """
    Invalid boolean value. Boolean fields don't really exist in our data structure, they are 1-byte signed integers and
    should contain either a 0 (=False) or a 1 (=True).
    """

    _message = "Field {field} is not a boolean (0 or 1) for {n} {objects}."


class MissingValueError(SingleFieldValidationError):
    """
    A required value was missing, i.e. NaN.
    """

    _message = "Field {field} is missing for {n} {objects}."


class IdNotInDatasetError(SingleFieldValidationError):
    """
    An object identifier does not exist in the original data.
    E.g. An update data set contains a record for a line that doesn't exist in the input data set.
    """

    _message = "ID does not exist in {ref_dataset} for {n} {objects}."
    ref_dataset: str

    def __init__(self, component: str, ids: List[int], ref_dataset: str):
        super().__init__(component=component, field="id", ids=ids)
        self.ref_dataset = ref_dataset

    def __eq__(self, other):
        return super().__eq__(other) and self.ref_dataset == other.ref_dataset


class InvalidIdError(SingleFieldValidationError):
    """
    An object identifier does not refer to the right type of object.
    E.g. An line's from_node refers to a source.

    Filters can have been applied to check a subset of the records. E.g. This error may apply only to power_sensors
    that are said to be connected to a source (filter: measured_terminal_type=source). This useful to spot validation
    mistakes, due to ambiguity.

    E.g. when a sym_power_sensor is connected to a load, but measured_terminal_type is accidentally set to 'source',
    the error is:

    "Field `measured_object` does not contain a valid source id for 1 sym_power_sensor. (measured_terminal_type=source)"

    """

    _message = "Field {field} does not contain a valid {ref_components} id for {n} {objects}. {filters}"
    ref_components: List[str]

    def __init__(
        self,
        component: str,
        field: str,
        ids: List[int],
        ref_components: Union[str, List[str]],
        filters: Optional[Dict[str, Any]] = None,
    ):
        # pylint: disable=too-many-arguments
        super().__init__(component=component, field=field, ids=ids)
        self.ref_components = [ref_components] if isinstance(ref_components, str) else ref_components
        self.filters = filters if filters else None

    @property
    def ref_components_str(self):
        """
        A string representation of the components to which this error applies
        """
        return "/".join(self.ref_components)

    @property
    def filters_str(self):
        """
        A string representation of the filters that have been applied to the data to which this error refers
        """
        if not self.filters:
            return ""
        filters = ", ".join(f"{k}={v.name if isinstance(v, Enum) else v}" for k, v in self.filters.items())
        return f"({filters})"

    def __eq__(self, other):
        return super().__eq__(other) and self.ref_components == other.ref_components and self.filters == other.filters


class TwoValuesZeroError(MultiFieldValidationError):
    """
    A record has a 0.0 value in two fields at the same time.
    E.g. A line's `r1`, `x1` are both 0.
    """

    _message = "Fields {field} are both zero for {n} {objects}."


class ComparisonError(SingleFieldValidationError):
    """
    Base class for comparison errors.
    E.g. A transformer's `i0` is not greater or equal to it's `p0` divided by it's `sn`
    """

    _message = "Invalid {field}, compared to {ref_value} for {n} {objects}."

    RefType = Union[int, float, str, Tuple[Union[int, float, str], ...]]

    def __init__(self, component: str, field: str, ids: List[int], ref_value: "ComparisonError.RefType"):
        super().__init__(component, field, ids)
        self.ref_value = ref_value

    @property
    def ref_value_str(self):
        """
        A string representation of the reference value. E.g. 'zero', 'one', 'field_a and field_b' or '123'.
        """
        if isinstance(self.ref_value, tuple):
            return self._delimiter.join(map(str, self.ref_value))
        if self.ref_value == 0:
            return "zero"
        if self.ref_value == 1:
            return "one"
        return str(self.ref_value)

    def __eq__(self, other):
        return super().__eq__(other) and self.ref_value == other.ref_value


class NotGreaterThanError(ComparisonError):
    """
    The value of a field is not greater than a reference value or expression.
    """

    _message = "Field {field} is not greater than {ref_value} for {n} {objects}."


class NotGreaterOrEqualError(ComparisonError):
    """
    The value of a field is not greater or equal to a reference value or expression.
    """

    _message = "Field {field} is not greater than (or equal to) {ref_value} for {n} {objects}."


class NotLessThanError(ComparisonError):
    """
    The value of a field is not less than a reference value or expression.
    """

    _message = "Field {field} is not smaller than {ref_value} for {n} {objects}."


class NotLessOrEqualError(ComparisonError):
    """
    The value of a field is not smaller or equal to a reference value or expression.
    """

    _message = "Field {field} is not smaller than (or equal to) {ref_value} for {n} {objects}."


class NotBetweenError(ComparisonError):
    """
    The value of a field is not between two a reference values or expressions (exclusive).
    """

    _message = "Field {field} is not between {ref_value} for {n} {objects}."


class NotBetweenOrAtError(ComparisonError):
    """
    The value of a field is not between two a reference values or expressions (inclusive).
    """

    _message = "Field {field} is not between (or at) {ref_value} for {n} {objects}."


class InfinityError(SingleFieldValidationError):
    """
    The value of a field is infinite.
    """

    _message = "Field {field} is infinite for {n} {objects}."


class TransformerClockError(MultiFieldValidationError):
    """
    Invalid clock number.
    """

    _message = (
        "Invalid clock number for {n} {objects}. "
        "If one side has wye winding and the other side has not, the clock number should be odd. "
        "If either both or none of the sides have wye winding, the clock number should be even."
    )


class FaultPhaseError(MultiFieldValidationError):
    """
    The fault phase does not match the fault type.
    """

    _message = "The fault phase is not applicable to the corresponding fault type for {n} {objects}."
