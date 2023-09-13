<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Serialization

The power-grid-model provides tools for serialization and deserialization for datasets.

Visit the [Python API Reference](../api_reference/python-api-reference.md#utils) and [C API Reference](../api_reference/power-grid-model-c-api-reference.rst#Serialization) for the full documentation of the serialization tools.

## Serialization format

Currently, two serialization formats are provided:

- [Serialization](#serialization)
  - [Serialization format](#serialization-format)
    - [JSON serialization format specification](#json-serialization-format-specification)
      - [JSON schema root object](#json-schema-root-object)
      - [JSON schema attributes object](#json-schema-attributes-object)
      - [JSON schema dataset object](#json-schema-dataset-object)
      - [JSON schema single dataset object](#json-schema-single-dataset-object)
      - [JSON schema batch dataset object](#json-schema-batch-dataset-object)
      - [JSON schema component dataset object](#json-schema-component-dataset-object)
      - [JSON schema component data object](#json-schema-component-data-object)
      - [JSON schema homogeneous component data object](#json-schema-homogeneous-component-data-object)
      - [JSON schema inhomogeneous component dataset object](#json-schema-inhomogeneous-component-dataset-object)
      - [JSON schema attribute value](#json-schema-attribute-value)
      - [JSON schema int32\_t](#json-schema-int32_t)
      - [JSON schema int8\_t](#json-schema-int8_t)
      - [JSON schema double](#json-schema-double)
      - [JSON schema RealValueOutput](#json-schema-realvalueoutput)
      - [JSON schema component](#json-schema-component)
      - [JSON schema attribute](#json-schema-attribute)
      - [JSON schema attributes](#json-schema-attributes)
    - [msgpack serialization format specification](#msgpack-serialization-format-specification)

### JSON serialization format specification

The JSON serialization format is a generic format and supports all [dataset types and structures](dataset-terminology.md#data-structures).
Rather than providing a full schema, this documentation provides the features by object.

#### JSON schema root object

The format consists of a [`PowerGridModelRoot`](#json-schema-root-object) JSON object containing metadata and the actual data.

* `PowerGridModelRoot`: `Object`
  * `version`: `string` containing the schema version (required, current version is `"1.0"`)
  * `type`: `string` containing the dataset type, e.g. `"input"`, `"update"`, ...
  * `is_batch`: `boolean` flag that describes whether the dataset is a batch or not.
  * `attributes`: [`Attributes`](#json-schema-attributes-object) containing specified attributes per component type (e.g.: `"node"`).
  * `data`: [`Dataset`](#json-schema-dataset-object) containing the actual dataset.

#### JSON schema attributes object

[`Attributes`](#json-schema-attributes-object) contains specified attributes per component type (e.g.: `"node"`).
It is only required for those components that contain `HomogeneousComponentData` objects and that data needs to follow the attributes listed in this object.
It may be empty if for data for all instances certain component is `InhomogeneousComponentData`.
It reduces compression when a dataset largely follows the exact same pattern.

* `Attributes`: `Object`
  * `Component`: `ComponentAttributes` containing the desired [`Attribute`s](#json-schema-attribute) for that [`Component`](#json-schema-component).

For example, for an `"update"` dataset that contains only updates to the `"from_status"` attribute of `"branch"` components, it may be `{"branch": ["from_status"]}`.

#### JSON schema dataset object

The dataset object is either a [`SingleDataset`](#json-schema-single-dataset-object), or a [`BatchDataset`](#json-schema-batch-dataset-object)

* `Dataset`: [`SingleDataset`](#json-schema-single-dataset-object) | [`BatchDataset`](#json-schema-batch-dataset-object)

#### JSON schema single dataset object

An object of components containing the [`ComponentDataset`](#json-schema-component-dataset-object) for each component.

* `SingleDataset`: `Object`
  * `component`: [`ComponentDataset`](#json-schema-component-dataset-object) the component dataset.

#### JSON schema batch dataset object

An array containing [`SingleDataset`](#json-schema-single-dataset-object) objects for all batch scenarios.

* `BatchDataset`: `Array`
  * [`SingleDataset`](#json-schema-single-dataset-object): a single dataset per batch scenario.

#### JSON schema component dataset object

An array of [`ComponentData`s](#json-schema-component-data-object) per component of the same type.

* `ComponentDataset`: `Array`
  * [`ComponentData`](#json-schema-component-data-object): the data per single component.

#### JSON schema component data object

A component data object is either a [`HomogeneousComponentData`](#json-schema-homogeneous-component-data-object) object or an [`InhomogeneousComponentData`](#json-schema-inhomogeneous-component-data-object) object

* `ComponentData`: [`HomogeneousComponentData`](#json-schema-homogeneous-component-data-object) | [`InhomogeneousComponentData`](#json-schema-inhomogeneous-component-data-object)

#### JSON schema homogeneous component data object

A homogeneous component data object contains the actual values of a certain component following the exact order of the attributes listed in the [`attributes`](#json-schema-attributes) field.

* `HomogeneousComponentData`: `Array`
  * [`AttributeValue`](#json-schema-attribute-value): the value of each attribute.

#### JSON schema inhomogeneous component dataset object

An inhomogeneous component data object contains actual values per attribute of a certain component.
Contrary to the [`HomogeneousComponentData`](#json-schema-homogeneous-component-data-object), it lists the names of the attributes for which the values are specified, so the attributes may be in arbitrary order and do not have to follow the schema listed in the [`attributes`](#json-schema-root-object) field in the root object.

* `HomogeneousComponentData`: `Object`
  * `Attribute`: [`AttributeValue`](#json-schema-attribute-value): the value of each attribute per attribute.

#### JSON schema attribute value

The actual value of an attribute. The value can be any of `null` if it is not provided, or any of [`int32_t`](#json-schema-int32_t), [`int8_t`](#json-schema-int8_t), [`double`](#json-schema-double) or [`RealValueOutput`](#json-schema-real-value) as listed for each attribute in [Components](components.md).
The type is listed for each attribute in [Components](components.md).

* `AttributeValue`: `null` | [`int32_t`](#json-schema-int32_t) | [`int8_t`](#json-schema-int8_t) | [`double`](#json-schema-double) | [`RealValueOutput`](#json-schema-real-value)

#### JSON schema int32_t

An integer `number` value usually representing an `ID`. It may be in the inclusive range $\left[-2^{31},+2^{31} - 1\right]$.
The type is listed for each attribute in [Components](components.md).

* `int32_t`: `number`

#### JSON schema int8_t

An integer `number` value usually representing an enumeration value or a discrete setting. It may be in the inclusive range $\left[-2^{7},+2^{7} - 1\right]$.
The type is listed for each attribute in [Components](components.md).

* `int8_t`: `number`

#### JSON schema double

A floating point `number` or `string` value usually representing a real.
If it is a `string`, it shall be either `"inf"` or `"+inf"` for positive infinity, or `"-inf"` for negative infinity.
Any other values are unsupported.
The type is listed for each attribute in [Components](components.md).

* `double`: `number`|`string`

#### JSON schema RealValueOutput

A real value of which the data format depends on the type of calculation.
For symmetric calculations, it is a [`double`](#json-schema-double). For asymmetric calculations, it is an `Array[double]`.
The type is listed for each attribute in [Components](components.md).

* `RealValueOutput`: `number`

#### JSON schema component

A string containing the component name (see also the [Components](components.md) reference). E.g.: `"node"`

* `Component`: `string`

#### JSON schema attribute

A string containing the name of an attribute of a component (see also the [Components](components.md) reference). E.g.: `"id"`.

* `Attribute`: `string`

#### JSON schema attributes

An array containing the desired [`Attribute`s](#json-schema-attribute) for a specific [`Component`](#json-schema-component).

* `AttributeList`: `Array`
  * `Attribute`: the attribute

### msgpack serialization format specification

The msgpack serialization format is a compressed version of the [JSON serialization format](#json-serialization-format-specification) and all features supported for JSON are also supported for msgpack.
