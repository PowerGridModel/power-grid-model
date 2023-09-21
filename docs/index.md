<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Power Grid Model

```{image} https://github.com/PowerGridModel/.github/raw/main/artwork/svg/color.svg
:alt: pgm_logo
:width: 300px
:align: right
```

`power-grid-model` is a library for steady-state distribution power system analysis distributed for Python and C.
The core of the library is written in C++.
Currently, it supports the following calculations:

* Power Flow
* State Estimation
* Short Circuit

For various conversions to the power-grid-model, refer to the [power-grid-model-io](https://github.com/PowerGridModel/power-grid-model-io) repository.

## Install from PyPI

You can directly install the package from PyPI.

```
pip install power-grid-model
```

## Install from Conda

If you are using `conda`, you can directly install the package from `conda-forge` channel.

```
conda install -c conda-forge power-grid-model
```

## Build and install from Source

To install the library from source, refer to the [Build Guide](advanced_documentation/build-guide.md).

## Citations

If you are using Power Grid Model in your research work, please consider citing our library using the references in [Citation](release_and_support/CITATION.md)

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
user_manual/dataset-terminology
user_manual/components
user_manual/calculations
user_manual/performance-guide
user_manual/data-validator
user_manual/model-validation
user_manual/serialization
```
```{toctree}
:caption: "API Reference"
:maxdepth: 2
api_reference/python-api-reference
api_reference/power-grid-model-c-api-reference
```

```{toctree}
:caption: "Examples"
:maxdepth: 2
examples/Power Flow Example.ipynb
examples/State Estimation Example.ipynb
examples/Short Circuit Example.ipynb
examples/Validation Examples.ipynb
examples/Make Test Dataset.ipynb
```

```{toctree}
:caption: "Advanced documentation"
:maxdepth: 2
advanced_documentation/native-data-interface
advanced_documentation/build-guide
advanced_documentation/c-api
advanced_documentation/core-design
```
```{toctree}
:caption: "Contribution"
:maxdepth: 2
contribution/CODE_OF_CONDUCT.md
contribution/CONTRIBUTING.md
contribution/folder-structure.md
contribution/GOVERNANCE.md
```
```{toctree}
:caption: "Release and Support"
:maxdepth: 2
release_and_support/RELEASE.md
release_and_support/SUPPORT.md
release_and_support/CITATION.md
```
