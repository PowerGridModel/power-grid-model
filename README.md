<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->
[![PyPI version](https://badge.fury.io/py/power-grid-model.svg)](https://badge.fury.io/py/power-grid-model)
[![Anaconda-Server Badge](https://anaconda.org/conda-forge/power-grid-model/badges/version.svg)](https://anaconda.org/conda-forge/power-grid-model)
[![License: MIT](https://img.shields.io/badge/License-MPL2.0-informational.svg)](https://github.com/PowerGridModel/power-grid-model/blob/main/LICENSE)
[![Build and Test C++ and Python](https://github.com/PowerGridModel/power-grid-model/actions/workflows/main.yml/badge.svg)](https://github.com/PowerGridModel/power-grid-model/actions/workflows/main.yml)
[![Check Code Quality](https://github.com/PowerGridModel/power-grid-model/actions/workflows/check-code-quality.yml/badge.svg)](https://github.com/PowerGridModel/power-grid-model/actions/workflows/check-code-quality.yml)
[![Clang Tidy](https://github.com/PowerGridModel/power-grid-model/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/PowerGridModel/power-grid-model/actions/workflows/clang-tidy.yml)
[![REUSE Compliance Check](https://github.com/PowerGridModel/power-grid-model/actions/workflows/reuse-compliance.yml/badge.svg)](https://github.com/PowerGridModel/power-grid-model/actions/workflows/reuse-compliance.yml)
[![docs](https://readthedocs.org/projects/power-grid-model/badge/)](https://power-grid-model.readthedocs.io/en/stable/)

[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=PowerGridModel_power-grid-model&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=PowerGridModel_power-grid-model)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=PowerGridModel_power-grid-model&metric=coverage)](https://sonarcloud.io/summary/new_code?id=PowerGridModel_power-grid-model)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=PowerGridModel_power-grid-model&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=PowerGridModel_power-grid-model)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=PowerGridModel_power-grid-model&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=PowerGridModel_power-grid-model)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=PowerGridModel_power-grid-model&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=PowerGridModel_power-grid-model)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=PowerGridModel_power-grid-model&metric=vulnerabilities)](https://sonarcloud.io/summary/new_code?id=PowerGridModel_power-grid-model)

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.8054429.svg)](https://zenodo.org/record/8054429)

[![](https://github.com/PowerGridModel/.github/blob/main/artwork/svg/color.svg)](#)

# Power Grid Model

`power-grid-model` is a library for steady-state distribution power system analysis distributed for Python and C.
The core of the library is written in C++.
Currently, it supports the following calculations:

* Symmetric and asymmetric power flow calculation with Newton-Raphson method, iterative current method and linear method
* Symmetric and asymmetric state estimation with iterative linear method

See the [power-grid-model documentation](https://power-grid-model.readthedocs.io/en/stable/) for more information.
For various conversions to the power-grid-model, refer to the [power-grid-model-io](https://github.com/PowerGridModel/power-grid-model-io) repository.

## Installation

### Install from PyPI

You can directly install the package from PyPI.

```
pip install power-grid-model
```

### Install from Conda

If you are using `conda`, you can directly install the package from `conda-forge` channel.

```
conda install -c conda-forge power-grid-model
```

### Build and install from Source

To install the library from source, refer to the [Build Guide](https://power-grid-model.readthedocs.io/en/stable/advanced_documentation/build-guide.html).

## Examples

Please refer to [Examples](https://github.com/PowerGridModel/power-grid-model-workshop/tree/main/examples) for more detailed examples for power flow and state estimation. 
Notebooks for validating the input data and exporting input/output data are also included.

## License

This project is licensed under the Mozilla Public License, version 2.0 - see [LICENSE](https://github.com/PowerGridModel/power-grid-model/blob/main/LICENSE) for details.

## Licenses third-party libraries

This project includes third-party libraries, 
which are licensed under their own respective Open-Source licenses.
SPDX-License-Identifier headers are used to show which license is applicable. 
The concerning license files can be found in the [LICENSES](https://github.com/PowerGridModel/power-grid-model/tree/main/LICENSES) directory.

## Contributing

Please read [CODE_OF_CONDUCT](https://github.com/PowerGridModel/.github/blob/main/CODE_OF_CONDUCT.md), [CONTRIBUTING](https://github.com/PowerGridModel/.github/blob/main/CONTRIBUTING.md), [PROJECT GOVERNANCE](https://github.com/PowerGridModel/.github/blob/main/GOVERNANCE.md) and [RELEASE](https://github.com/PowerGridModel/.github/blob/main/RELEASE.md) for details on the process 
for submitting pull requests to us.

Visit [Contribute](https://github.com/PowerGridModel/power-grid-model/contribute) for a list of good first issues in this repo.

## Citations

If you are using Power Grid Model in your research work, please consider citing our library using the following references.

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.8054429.svg)](https://zenodo.org/record/8054429)

```bibtex
@software{Xiang_PowerGridModel_power-grid-model,
  author = {Xiang, Yu and Salemink, Peter and Bharambe, Nitish and Govers, Martinus and van den Bogaard, Jonas and Stoeller, Bram and Jagutis, Laurynas and Wang, Chenguang and {Contributors from the LF Energy project Power Grid Model}},
  doi = {10.5281/zenodo.8054429},
  license = {MPL-2.0},
  title = {{PowerGridModel/power-grid-model}},
  url = {https://github.com/PowerGridModel/power-grid-model}
}
@inproceedings{Xiang2023,
  author = {Xiang, Yu and Salemink, Peter and Stoeller, Bram and Bharambe, Nitish and van Westering, Werner},
  booktitle = {CIRED 2023 - The 27th International Conference and Exhibition on Electricity Distribution},
  title = {Power grid model: A high-performance distribution grid calculation library},
  year = {2023},
  volume={2023},
  number = {},
  pages={1-5}
}
```

## Contact

Please read [SUPPORT](https://github.com/PowerGridModel/.github/blob/main/SUPPORT.md) for how to connect and get into contact with the Power Gird Model project.
