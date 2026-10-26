<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Power Grid Model

```{image} https://github.com/PowerGridModel/.github/raw/main/artwork/svg/color.svg
:alt: pgm_logo
:width: 300px
:align: right
```

`power-grid-model` is a library for steady-state distribution power system analysis, hosted by the
Linux Foundation Energy (see also the
[LF Energy landing page](https://lfenergy.org/projects/power-grid-model/)).
It is distributed for Python and C.
The core of the library is written in C++.
Currently, it supports both symmetric and asymmetric calculations for the following calculation types:

* Power Flow
* State Estimation
* Short Circuit


### Ecosystem & Extensions

Enhance your workflow or convert your network data using these official and community repositories:

* **Extended Python Interface (PGM-DS):** For a high-level, extended Python interface to the model, refer to the [power-grid-model-ds](https://github.com/PowerGridModel/power-grid-model-ds) repository.
* **Data Conversions (Official):** For converting various standard data formats to the Power Grid Model, refer to the official [power-grid-model-io](https://github.com/PowerGridModel/power-grid-model-io) repository.
* **CGMES Conversions (Community):** For ENTSO-E CIM/CGMES datasets, you can use the third-party [CGMES2PGM Converter](https://github.com/SOPTIM/cgmes2pgm_converter) to generate PGM input data. 
For dataset uploading and state estimation workflows, refer to the accompanying [CGMES2PGM Suite](https://github.com/SOPTIM/cgmes2pgm_suite).

```{note}
Do you wish to be updated on the latest news and releases?
Subscribe to the Power Grid Model mailing list by sending an (empty) email to:
<a href="mailto:powergridmodel+subscribe@lists.lfenergy.org">powergridmodel+subscribe@lists.lfenergy.org</a>
```

## Install from PyPI

You can directly install the package from PyPI.

```sh
pip install power-grid-model
```

## Install from Conda

If you are using `conda`, you can directly install the package from `conda-forge` channel.

```sh
conda install -c conda-forge power-grid-model
```

## Installation via Homebrew

If you are using the C API with `brew`, you can install the C API (and experimental C++ wrapper) directly:

```sh
brew install powergridmodel/pgm/power-grid-model
```

If you are interested in having this package in [`homebrew-core`](https://github.com/Homebrew/homebrew-core), please
consider contributing the [formula](https://github.com/PowerGridModel/homebrew-pgm), in conformance to
[their documentation](https://docs.brew.sh/Acceptable-Formulae).

## Build and install from Source

To install the library from source, refer to the [Build Guide](advanced_documentation/build-guide.md).

## Citations

If you are using Power Grid Model in your research work, please consider citing our library using the references in
[Citation](release_and_support/CITATION.md).

## Contents

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
user_manual/non-native-components
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
api_reference/cli-reference
```

```{toctree}
:caption: "Examples"
:maxdepth: 2
example_notebooks/Power Flow Example.ipynb
example_notebooks/Serialization Example.ipynb
example_notebooks/State Estimation Example.ipynb
example_notebooks/Short Circuit Example.ipynb
example_notebooks/Multi-dimensional batch calculation example.ipynb
example_notebooks/Validation Examples.ipynb
example_notebooks/Make Test Dataset.ipynb
example_notebooks/Asymmetric Calculation Example.ipynb
example_notebooks/Transformer Examples.ipynb
example_notebooks/Generic Branch Example.ipynb
example_notebooks/Asymmetric Line.ipynb
example_notebooks/Logging Example.ipynb
```

```{toctree}
:caption: "Algorithms"
:maxdepth: 2
algorithms/pf-algorithms
algorithms/tap-changing-algorithms
algorithms/sc-algorithms
algorithms/se-algorithms
algorithms/lu-solver
algorithms/dense-and-selective-inverse
```

```{toctree}
:caption: "Advanced documentation"
:maxdepth: 2
advanced_documentation/native-data-interface
advanced_documentation/build-guide
advanced_documentation/devcontainer-setup.md
advanced_documentation/c-api
advanced_documentation/high-level-design
advanced_documentation/core-design
advanced_documentation/python-wrapper-design
advanced_documentation/terminology
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
release_and_support/COMMUNITY.md
release_and_support/SECURITY.md
release_and_support/CITATION.md
```
