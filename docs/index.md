<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

```{warning}
The documentation is under heavy development
```

# Power Grid Model

```{image} images/pgm-logo-color.svg
:alt: pgm_logo
:width: 150px
:align: right
```

`power-grid-model` is a Python library for steady-state distribution power system analysis.
The core of the library is written in C++.
Currently, it supports the following calculations:

* Symmetric and asymmetric power flow calculation with Newton-Raphson method, iterative current method and linear method
* Symmetric and asymmetric state estimation with iterative linear method

For various conversions to the power-grid-model, refer to the [power-grid-model-io](https://github.com/alliander-opensource/power-grid-model-io) repository.

## Install from PyPI

You can directly install the package from PyPI.

```
pip install power-grid-model
```

## Build and install from Source

To install the library from source, refer to the [Build Guide](advanced_documentation/build-guide.md).

# Contents

Detailed contents of the documentation are structured as follows.

```{toctree}
:maxdepth: 2
self
quickstart
```

```{toctree}
:caption: "User Manual"
:maxdepth: 2
user_manual/data-model
user_manual/components
user_manual/calculations
user_manual/performance-guide
user_manual/data-validator
```
```{toctree}
:caption: "API Reference"
:maxdepth: 2
api_reference/python-api-reference
```

```{toctree}
:caption: "Examples"
:maxdepth: 2
examples/Power Flow Example.ipynb
examples/State Estimation Example.ipynb
examples/Validation Examples.ipynb
examples/Make Test Dataset.ipynb
```

```{toctree}
:caption: "Advanced documentation"
:maxdepth: 2
advanced_documentation/native-data-interface
advanced_documentation/build-guide
```
```{toctree}
:caption: "Contribution"
:maxdepth: 2
CODE_OF_CONDUCT.md
CONTRIBUTING.md
PROJECT_GOVERNANCE.md
```
```{toctree}
:caption: "Release and Support"
:maxdepth: 2
release_and_support/RELEASE.md
release_and_support/SUPPORT.md
```
