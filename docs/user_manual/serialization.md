<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Serialization

The power-grid-model provides tools for serialization and deserialization for datasets.

Visit the [Python API Reference](../api_reference/python-api-reference.md#utils) and
[C API Reference](../api_reference/power-grid-model-c-api-reference.rst) for the full documentation of the serialization
tools.

## Serialization format

Currently, two serialization formats are provided:

- [JSON serialization format specification](#json-serialization-format-specification)
- [msgpack serialization format specification](#msgpack-serialization-format-specification)

### JSON serialization format specification

The JSON serialization format is a generic format and supports all
[dataset types and structures](dataset-terminology.md#data-structures).
Rather than providing a full schema, this documentation provides the features by object.

#### JSON schema root object

The format consists of a [`PowerGridModelRoot`](#json-schema-root-object) JSON object containing metadata and the actual
data.

- [`PowerGridModelRoot`](#json-schema-root-object): `Object`
  - `version`: `string` containing the schema version (required, current version is `"1.0"`)
  - `type`: `string` containing the dataset type, e.g. `"input"`, `"update"`, etc.
  - `is_batch`: `boolean` flag that describes whether the dataset is a batch or not.
  - `attributes`: [`Attributes`](#json-schema-attributes-object) containing specified attributes per component type
    (e.g.: `"node"`).
  - `data`: [`Dataset`](#json-schema-dataset-object) containing the actual dataset.

#### JSON schema attributes object

[`Attributes`](#json-schema-attributes-object) contains specified attributes per [`Component`](#json-schema-component)
type (e.g.: `"node"`).
It is only required for those components that contain `HomogeneousComponentData` objects and that data needs to follow
the attributes listed in this object.
It may be empty if for data for all instances certain component is `InhomogeneousComponentData`.
It reduces compression when a dataset largely follows the exact same pattern.

- [`Attributes`](#json-schema-attributes-object): `Object`
  - [`Component`](#json-schema-component): [`ComponentAttributes`](#json-schema-component-attributes) containing the
    desired [`Attribute`](#json-schema-attribute)s for that [`Component`](#json-schema-component).

For example, for an `"update"` dataset that contains only updates to the `"from_status"` attribute of `"branch"`
components, it may be `{"branch": ["from_status"]}`.

#### JSON schema component

A [`Component`](#json-schema-component) string contains the component name (see also the [Components](components.md)
reference).
E.g.: `"node"`

- [`Component`](#json-schema-component): `string`

#### JSON schema component attributes

A [`ComponentAttributes`](#json-schema-component-attributes) array contains the desired
[`Attribute`](#json-schema-attribute)s for a specific [`Component`](#json-schema-component).

- [`ComponentAttributes`](#json-schema-component-attributes): `Array`
  - [`Attribute`](#json-schema-attribute): the attribute

#### JSON schema attribute

A [`Attribute`](#json-schema-attribute) string contains the name of an attribute of a component (see also the
[Components](components.md) reference).
E.g.: `"id"`.

- [`Attribute`](#json-schema-attribute): `string`

#### JSON schema dataset object

The [`Dataset`](#json-schema-dataset-object) object is either a [`SingleDataset`](#json-schema-single-dataset-object) if
the [`is_batch`](#json-schema-root-object) field in the [`PowerGridModelRoot`](#json-schema-root-object) object is
`false`, or a [`BatchDataset`](#json-schema-batch-dataset-object) otherwise.

- [`Dataset`](#json-schema-dataset-object): [`SingleDataset`](#json-schema-single-dataset-object) |
  [`BatchDataset`](#json-schema-batch-dataset-object)

#### JSON schema single dataset object

A [`SingleDataset`](#json-schema-single-dataset-object) object contains the
[`ComponentDataset`](#json-schema-component-dataset-object) for each component.

- [`SingleDataset`](#json-schema-single-dataset-object): `Object`
  - [`Component`](#json-schema-component): [`ComponentDataset`](#json-schema-component-dataset-object) the component
    dataset.

#### JSON schema batch dataset object

A [`BatchDataset`](#json-schema-batch-dataset-object) is an array containing
[`SingleDataset`](#json-schema-single-dataset-object) objects for all batch scenarios.

- [`BatchDataset`](#json-schema-batch-dataset-object): `Array`
  - [`SingleDataset`](#json-schema-single-dataset-object): a single dataset per batch scenario.

**NOTE:** The actual deserialized data representation may be sparse or dense, depending on the contents.
Regardless of whether the deserialized data representation data is sparse or dense, the serialization format remains the
same.

#### JSON schema component dataset object

A [`ComponentDataset`](#json-schema-component-dataset-object) is an array of
[`ComponentData`](#json-schema-component-data-object)s per component of the same type.

- [`ComponentDataset`](#json-schema-component-dataset-object): `Array`
  - [`ComponentData`](#json-schema-component-data-object): the data per single component.

**NOTE:** The actual deserialized data representation may be row based or columnar, depending on the `data_filter`
provided at deserialization (Check {py:function}`json_deserialize <power_grid_model.utils.json_deserialize>` for
example).
Regardless of whether the deserialized data representation data is row based or columnar, the serialization format
remains the same.

#### JSON schema component data object

A [`ComponentData`](#json-schema-component-data-object) object is either a
[`HomogeneousComponentData`](#json-schema-homogeneous-component-data-object) object or an
[`InhomogeneousComponentData`](#json-schema-inhomogeneous-component-data-object) object

- [`ComponentData`](#json-schema-component-data-object):
  [`HomogeneousComponentData`](#json-schema-homogeneous-component-data-object) |
  [`InhomogeneousComponentData`](#json-schema-inhomogeneous-component-data-object)

#### JSON schema homogeneous component data object

A [`HomogeneousComponentData`](#json-schema-homogeneous-component-data-object) object contains the actual values of a
certain component following the exact order of the attributes listed in the [`attributes`](#json-schema-root-object)
field in the [`PowerGridModelRoot`](#json-schema-root-object) object.

- [`HomogeneousComponentData`](#json-schema-homogeneous-component-data-object): `Array`
  - [`AttributeValue`](#json-schema-attribute-value): the value of each attribute.

#### JSON schema inhomogeneous component data object

An [`InhomogeneousComponentData`](#json-schema-inhomogeneous-component-data-object) object contains actual values per
attribute of a certain component.
Contrary to the [`HomogeneousComponentData`](#json-schema-homogeneous-component-data-object), it lists the names of the
attributes for which the values are specified, so the attributes may be in arbitrary order and do not have to follow the
schema listed in the [`attributes`](#json-schema-root-object) field in the
[`PowerGridModelRoot`](#json-schema-root-object) object.

- [`InhomogeneousComponentData`](#json-schema-inhomogeneous-component-data-object): `Object`
  - [`Attribute`](#json-schema-attribute): [`AttributeValue`](#json-schema-attribute-value): the value of each attribute
    per attribute.

#### JSON schema attribute value

The [`AttributeValue`](#json-schema-attribute-value) contains the actual value of an attribute.
The value can be any of [`null`](#json-schema-null-absence-of-value) if it is not provided, or any of
[`int32_t`](#json-schema-int32_t), [`int8_t`](#json-schema-int8_t), [`double`](#json-schema-double),
[`RealValueInput`](#json-schema-realvalueinput) or [`RealValueOutput`](#json-schema-realvalueoutput).
The type is listed for each attribute in [Components](components.md).

- [`AttributeValue`](#json-schema-attribute-value): [`null`](#json-schema-null-absence-of-value) |
  [`int32_t`](#json-schema-int32_t) | [`int8_t`](#json-schema-int8_t) | [`double`](#json-schema-double) |
  [`RealValueInput`](#json-schema-realvalueinput) | [`RealValueOutput`](#json-schema-realvalueoutput)

#### JSON schema null (absence of value)

**NOTE:** This is the [JSON](#json-serialization-format-specification)-specific version of
[absence of value](#json-schema-null-absence-of-value).
For [`msgpack`](#msgpack-serialization-format-specification), refer to
[absence of value for `msgpack`](#msgpack-schema-nil-absence-of-value).

- [absence of value](#json-schema-null-absence-of-value): `null`

**NOTE:** any `nan` values for concrete `number` types represent absence of value and are represented by
[`null`](#json-schema-null-absence-of-value) in the [JSON schema](#json-serialization-format-specification).

#### JSON schema int32_t

An [`int32_t`](#json-schema-int32_t) is an integer `number` value usually representing an ID.
It may be in the inclusive range $\left[-2^{31},+2^{31} - 1\right]$.
The type is listed for each attribute in [Components](components.md).

- [`int32_t`](#json-schema-int32_t): `number`

**NOTE:** the special value `-2147483648` represents absence of value and may also be represented by
[`null`](#json-schema-null-absence-of-value) in the [JSON schema](#json-serialization-format-specification).

#### JSON schema int8_t

An [`int8_t`](#json-schema-int8_t) integer `number` value usually representing an enumeration value or a discrete
setting.
It may be in the inclusive range $\left[-2^{7},+2^{7} - 1\right]$.
The type is listed for each attribute in [Components](components.md).

- [`int8_t`](#json-schema-int8_t): `number`

**NOTE:** the special value `-128` represents absence of value and may also be represented by
[`null`](#json-schema-null-absence-of-value) in the [JSON schema](#json-serialization-format-specification).

#### JSON schema double

**NOTE:** This is the [JSON](#json-serialization-format-specification)-specific version of
[`double`](#json-schema-double).
For [`msgpack`](#msgpack-serialization-format-specification), refer to [`double` for `msgpack`](#msgpack-schema-double).

A [`double`](#json-schema-double) floating point `number` or `string` value, usually representing a real.
If it is a `string`, it shall be either `"inf"` or `"+inf"` for positive infinity, or `"-inf"` for negative infinity.
Any other values are unsupported.
The type is listed for each attribute in [Components](components.md).

- [`double`](#json-schema-double): `number`|`string`

**NOTE:** the special value `nan` represents absence of value and is represented by
[`null`](#json-schema-null-absence-of-value) in the [JSON schema](#json-serialization-format-specification).

#### JSON schema RealValueInput

A [`RealValueInput`](#json-schema-realvalueinput) of which the data format depends on the type of calculation.
For symmetric components, it is a [`double`](#json-schema-double).
For asymmetric components, it is an Array of size 3 containing [`double`](#json-schema-double) or
[`null`](#json-schema-null-absence-of-value) values.
The type is listed for each attribute in [Components](components.md).

- [`RealValueInput`](#json-schema-realvalueinput): [`double`](#json-schema-double) for symmetric calculations.
- [`RealValueInput`](#json-schema-realvalueinput): `Array` of size 3 for asymmetric calculations, one for each phase.
  - [`double`](#json-schema-double) | [`null`](#json-schema-null-absence-of-value): the value for the `_a` phase, if
    specified.
  - [`double`](#json-schema-double) | [`null`](#json-schema-null-absence-of-value): the value for the `_b` phase, if
    specified.
  - [`double`](#json-schema-double) | [`null`](#json-schema-null-absence-of-value): the value for the `_c` phase, if
    specified.

#### JSON schema RealValueOutput

A [`RealValueOutput`](#json-schema-realvalueoutput) of which the data format depends on the type of calculation.
For symmetric calculations, it is a [`double`](#json-schema-double).
For asymmetric calculations, it is an Array of size 3 containing [`double`](#json-schema-double) or
[`null`](#json-schema-null-absence-of-value) values.
The type is listed for each attribute in [Components](components.md).

- [`RealValueOutput`](#json-schema-realvalueoutput): [`double`](#json-schema-double) for symmetric calculations.
- [`RealValueOutput`](#json-schema-realvalueoutput): `Array` of size 3 for asymmetric calculations, one for each phase.
  - [`double`](#json-schema-double) | [`null`](#json-schema-null-absence-of-value): the value for the `_a` phase, if
    specified.
  - [`double`](#json-schema-double) | [`null`](#json-schema-null-absence-of-value): the value for the `_b` phase, if
    specified.
  - [`double`](#json-schema-double) | [`null`](#json-schema-null-absence-of-value): the value for the `_c` phase, if
    specified.

#### JSON schema single dataset example

The following example contains an input dataset.
The nodes and sym_loads are represented using
[`HomogeneousComponentData`](#json-schema-homogeneous-component-data-object),
the lines are represented using [`InomogeneousComponentData`](#json-schema-inhomogeneous-component-data-object),
while the sources are represented using a mixture of
[`HomogeneousComponentData`](#json-schema-homogeneous-component-data-object) and
[`InomogeneousComponentData`](#json-schema-inhomogeneous-component-data-object).

```json
{
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {
    "node": ["id", "u_rated"],
    "sym_load": ["id", "node", "status", "type", "p_specified", "q_specified"],
    "source": ["id", "node", "status", "u_ref", "sk"]
  },
  "data": {
    "node": [
      [1, 10.5e3],
      [2, 10.5e3],
      [3, 10.5e3],
      [100, "inf"],
      [101, "+inf"],
      [102, "-inf"]
    ],
    "line": [
      {
        "id": 4,
        "from_node": 1,
        "to_node": 2,
        "from_status": 1,
        "to_status": 1,
        "r1": 0.11,
        "x1": 0.12,
        "c1": 4e-05,
        "tan1": 0.1,
        "i_n": 500.0
      },
      {
        "id": 5,
        "from_node": 2,
        "to_node": 3,
        "from_status": 1,
        "to_status": 1,
        "r1": 0.15,
        "x1": 0.16,
        "c1": 5e-05,
        "tan1": 0.12,
        "i_n": 550.0
      }
    ],
    "source": [
      [15, 1, 1, 1.03, 1e20],
      [16, 1, 1, 1.04, null],
      {
        "id": 17,
        "node": 1,
        "status": 1,
        "u_ref": 1.03,
        "sk": 1e10,
        "rx_ratio": 0.2
      }
    ],
    "sym_load": [
      [7, 2, 1, 0, 1.01e6, 0.21e6],
      [8, 3, 1, 0, 1.02e6, 0.22e6]
    ]
  }
}
```

#### JSON schema batch dataset example

The following example contains a batch update dataset containing two sym_loads and one asym_load.
Not every scenario updates all components and attributes, reducing the total amount of data necessary to represent the
batch dataset.

```json
{
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {
    "sym_load": ["id", "p_specified", "q_specified"],
    "asym_load": ["id", "p_specified"]
  },
  "data": [
    {
      "sym_load": [
        [7, 20.0, 50.0]
      ],
      "asym_load": [
        [9, [100.0, null, 200.0]]
      ]
    },
    {
      "asym_load": [
        [9, null]
      ]
    },
    {
      "sym_load": [
        [7, null, 10.0],
        {
          "id": 8,
          "status": 0
        }
      ],
      "asym_load": [
        {
          "id": 9,
          "q_specified": [70.0, 80.0, 90.0]
        }
      ]
    }
  ]
}
```

### msgpack serialization format specification

The msgpack serialization format is a compressed version of the
[JSON serialization format](#json-serialization-format-specification) and all features supported for JSON are also
supported for msgpack.

#### msgpack schema nil (absence of value)

**NOTE:** This is the [`msgpack`](#msgpack-serialization-format-specification)-specific version of
[absence of value](#msgpack-schema-nil-absence-of-value).
For [JSON](#json-serialization-format-specification), refer to
[absence of value for JSON](#json-schema-null-absence-of-value).

- [absence of value](#msgpack-schema-nil-absence-of-value): `nil` (the byte `\xc0`)

**NOTE:** any `nan` values for concrete `number` types represent absence of value are represented by
[`nil`](#msgpack-schema-nil-absence-of-value) in the [msgpack schema](#msgpack-serialization-format-specification).

#### msgpack schema double

**NOTE:** This is the [`msgpack`](#msgpack-serialization-format-specification)-specific version of
[`double`](#msgpack-schema-double).
For [JSON](#json-serialization-format-specification), refer to [`double` for JSON](#json-schema-double).

A [`double`](#msgpack-schema-double) floating point `number` value usually representing a real.
Infinities are represented using the [IEEE 754](https://en.wikipedia.org/wiki/IEEE_754) standard for double-precision
floating point values representation for infinities.
Any other values are unsupported.
The type is listed for each attribute in [Components](components.md).

- [`double`](#msgpack-schema-double): `number`

**NOTE:** the special value `nan` represents absence of value and may also be represented by
[`nil`](#msgpack-schema-nil-absence-of-value) in the [msgpack schema](#msgpack-serialization-format-specification).
