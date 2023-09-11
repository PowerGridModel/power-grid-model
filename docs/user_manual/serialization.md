<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Serialization

The power-grid-model provides tools for serialization and deserialization for datasets.
Two types of interfaces are provided: (null-terminated) strings and raw buffer.
Their availability may depend on the desired serialization type.

| Serialization format | (null-terminated) string | raw bytes/buffer |
| -------------------- | ------------------------ | ---------------- |
| JSON                 | &#10004;                 | &#10004;         |
| msgpack              | &#10060;                 | &#10004;         |

Visit the [Python API Reference](../api_reference/python-api-reference.md) and [C API Reference](../api_reference/power-grid-model-c-api-reference.rst) for the full documentation of the serialization tools.
