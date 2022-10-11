<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

```{warning}
The documentation is under heavy development
```

% TODO Add short introduction instead of including the README (Right now copied from readme)

`power-grid-model` is a Python library for steady-state distribution power system analysis.
The core of the library is written in C++.
Currently, it supports the following calculations:

* Symmetric and asymmetric power flow calculation with Newton-Raphson method and linear method
* Symmetric and asymmetric state estimation with iterative linear method

# Installation

## Install from PyPI

You can directly install the package from PyPI.

```
pip install power-grid-model
```

## Build and install from Source

To install the library from source, refer to the [Build Guide](build-guide.md).

# Contents

Detailed contents of the documentation are structured as follows.

```{toctree}
:caption: "Installation and quickstart"
:maxdepth: 2
self
getting-started
```

```{toctree}
:caption: "Getting Started"
:maxdepth: 2
data-model
components
calculations
performance-guide
data-validator
```
```{toctree}
:caption: "API Reference"
:maxdepth: 2
python-api-reference
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
native-data-interface
build-guide
```
```{toctree}
:caption: "Contribution"
:maxdepth: 2
contribution/CODE_OF_CONDUCT.md
contribution/CONTRIBUTING.md
contribution/PROJECT_GOVERNANCE.md
contribution/RELEASE.md
contribution/SUPPORT.md
```